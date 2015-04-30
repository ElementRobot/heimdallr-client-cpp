
#define BOOST_TEST_MODULE heimdallr_client
#define WAIT_TIME 200
#define PROVIDER_ONE_UUID "1f383dc1-7eae-4e9a-9408-4a345204b372"

#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

#include "heimdallr_client.h"

using namespace std;

void sleepMS(unsigned time) {
    boost::this_thread::sleep(boost::posix_time::millisec(time));
}

class TestProvider : public hmdlr::Provider {
public:
    TestProvider(string token) : hmdlr::Provider(token) {
        url_ = "http://hmdlr.co";
        auth_source_ = "testing";
    } 
    vector<hmdlr::delayed_message> getDelayedMessages() {
        return delayed_messages_;
    }
};

class TestConsumer : public hmdlr::Consumer {
public:
    TestConsumer(string token) : hmdlr::Consumer(token) {
        url_ = "http://hmdlr.co";
        auth_source_ = "testing";
    }
    vector<hmdlr::delayed_message> getDelayedMessages() {
        return delayed_messages_;
    }
};

struct ClientFixture {
    ClientFixture() {
        obj_msg = sio::object_message::create();
        obj_msg->get_map()["stringField"] = sio::string_message::create("");
        obj_msg->get_map()["arrayField"] = sio::array_message::create();
        obj_msg->get_map()["numberField"] = sio::double_message::create(0.0);
        
        str_msg = sio::string_message::create("");
    }
    sio::message::ptr obj_msg;
    sio::message::ptr str_msg;
};

struct ClientPairFixture : ClientFixture {
    ClientPairFixture() : ClientPairFixture("schema-uuid", "schema-uuid", true) {}
    ClientPairFixture(string provider_token, string consumer_token, bool connect) : 
        ClientFixture(),
        provider(provider_token),
        consumer(consumer_token)
    {   
        provider.on("hmdlr-error", [&](sio::event& ev) {
            provider_error = true;
            printf(
                "PROVIDER ERROR: %s\n",
                ev.get_message()->get_map()["message"]->get_string().c_str()
            );
        });
        provider.on("control", [&](sio::event& ev) {
            control_count++;
        });
        
        consumer.on("hmdlr-error", [&](sio::event& ev) {
            consumer_error = true;
            printf(
                "CONSUMER ERROR: %s\n",
                ev.get_message()->get_map()["message"]->get_string().c_str()
            );
        });
        consumer.on("event", [&](sio::event& ev) {
            event_count++;
        });
        consumer.on("sensor", [&](sio::event& ev) {
            sensor_count++;
        });
        
        if (connect) {
            provider.connect();
            consumer.connect();
        }
        
    }
    ~ClientPairFixture() {
        sleepMS(WAIT_TIME);
        BOOST_REQUIRE_MESSAGE(!provider_error, "Provider received an error");
        BOOST_REQUIRE_MESSAGE(!consumer_error, "Consumer received an error");
    }
    
    TestProvider provider;
    TestConsumer consumer;
    bool provider_error = false;
    bool consumer_error = false;
    int event_count = 0;
    int sensor_count = 0;
    int control_count = 0;
};

BOOST_AUTO_TEST_SUITE( provider )
BOOST_AUTO_TEST_CASE( receives_hmdlr_error ) {
    ClientPairFixture f("invalid-uuid", "N/A", false);
    
    f.provider.connect();
    
    sleepMS(WAIT_TIME);
    
    BOOST_REQUIRE(f.provider_error);
    
    f.provider_error = false;
}
    
BOOST_AUTO_TEST_CASE( removes_listener ) {
    ClientPairFixture f("invalid-uuid", "N/A", false);
    
    f.provider.removeListener("hmdlr-error");
    f.provider.connect();
}
    
BOOST_AUTO_TEST_CASE( authorizes ) {
    ClientPairFixture f("valid-uuid", "N/A", false);
    bool authorized = false;
    
    f.provider.on("auth-success", [&](sio::event& ev) {
        authorized = true;
    });
    f.provider.connect();
    
    sleepMS(WAIT_TIME);
    
    BOOST_REQUIRE(authorized);
}

BOOST_AUTO_TEST_CASE( waits_until_ready ) {
    ClientPairFixture f("schema-uuid", "N/A", false);
    vector<hmdlr::delayed_message> msg_vector;
    sio::message::ptr data;
    
    f.provider.sendEvent("objectTest", f.obj_msg);
    f.provider.sendSensor("stringTest", f.str_msg);
    msg_vector = f.provider.getDelayedMessages();
    
    BOOST_REQUIRE_MESSAGE(
        msg_vector.size() == 2,
        "size before connect: " << msg_vector.size()
    );
    
    data = msg_vector[0].packet->get_map()["data"];
    BOOST_REQUIRE_MESSAGE(
        f.obj_msg->get_flag() == data->get_flag(),
        "msg_vector[0] flag was: " << data->get_flag()
    );
    
    data = msg_vector[1].packet->get_map()["data"];
    BOOST_REQUIRE_MESSAGE(
        f.str_msg->get_flag() == data->get_flag(),
        "msg_vector[1] flag was: " << data->get_flag()
    );
    
    f.provider.connect();
    sleepMS(WAIT_TIME);
    msg_vector = f.provider.getDelayedMessages();
    
    BOOST_REQUIRE_MESSAGE(
        msg_vector.size() == 0,
        "size after connect: " << msg_vector.size()
    );
}

BOOST_FIXTURE_TEST_CASE( completes_persistent, ClientPairFixture ) {
    provider.completed("test");
}

BOOST_FIXTURE_TEST_CASE( sends_packets, ClientPairFixture ) {
    provider.sendEvent("objectTest", obj_msg);
    provider.sendSensor("objectTest", obj_msg);
}
BOOST_AUTO_TEST_SUITE_END()
    

BOOST_AUTO_TEST_SUITE( consumer )
BOOST_AUTO_TEST_CASE( receives_hmdlr_error ) {
    ClientPairFixture f("N/A", "invalid-uuid", false);
    
    f.consumer.connect();
    
    sleepMS(WAIT_TIME);
    
    BOOST_REQUIRE(f.consumer_error);
    
    f.consumer_error = false;
}
    
BOOST_AUTO_TEST_CASE( removes_listener ) {
    ClientPairFixture f("N/A", "invalid-uuid", false);
    
    f.consumer.removeListener("hmdlr-error");
    f.consumer.connect();
}
    
BOOST_AUTO_TEST_CASE( authorizes ) {
    ClientPairFixture f("N/A", "valid-uuid", false);
    bool authorized = false;
    
    f.consumer.on("auth-success", [&](sio::event& ev) {
        authorized = true;
    });
    f.consumer.connect();
    
    sleepMS(WAIT_TIME);
    
    BOOST_REQUIRE(authorized);
}

BOOST_AUTO_TEST_CASE( waits_until_ready ) {
    ClientPairFixture f(PROVIDER_ONE_UUID, "schema-uuid", false);
    vector<hmdlr::delayed_message> msg_vector;
    sio::message::ptr data;
    
    f.consumer.sendControl(PROVIDER_ONE_UUID, "objectTest", f.obj_msg);
    f.consumer.sendControl(PROVIDER_ONE_UUID, "stringTest", f.str_msg);
    msg_vector = f.consumer.getDelayedMessages();
    
    BOOST_REQUIRE_MESSAGE(
        msg_vector.size() == 2,
        "size before connect: " << msg_vector.size()
    );
    
    data = msg_vector[0].packet->get_map()["data"];
    BOOST_REQUIRE_MESSAGE(
        f.obj_msg->get_flag() == data->get_flag(),
        "msg_vector[0] flag was: " << data->get_flag()
    );
    
    data = msg_vector[1].packet->get_map()["data"];
    BOOST_REQUIRE_MESSAGE(
        f.str_msg->get_flag() == data->get_flag(),
        "msg_vector[1] flag was: " << data->get_flag()
    );
    
    f.consumer.connect();
    f.provider.connect();
    sleepMS(WAIT_TIME);
    msg_vector = f.consumer.getDelayedMessages();
    
    BOOST_REQUIRE_MESSAGE(
        msg_vector.size() == 0,
        "size after connect: " << msg_vector.size()
    );
}

BOOST_AUTO_TEST_CASE( sends_control ) {
    ClientPairFixture f(PROVIDER_ONE_UUID, "schema-uuid", true);
    
    f.consumer.sendControl(PROVIDER_ONE_UUID, "objectTest", f.obj_msg);
}

BOOST_FIXTURE_TEST_CASE( sets_filter, ClientPairFixture ) {
    sio::message::ptr filter = sio::object_message::create();
    
    filter->get_map()["event"] = sio::array_message::create();
    filter->get_map()["sensor"] = sio::array_message::create();
    
    consumer.setFilter(PROVIDER_ONE_UUID, filter);
}

BOOST_AUTO_TEST_CASE( gets_state ) {
    ClientPairFixture f(PROVIDER_ONE_UUID, "schema-uuid", true);
    
    f.provider.sendEvent("objectTest", f.obj_msg);
    f.provider.sendEvent("stringTest", f.str_msg);
    sleepMS(WAIT_TIME);
    
    sio::message::ptr subtypes = sio::array_message::create();
    subtypes->get_vector().push_back(sio::string_message::create("objectTest"));
    subtypes->get_vector().push_back(sio::string_message::create("stringTest"));
    
    // schema is the UUID
    f.consumer.getState(PROVIDER_ONE_UUID, subtypes);
    
    sleepMS(WAIT_TIME);
    
    BOOST_REQUIRE_MESSAGE(f.event_count == 2, "event_count was: " << f.event_count);
}
BOOST_AUTO_TEST_SUITE_END()
    

BOOST_AUTO_TEST_SUITE( client_pair )    
BOOST_AUTO_TEST_CASE ( interaction ) {
    ClientPairFixture f(PROVIDER_ONE_UUID, "schema-uuid", true);
    
    f.consumer.subscribe(PROVIDER_ONE_UUID);
    f.provider.sendEvent("stringTest", f.str_msg);
    f.provider.sendSensor("objectTest", f.obj_msg);
    f.consumer.sendControl(PROVIDER_ONE_UUID, "objectTest", f.obj_msg);
    
    sleepMS(WAIT_TIME );
    BOOST_REQUIRE_MESSAGE(
        f.event_count == 1,
        "event_count was: " << f.event_count
    );
    BOOST_REQUIRE_MESSAGE(
        f.sensor_count == 1,
        "sensor_count was: " << f.sensor_count
    );
    BOOST_REQUIRE_MESSAGE(
        f.control_count == 1,
        "control_count was: " << f.control_count
    );
}
BOOST_AUTO_TEST_SUITE_END()
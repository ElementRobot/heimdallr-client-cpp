
//#define BOOST_TEST_MODULE heimdallr_client

//#include <boost/test/unit_test.hpp>
//#include <boost/thread/thread.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

#include "heimdallr_client.h"

//void sleepMilli(unsigned time) {
//    boost::this_thread::sleep(boost::posix_time::millisec(time));
//}

class TestProvider : public hmdlr::Provider {
public:
    TestProvider(string token) : hmdlr::Provider(token) {
        url_ = "http://hmdlr.co";
        auth_source_ = "testing";}
};

class TestConsumer : public hmdlr::Consumer {
public:
    TestConsumer(string token) : hmdlr::Consumer(token) {
        url_ = "http://hmdlr.co";
        auth_source_ = "testing";}
};

//BOOST_AUTO_TEST_SUITE( provider_suite )
//    
//BOOST_AUTO_TEST_CASE( authorizes_on_connect )
//{
//    bool authorized = false;
//    TestProvider provider("valid-uuid");
//    provider.on("auth-success", [&](sio::event& _) {
//        authorized = true;
//    });
//    provider.connect();
////    sleepMilli(1000);
//    BOOST_REQUIRE_MESSAGE(authorized, "Failed to authorize");
//}
//
//BOOST_AUTO_TEST_SUITE_END()

int main(){
  
}
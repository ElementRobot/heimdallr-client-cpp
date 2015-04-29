
#include <cstdlib>
#include <stdio.h>
#include <exception>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "heimdallr_client.h"

using namespace std;

string now(void) {
    using namespace boost::posix_time;
    ptime t = microsec_clock::universal_time();
    return to_iso_extended_string(t) + "Z";
}

namespace hmdlr {
    Client::Client(string token, string nsp) {
        ready_ = false;
        token_ = token;
        connection_ = client_.socket(nsp);
        
        connection_->on("hmdlr-error", [&](string const& name, sio::message::ptr const& data, bool is_ack, sio::message::ptr& ack_resp) {
            string msg = data->get_map()["message"]->get_string();
            
            throw error(msg.c_str());
        });
        
        connection_->on("auth-success", [&](sio::event& _) {
            ready_ = true;
            
            for (unsigned i = 0; i < delayed_messages_.size(); i++) {
                connection_->emit(
                    delayed_messages_[i].name,
                    delayed_messages_[i].packet
                );
            }
            delayed_messages_.clear();
        });
        
        client_.set_open_listener([&]() {
            sio::message::ptr packet = sio::object_message::create();
            
            packet->get_map()["token"] = sio::string_message::create(token_);
            packet->get_map()["authSource"] =
                sio::string_message::create(auth_source_);
            
            connection_->emit("authorize", packet);
        });
    }
    
    void Client::connect() {
        client_.connect(url_);
    }
    
    void Client::on(string msg_name, sio::socket::event_listener fn) {
        connection_->on(msg_name, fn);
    }
    
    void Client::sendMsg(string msg_name, sio::message::ptr const& packet) {
        if (ready_) {
            printf("SEDNING PACKET");
            connection_->emit(msg_name, packet);
        } else {
            printf("DELAYED PACKET");
            delayed_messages_.push_back(delayed_message(msg_name, packet));
        }
    }
    
    Provider::Provider(string token) : Client(token, "/provider") {}
    
    void Provider::sendEvent(string subtype, sio::message::ptr const& data) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["subtype"] = sio::string_message::create(subtype);
        packet->get_map()["data"] = data;
        packet->get_map()["t"] = sio::string_message::create(now());
        
        sendMsg("event", packet);
    }
    
    void Provider::sendSensor(string subtype, sio::message::ptr const& data) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["subtype"] = sio::string_message::create(subtype);
        packet->get_map()["data"] = data;
        packet->get_map()["t"] = sio::string_message::create(now());
        
        sendMsg("sensor", packet);
    }
    
    void Provider::completed(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["subtype"] = sio::string_message::create("completed");
        packet->get_map()["uuid"] = sio::string_message::create(uuid);
        packet->get_map()["t"] = sio::string_message::create(now());
        
        sendMsg("event", packet);
    }
    
    Consumer::Consumer(string token) : Client(token, "/consumer") {}
    
    void Consumer::getState(string uuid, vector<string> subtypes) { throw "Not Implemented"; };
    void Consumer::joinStream(string uuid) { throw "Not Implemented"; };
    void Consumer::leaveStream(string uuid) { throw "Not Implemented"; };
    void Consumer::setFilter(string uuid, sio::message::ptr const& data) { throw "Not Implemented"; };
    void Consumer::sendControl(string uuid, sio::message::ptr const& data) { throw "Not Implemented"; };
}
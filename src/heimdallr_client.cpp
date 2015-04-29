
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
    
    void Client::removeListener(string msg_name) {
        connection_->off(msg_name);
    }
    
    void Client::sendMessage(string msg_name, sio::message::ptr const& packet) {
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
        
        sendMessage("event", packet);
    }
    
    void Provider::sendSensor(string subtype, sio::message::ptr const& data) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["subtype"] = sio::string_message::create(subtype);
        packet->get_map()["data"] = data;
        packet->get_map()["t"] = sio::string_message::create(now());
        
        sendMessage("sensor", packet);
    }
    
    void Provider::completed(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["subtype"] = sio::string_message::create("completed");
        packet->get_map()["data"] = sio::string_message::create(uuid);
        packet->get_map()["t"] = sio::string_message::create(now());
        
        sendMessage("event", packet);
    }
    
    Consumer::Consumer(string token) : Client(token, "/consumer") {}
    
    void Consumer::sendControl(string uuid, string subtype, sio::message::ptr const& data) {
        sendControl(uuid, subtype, data, false);
    };
    
    void Consumer::sendControl(string uuid, string subtype, sio::message::ptr const& data, bool persistent) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        packet->get_map()["subtype"] = sio::string_message::create(subtype);
        packet->get_map()["data"] = data;
        packet->get_map()["persistent"] = sio::int_message::create(int(persistent));
        
        sendMessage("control", packet);
    };
    
    void Consumer::subscribe(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        
        sendMessage("subscribe", packet);
    }
    
    void Consumer::unsubscribe(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        
        sendMessage("unsubscribe", packet);
    }
    
    void Consumer::setFilter(string uuid, sio::message::ptr const& filter) {
        if (filter->get_flag() != sio::message::flag_object) {
            throw error("filter should be an object_message");
        }
        
        sio::message::ptr packet = sio::object_message::create();
        map<string, sio::message::ptr> filter_map = filter->get_map();
        map<string, sio::message::ptr>::iterator event;
        map<string, sio::message::ptr>::iterator sensor;
        
        event = filter_map.find("event");
        sensor = filter_map.find("sensor");
        
        if (event == filter_map.end() && sensor == filter_map.end()) {
            throw error("filter should contain event and/or sensor keys");
        }
        
        if (
            event != filter_map.end() &&
                event->second->get_flag() != sio::message::flag_array
        ) {
            throw error("event filter should be an array_message");
        }
        
        if (
            sensor != filter_map.end() &&
                sensor->second->get_flag() != sio::message::flag_array
        ) {
            throw error("sensor filter should be an array_message");
        }
        
        filter_map["provider"] = sio::string_message::create(uuid);
        sendMessage("setFilter", filter);
    }
    
    void Consumer::getState(string uuid, sio::message::ptr const& subtypes) {
        if (subtypes->get_flag() != sio::message::flag_array) {
            throw error("subtypes should be an array_message");
        }
        
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        packet->get_map()["subtypes"] = subtypes;
        
        sendMessage("getState", packet);
    };
    
    void Consumer::joinStream(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        
        sendMessage("joinStream", packet);
    };
    
    void Consumer::leaveStream(string uuid) {
        sio::message::ptr packet = sio::object_message::create();
        
        packet->get_map()["provider"] = sio::string_message::create(uuid);
        
        sendMessage("leaveStream", packet);
    };
}
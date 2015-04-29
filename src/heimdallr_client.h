
#include <vector>

#include "sio_client.h"
#include "sio_message.h"
#include "sio_socket.h"

#ifndef HEIMDALLR_CLIENT
#define HEIMDALLR_CLIENT

using namespace std;

string now(void);


namespace hmdlr {
    class error : public exception {
    public:
        explicit error(const char* message) : msg_(message) {}
        explicit error(const string& message) : msg_(message) {}
        virtual ~error() throw () {}
        virtual const char* what() const throw () {
            return msg_.c_str();
        }
        
    protected:
        string msg_;
        
    };
    
    struct delayed_message {
        string name;
        sio::message::ptr packet;
        delayed_message(string name, sio::message::ptr packet) : 
            name(name),
            packet(packet)
        {};
    };
    
    class Client {
    public:
       Client(string, string);
        void connect(void);
        void on(string, sio::socket::event_listener);
        void removeListener(string);
        void removeListener(string, sio::socket::event_listener);
      
    protected:
        void sendMessage(string, sio::message::ptr const&);
        string url_ = "https://heimdallr.co";
        string auth_source_ = "skyforge";
        
    private:
        string token_;
        bool ready_;
        sio::client client_;
        sio::socket::ptr connection_;
        vector<delayed_message> delayed_messages_;
      
    };
    
    class Provider : public Client {
    public:
        Provider(string);
        void completed(string);
        void sendEvent(string, sio::message::ptr const&);
        void sendSensor(string, sio::message::ptr const&);
        
    };
    
    class Consumer : public Client {
    public:
        Consumer(string);
        void sendControl(string, string, sio::message::ptr const&);
        void sendControl(string, string, sio::message::ptr const&, bool);
        void subscribe(string);
        void unsubscribe(string);
        void setFilter(string, sio::message::ptr const&);
        void getState(string, sio::message::ptr const&);
        void joinStream(string);
        void leaveStream(string);
        
    };
}


#endif
// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "match_server/Match.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/TToString.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::match_service;
using namespace std;

struct Task{
    User user;
    string type;//记录add或者remove
};

struct Queue{
    queue<Task> q;
    mutex m;
    condition_variable cv;
}task_queue;

class Pool{
    private:
        vector<User> users;
    public:
        void save_result(int a,int b){
            printf("Match result is %d and %d.\n",a,b);
        }

        void match(){
            while(users.size()>1){
                auto a=users[0], b=users[1];
                save_result(a.id, b.id);
                users.erase(users.begin());
                users.erase(users.begin());
            }
        }

        void add(User user){
            users.push_back(user);
        }

        void remove(User user){
            for(uint32_t i=0; i<users.size(); i++){
                if(users[i].id == user.id){
                    users.erase(users.begin() + i);
                    break;
                }
            }
        }
}pool;


class MatchHandler : virtual public MatchIf {
    public:
        MatchHandler() {
            // Your initialization goes here
        }

        int32_t add_user(const User& user, const std::string& info) {
            // Your implementation goes here
            printf("add_user\n");

            unique_lock<mutex> lock(task_queue.m);
            task_queue.q.push({user,"add"});
            task_queue.cv.notify_all();

            return 0;
        }

        int32_t remove_user(const User& user, const std::string& info) {
            // Your implementation goes here
            printf("remove_user\n");

            unique_lock<mutex> lock(task_queue.m);
            task_queue.q.push({user,"remove"});
            task_queue.cv.notify_all();

            return 0;
        }
};

class MatchCloneFactory : virtual public MatchIfFactory {
    public:
        ~MatchCloneFactory() override = default;
        MatchIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) override
        {
            std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
            /*cout << "Incoming connection\n";
            cout << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
            cout << "\tPeerHost: "    << sock->getPeerHost() << "\n";
            cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
            cout << "\tPeerPort: "    << sock->getPeerPort() << "\n";*/
            return new MatchHandler;
        }
        void releaseHandler( MatchIf* handler) override {
            delete handler;
        }
};

void match_task(){
    while (true){
        unique_lock<mutex> lock(task_queue.m);
        if(task_queue.q.empty()){
            task_queue.cv.wait(lock);
        }
        else {
            auto task = task_queue.q.front();
            task_queue.q.pop();
            task_queue.m.unlock();

            if(task.type == "add")
                pool.add(task.user);
            if(task.type == "remove")
                pool.remove(task.user);

            pool.match();
        }
    }
}

int main(int argc, char **argv) {
    TThreadedServer server(
            std::make_shared<MatchProcessorFactory>(std::make_shared<MatchCloneFactory>()),
            std::make_shared<TServerSocket>(9090), //port
            std::make_shared<TBufferedTransportFactory>(),
            std::make_shared<TBinaryProtocolFactory>());

    cout<<"Start Match Server"<<endl;

    thread matching_thread(match_task);

    server.serve();
    return 0;
}


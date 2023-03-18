#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "match_client/Match.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace match_service;

void operate(string op,int id,string name,int score){
    std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MatchClient client(protocol);

    transport->open();//打开连接

    User user;
    user.id=id;
    user.name=name;
    user.score=score;

    if(op=="add")
        client.add_user(user,"");

    if(op=="remove")
        client.remove_user(user,"");

    transport->close();//关闭连接
}

int main() {
    string op,name;
    int id,score;
    while(cin>>op>>id>>name>>score){
        operate(op,id,name,score);
    }

    return 0;
}

#pragma once
#include <iot/serversocket.hh>

class ClientConnection {
public:
    static void execute(rohit::socket_t client_id);
};

class ClientConnectionThreaded {
public:
    static void execute(rohit::socket_t client_id);
};



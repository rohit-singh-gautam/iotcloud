#pragma once
#include "serversocket.hh"

class ClientConnection {
public:
    static void execute(iotcloud::socket_t client_id);
};

class ClientConnectionThreaded {
public:
    static void execute(iotcloud::socket_t client_id);
};



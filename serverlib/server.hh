#pragma once

#include <iot/net/serversocket.hh>


class ClientConnectionThreaded {
public:
    static void execute(rohit::socket_t client_id);
};

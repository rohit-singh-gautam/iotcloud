#pragma once

#include <iot/serversocket.hh>


class ClientConnectionThreaded {
public:
    static void execute(rohit::socket_t client_id);
};

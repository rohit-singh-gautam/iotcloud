#pragma once

#include "serversocket.hh"


class ClientConnectionThreaded {
public:
    static void execute(iotcloud::socket_t client_id);
};

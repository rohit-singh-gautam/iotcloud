////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/net/serversocket.hh>


class ClientConnectionThreaded {
public:
    static void execute(rohit::socket_t client_id);
};

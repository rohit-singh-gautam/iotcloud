////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <cstdlib>
#include <cstring>
#include <thread>
#include "socket.hh"
#include <iot/core/log.hh>

namespace rohit {

template <class Execution>
class socketserver {
private:
    server_socket_t socket_id;
    int port;
    int backlog;

    Execution execution;

    bool running;

    template <class Execution1>
    friend inline std::ostream& operator<<(std::ostream& os, const socketserver<Execution1> &sockSrv);

public:
    inline socketserver(int port, int backlog = 5) 
        : socket_id(port, backlog), port(port), backlog(backlog), execution(this), running(false) {  }

    err_t execute() {
        running = true;
        while(running) {
            int client_id = accept(socket_id, NULL, NULL);
            if (client_id < 0) {
                return err_t::ACCEPT_FAILURE;
            }

            execution.execute(client_id);
        }

        return err_t::SUCCESS;
    }
    err_t stop() { running = false; return err_t::SUCCESS; }

    inline const ipv6_socket_addr_t get_peer_ipv6_addr() const {
        return socket_id.get_peer_ipv6_addr();
    }

    inline const ipv6_socket_addr_t get_local_ipv6_addr() const {
        return socket_id.get_local_ipv6_addr();
    }

    friend class server_execution;
}; // class socketserver

template <class Execution>
inline std::ostream& operator<<(std::ostream& os, const socketserver<Execution> &sockSrv) {
    return os << sockSrv.socket_id;
}

// This is simplest execution.
template <class ClientExecution>
class server_execution_simplest {
public:
    inline server_execution_simplest(const socketserver<server_execution_simplest<ClientExecution>> *) {}

    inline void execute(socket_t client_id) {
        ClientExecution::execute(client_id);
    }
};

template <class ClientExecution>
class server_execution_threaded {
private:
    // We are not expecting same client execution to run on multiple ports
    // Hence ClientExecution is static
    std::vector<std::jthread> threads { };

    static void * execute_client(void *args) {
        socket_t &client_id = *(socket_t *)(args);
        ClientExecution::execute(client_id);
        client_id.close();
        return NULL;
    }

public:
    inline server_execution_threaded(const socketserver<server_execution_threaded<ClientExecution>> *) { }

    inline void execute(socket_t client_id) {
        std::jthread newthread { execute_client, client_id };
        threads.push_back(newthread);
    }
};

// Below is sample no implementation
// This must be thread safe and execute will be called from multiple thread
class ClientExecutionSample {
public:
    static void execute(socket_t client_id);
};



} // namespace rohit
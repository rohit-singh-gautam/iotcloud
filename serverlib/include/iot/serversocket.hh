#pragma once
#include <string>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include "socket.hh"
#include "log.hh"

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

    error_t execute() {
        running = true;
        while(running) {
            int client_id = accept(socket_id, NULL, NULL);
            if (client_id < 0) {
                return error_t::ACCEPT_FAILURE;
            }

            execution.execute(client_id);
        }

        return error_t::SUCCESS;
    }
    error_t stop() { running = false; return error_t::SUCCESS; }

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

// This is simpliest execution.
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
public:
    static const constexpr size_t MAX_THREAD = 1000;
private:
    // We are not expecting same client execution to run on multiple ports
    // Hence ClientExecution is static
    pthread_t pthread[MAX_THREAD];
    int next_index = 0;

    static void * execute_client(void *args) {
        socket_t &client_id = *(socket_t *)(args);
        ClientExecution::execute(client_id);
        client_id.close();
        return NULL;
    }

public:
    inline server_execution_threaded(const socketserver<server_execution_threaded<ClientExecution>> *) {
        memset(pthread, '\0', sizeof(pthread));
    }

    inline void execute(socket_t client_id) {
        if (pthread[next_index]) {
            auto errJoin = error_t::pthread_join_ret(pthread_join(pthread[next_index], NULL));
            // We are just logging here
            if (errJoin != error_t::SUCCESS)
                log_warning<logger_message_id::PTHREAD_JOIN_FAILED>(
                    pthread[next_index]);
        }
        auto errCreate = error_t::pthread_create_ret(
            pthread_create(&pthread[next_index], NULL, execute_client, (void *)&client_id));

        if (errCreate != error_t::SUCCESS) // TODO: Wait for a while and loop if not able to create thread
            log_error<logger_message_id::PTHREAD_CREATE_FAILED>(); 
        else ++next_index;
    }
};

// Below is sample no implementation
// This must be thread safe and execute will be called from multiple thread
class ClientExecutionSample {
public:
    static void execute(socket_t client_id);
};



} // namespace rohit
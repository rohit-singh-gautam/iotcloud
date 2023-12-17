/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iot/net/serverevent.hh>
#include <iot/message.hh>

namespace rohit {

namespace message {
    constexpr message_success_t success_request { };
    constexpr message_success_t bad_request { };
};

template <bool use_ssl>
class iotserverevent : public serverpeerevent<use_ssl> {
public:
    static constexpr bool movable = true;

private:
    using serverpeerevent<use_ssl>::peer_id;
    using serverpeerevent<use_ssl>::enter_loop;
    using serverpeerevent<use_ssl>::exit_loop;
    using serverpeerevent<use_ssl>::client_state;
    using serverpeerevent<use_ssl>::write_queue;

    using serverpeerevent_base::push_write;
    using serverpeerevent_base::pop_write;
    using serverpeerevent_base::get_write_buffer;
    using serverpeerevent_base::is_write_left;

public:
    using serverpeerevent<use_ssl>::serverpeerevent;
    
    using serverpeerevent<use_ssl>::write_all;

    void read_helper();

    void execute() override;

    using serverpeerevent<use_ssl>::close;
};

typedef std::function<void(const std::uint8_t *, size_t)> write_function;

typedef std::function<void(message_base_t *, write_function)> read_function;

inline void write_bad_request(write_function writeFunction)
{
    auto write_buffer = reinterpret_cast<const std::uint8_t *>(&message::bad_request);
    auto write_buffer_size = sizeof(message_bad_request_t);

    writeFunction(write_buffer, write_buffer_size);
}

void read_register(message_base_t *, write_function);
void read_connect(message_base_t *, write_function);
void read_command(message_command_t *, write_function);


inline void write_success_request(write_function writeFunction)
{
    auto write_buffer = reinterpret_cast<const std::uint8_t *>(&message::success_request);
    auto write_buffer_size = sizeof(message_success_t);

    writeFunction(write_buffer, write_buffer_size);
}

inline void read_keep_alive(write_function writeFunction)
{
    write_success_request(writeFunction);
}


template <bool use_ssl>
void iotserverevent<use_ssl>::read_helper() {
    size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    size_t read_buffer_length { };

    auto err = peer_id.read(read_buffer, read_buffer_size, read_buffer_length);

    if (err == err_t::SOCKET_RETRY) {
        // No state change required
        return;
    }
    if (isFailure(err)) {
        log<log_t::IOT_EVENT_SERVER_READ_FAILED>(err);
        return;
    }

    if (read_buffer_length == 0) {
        // No data indication that wait
        return;
    }

    rohit::message_base_t *base = (rohit::message_base_t *)read_buffer;

    if constexpr (config::debug) {
        std::cout << "------Request Start---------\n" << *base << "\n------Request End---------\n";
    }

    auto writeFunction = [this](const std::uint8_t *write_buffer, size_t size)
    {
        if constexpr (config::debug) {
            message_base_t *write_base = (message_base_t *)write_buffer;
            std::cout << "------Response Start---------\n" << *write_base << "\n------Response End---------\n";
        }
        this->push_write(write_buffer, size);
    };

    switch(base->getMessageCode())
    {
        case message_code_t::COMMAND:
            read_command(static_cast<message_command_t *>(base), writeFunction);
            break;

        case message_code_t::CONNECT:
            read_connect(base, writeFunction);
            break;
            
        case message_code_t::REGISTER:
            read_register(base, writeFunction);
            break;

        case message_code_t::KEEP_ALIVE:
            read_keep_alive(writeFunction);
            break;

        default:
            write_bad_request(writeFunction);
            break;
    }

    write_all();

    // Tail recurssion
    read_helper();
}

template <bool use_ssl>
void iotserverevent<use_ssl>::execute() {
    switch (client_state) {
        case state_t::SOCKET_PEER_ACCEPT: {
            auto err = peer_id.accept();
            if (err == err_t::SUCCESS) {
                client_state = state_t::SOCKET_PEER_EVENT;
            } else {
                close();
            }
            break;
        }
        case state_t::SOCKET_PEER_CLOSE: {
            close();
            break;
        }
        case state_t::SOCKET_PEER_EVENT:
        case state_t::SOCKET_PEER_READ: {
            read_helper();
            break;
        }
        case state_t::SOCKET_PEER_WRITE: {
            write_all();
            read_helper();
            break;
        }
        default:;

    }
}

} // namespace rohit
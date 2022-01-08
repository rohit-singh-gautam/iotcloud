////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/error.hh>
#include <iot/core/math.hh>
#include <iot/core/log.hh>
#include <iot/core/guid.hh>
#include <iot/core/ipv6addr.hh>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <iot/states/states.hh>

int success = 0;
int failed = 0;

void test_err_t() {
    for(rohit::err_t i = rohit::err_t::SUCCESS; i <= rohit::err_t::MAX_FAILURE; ++i) {
        std::cout << "Error value: " << (rohit::log_id_type)i << ", " << i << std::endl;
    }
}


void test_itoa() {
    std::pair<uint16_t, std::string> port_list[] = {
        {  8080,  "8080"},
        {    80,    "80"},
        {   443,   "443"},
        {  8443,  "8443"},
        {    21,    "21"},
        {    22,    "22"},
        { 50000, "50000"},
    };

    for(auto port_entry: port_list) {
        uint16_t port = port_entry.first;
        char portstr[6];
        auto count = rohit::to_string(port, portstr);
        auto len = strlen(port_entry.second.c_str()) + 1;

        if (count != len) {
            std::cout << "Failed length count  number: " << port_entry.first
                        << ", match: " << port_entry.second
                        << ", converted: " << portstr
                        << ", original len: " << len
                        << ", converted len: " << count << std::endl;
        }

        std::cout << "Port str: " << portstr << ", port value: " << port << std::endl;

        std::string cppStrPort(portstr);

        if (port_entry.second != cppStrPort) {
            std::cout << "Failed to match number: " << port_entry.first
                        << ", to match: " << port_entry.second
                        << ", converted: " << portstr << std::endl;
            ++failed;
        } else ++success;
    }
}

template <rohit::log_t ID>
void test_types_helper() {
    using rohit::logger;
    using rohit::logger_level;
    using desc = rohit::log_description<ID>;

    std::cout << "Log: " << desc::id_str << ":" << desc::value << ", types: ";
    for(size_t i = 0; i < desc::type_count; i++) {
        std::cout << rohit::type_str[(size_t)(desc::type_list[i])] << "|";
    }

    std::cout << std::endl;
}

void test_types_what_type() {
    using namespace rohit;
    std::cout << "Type comparison template test " << std::endl;
    std::cout << "char: " << type_str[(size_t)what_type<char>::value] << std::endl;
    std::cout << "int8_t: " << type_str[(size_t)what_type<int8_t>::value] << std::endl;
    std::cout << "int16_t: " << type_str[(size_t)what_type<int16_t>::value] << std::endl;
    std::cout << "int32_t: " << type_str[(size_t)what_type<int32_t>::value] << std::endl;
    std::cout << "int64_t: " << type_str[(size_t)what_type<int64_t>::value] << std::endl;
    std::cout << "uint8_t: " << type_str[(size_t)what_type<uint8_t>::value] << std::endl;
    std::cout << "uint16_t: " << type_str[(size_t)what_type<uint16_t>::value] << std::endl;
    std::cout << "uint32_t: " << type_str[(size_t)what_type<uint32_t>::value] << std::endl;
    std::cout << "uint64_t: " << type_str[(size_t)what_type<uint64_t>::value] << std::endl;
    std::cout << "float_t: " << type_str[(size_t)what_type<float_t>::value] << std::endl;
    std::cout << "double_t: " << type_str[(size_t)what_type<double_t>::value] << std::endl;
    std::cout << "size_t: " << type_str[(size_t)what_type<size_t>::value] << std::endl;
    std::cout << "ssize_t: " << type_str[(size_t)what_type<ssize_t>::value] << std::endl;
    std::cout << "ipv6_addr_t: " << type_str[(size_t)what_type<ipv6_addr_t>::value] << std::endl;
    std::cout << "ipv6_socket_addr_t: " << type_str[(size_t)what_type<ipv6_socket_addr_t>::value] << std::endl;

    std::cout << "short: " << type_str[(size_t)what_type<short>::value] << std::endl;
    std::cout << "int: " << type_str[(size_t)what_type<int>::value] << std::endl;
    std::cout << "long: " << type_str[(size_t)what_type<long>::value] << std::endl;
    std::cout << "long long: " << type_str[(size_t)what_type<long long>::value] << std::endl;
    std::cout << "long long int: " << type_str[(size_t)what_type<long long int>::value] << std::endl;
    std::cout << "unsigned: " << type_str[(size_t)what_type<unsigned>::value] << std::endl;
    std::cout << "unsigned long: " << type_str[(size_t)what_type<unsigned long>::value] << std::endl;
    std::cout << "unsigned long long: " << type_str[(size_t)what_type<unsigned long long>::value] << std::endl;

    std::cout << "sizeof(long long)" << sizeof(long long) << std::endl;

    constexpr rohit::guid_t guid = rohit::to_guid("f81d4fae-7dec-11d0-a765-00a0c91e6bf6");



    rohit::ipv6_socket_addr_t ipv6sockaddr("::1", 8080);
    rohit::ipv6_addr_t ipv6addr = rohit::to_ipv6_addr_t("eb::1");
    rohit::ipv6_port_t ipv6port = 8080;
}

void test_types() {
    using rohit::log_t;
    test_types_helper<log_t::PTHREAD_CREATE_FAILED>();
    test_types_helper<log_t::PTHREAD_JOIN_FAILED>();
    test_types_helper<log_t::SYSTEM_ERROR>();
    test_types_helper<log_t::IOT_ERROR>();
    test_types_helper<log_t::TEST_INTEGER_LOGS>();
    test_types_helper<log_t::TEST_FLOAT_LOGS>();
    test_types_helper<log_t::TEST_IPV6ADDR_LOGS>();
    test_types_helper<log_t::MAX_LOG>();
    test_types_helper<log_t::TEST_GUID_LOG>();
    test_types_helper<log_t::TEST_STATE_LOG>();
    test_types_what_type();
}

void test_readlog(rohit::logreader &log_reader) {
    auto logstr = log_reader.readnext();
    std::cout << "LOG:" << logstr << std::endl;
}

void test_logs() try {
    using rohit::logger;
    using rohit::logger_level;
    using rohit::log_t;
    using rohit::log;

    std::cout << "Total type of logs count: " << rohit::log_t_count << std::endl;
    const char *log_filename = "/tmp/test_logs.bin";

    remove(log_filename);
    std::cout << "Initializing log \n"; 
    rohit::init_log_thread(log_filename);

    std::cout << "Writing log" << std::endl;
    log<log_t::PTHREAD_JOIN_FAILED>(101);
    log<log_t::TEST_FLOAT_LOGS>(101.0f, 102.0);

    size_t log_count = 201;
    for (size_t count = 0; count < log_count; ++count) {
        log<log_t::TEST_INTEGER_LOGS>(101, 102l, 103ll,
            (int16_t)104, (int8_t)105, 201u, 202lu, 203llu, (uint16_t)204, (uint8_t)205);
    }

    rohit::ipv6_socket_addr_t ipv6sockaddr("eb::1", 8080);
    rohit::ipv6_addr_t ipv6addr = rohit::to_ipv6_addr_t("eb::1");
    rohit::ipv6_port_t ipv6port = 8080;

    log<log_t::TEST_IPV6ADDR_LOGS>('v', ipv6sockaddr, ipv6sockaddr, ipv6addr, ipv6addr, ipv6port);
    log<log_t::SYSTEM_ERROR>(EINVAL);
    log<log_t::IOT_ERROR>(rohit::err_t::GUID_BAD_STRING_FAILURE);

    rohit::guid_t guid = rohit::to_guid("f81d4fae-7dec-11d0-a765-00a0c91e6bf6");
    log<log_t::TEST_GUID_LOG>(guid, guid);
    log<log_t::TEST_STATE_LOG>(rohit::state_t::EVENT_DIST_NONE);

    constexpr auto wait_time = std::chrono::milliseconds(rohit::config::log_thread_wait_in_millis*2);
    std::this_thread::sleep_for(wait_time);
    sync();

    std::cout << "Reading log" << std::endl;
    // Read written logs
    rohit::logreader log_reader(log_filename);

    test_readlog(log_reader);
    test_readlog(log_reader);

    for (size_t count = 0; count < log_count; ++count) {
        try {
            std::cout << count << ": ";
            test_readlog(log_reader);
        } catch(rohit::exception_t exception) {
            std::cout << "test_logs failed with exception is " << exception << std::endl;
            break;
        }
    }

    test_readlog(log_reader);
    test_readlog(log_reader);
    test_readlog(log_reader);
    test_readlog(log_reader);
    test_readlog(log_reader);

    rohit::destroy_log_thread();

    char guid_str[rohit::guid_t::guid_string_withnull_size] = {};
    rohit::to_string(guid, guid_str);
    std::cout << "GUID String: " << guid_str << std::endl;
    rohit::to_string<rohit::number_case::upper>(guid, guid_str);
    std::cout << "GUID String in caps: " << guid_str << std::endl;

    std::cout << "Read done " << std::endl;
    std::cout << std::endl;
} catch (rohit::exception_t exception) {
    std::cout << "test_logs failed with exception is " << exception << std::endl;
}

int main() {
    test_err_t();
    test_itoa();

    std::cout << "Test Logs " << std::endl;
    test_logs();
    std::cout << std::endl << std::endl;

    std::cout << "Test Types " << std::endl;
    test_types();
    std::cout << std::endl << std::endl;

    std::cout << "Success: " << success << ", Failed: " << failed << std::endl;

    return EXIT_SUCCESS;
}

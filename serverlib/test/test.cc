#include <iot/core/math.hh>
#include <iot/log.hh>
#include "test.hh"
#include <iot/serversocket.hh>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include "../httpserver/http11driver.hh"
#include <pthread.h>

int success = 0;
int failed = 0;

void test_err_t() {
    for(rohit::err_t i = rohit::err_t::SUCCESS; i <= rohit::err_t::MAX_FAILURE; ++i) {
        std::cout << "Error value: " << (rohit::log_id_type)i << ", " << i << std::endl;
    }
}

void ClientConnection::execute(rohit::socket_t client_id) {
    std::cout << "Connection Received from " << client_id.get_peer_ipv6_addr() << std::endl;

    size_t read_buffer_size = 1024;
    char read_buffer[read_buffer_size];
    size_t read_buffer_length;
    client_id.read(read_buffer, read_buffer_size, read_buffer_length);
    std::string request_string(read_buffer, read_buffer_length);
    std::cout << "------Request Start---------\n" << request_string << "\n------Request End---------\n";

    iotcloud::http11driver driver;
    driver.parse(request_string);
    std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

    http_response responseContent(
        http_header::VERSION::VER_1_1,
        200_rc, {
            {http_header::FIELD::Server, "IOTCLOUD"},
            {http_header::FIELD::Content_Type, "application/json"},
        }, 
        std::string("{result:""success""}\n"));
    responseContent.addMD5();
    std::stringstream strResponse;
    strResponse << responseContent;
    std::string strResponseHeader = strResponse.str();

    std::cout << "------Response Start---------\n" << strResponseHeader << "\n------Response End---------\n";

    client_id.write(strResponseHeader.c_str(), strResponseHeader.length());
}

void ClientConnectionThreaded::execute(rohit::socket_t client_id) {
    std::cout << "Connection Received from " << client_id.get_peer_ipv6_addr() << std::endl;

    size_t read_buffer_size = 1024;
    char read_buffer[read_buffer_size];
    size_t read_buffer_length;
    client_id.read(read_buffer, read_buffer_size, read_buffer_length);
    std::string request_string(read_buffer, read_buffer_length);
    std::cout << "------Request Start---------\n" << request_string << "\n------Request End---------\n";

    iotcloud::http11driver driver;
    driver.parse(request_string);
    std::cout << "------Driver Start---------\n" << driver << "\n------Driver End---------\n";

    http_response responseContent(
        http_header::VERSION::VER_1_1,
        200_rc, {
            {http_header::FIELD::Server, "IOTCLOUD"},
            {http_header::FIELD::Content_Type, "application/json"},
        }, 
        std::string("{result:""success""}\n"));
    responseContent.addMD5();
    std::stringstream strResponse;
    strResponse << responseContent;
    std::string strResponseHeader = strResponse.str();

    std::cout << "------Response Start---------\n" << strResponseHeader << "\n------Response End---------\n";

    client_id.write(strResponseHeader.c_str(), strResponseHeader.length());
}

void *test_serversocket(void *) try {
    int port = 8081;
    rohit::socketserver<rohit::server_execution_simplest<ClientConnection>> serversocket(port);
    
    rohit::err_t err = serversocket.execute();
    if (isFailure(err)) {
        std::cout << "Failed with failure " << err << std::endl;
    }
 
    pthread_exit(NULL);
} catch (rohit::exception_t excep) {
    std::cout << excep << std::endl;
    pthread_exit(NULL);
}

void *test_serversocket_threaded(void *) try {
    int port = 8080;
    rohit::socketserver<rohit::server_execution_threaded<ClientConnectionThreaded>> serversocket(port);

    rohit::err_t err = serversocket.execute();
    if (isFailure(err)) {
        std::cout << "Failed with failure " << err << std::endl;
    }
 
    pthread_exit(NULL);
} catch (rohit::exception_t excep) {
    std::cout << excep << std::endl;
    pthread_exit(NULL);
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
        auto count = rohit::to_string(portstr, port);
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

template <rohit::logger_message_id ID>
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

#define fmtfn(x, ...) check_formatstring_args<rohit::logger_message_id::x>(__VA_ARGS__)
#define fmtstr(x, ...) "check_formatstring_args<rohit::logger_message_id::" #x ">(" #__VA_ARGS__ ")"

#define check_formatstring_args_macro(result, x, ...)  { \
    auto ret = fmtfn(x, __VA_ARGS__); \
    if (ret == result) success++; \
    else failed++; \
    std::cout << fmtstr(x, __VA_ARGS__) ": (SIZE_MAX,18446744073709551615==SUCCESS) " << ret << std::endl; }

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

    check_formatstring_args_macro(SIZE_MAX, PTHREAD_CREATE_FAILED);
    check_formatstring_args_macro(0, TEST_INTEGER_LOGS);
    check_formatstring_args_macro(0, PTHREAD_JOIN_FAILED);
    check_formatstring_args_macro(SIZE_MAX, PTHREAD_JOIN_FAILED, 101lu);
    check_formatstring_args_macro(SIZE_MAX, SYSTEM_ERROR, EINVAL);
    check_formatstring_args_macro(SIZE_MAX, IOT_ERROR, rohit::err_t::MATH_INSUFFICIENT_BUFFER);
    check_formatstring_args_macro(SIZE_MAX, TEST_FLOAT_LOGS, 101.0f, 102.0);
    check_formatstring_args_macro(SIZE_MAX, TEST_INTEGER_LOGS, 101, 102l, 103ll, (int16_t)104, (int8_t)105, 201u, 202lu, 203llu, (uint16_t)204, (uint8_t)205);

    rohit::ipv6_socket_addr_t ipv6sockaddr("::1", 8080);
    rohit::ipv6_addr_t ipv6addr = rohit::string_to_ipv6_addr_t("eb::1");
    rohit::ipv6_port_t ipv6port = 8080;
    check_formatstring_args_macro(SIZE_MAX, TEST_IPV6ADDR_LOGS, 'v', ipv6sockaddr, ipv6sockaddr, ipv6addr, ipv6addr, ipv6port);
}

void test_types() {
    using rohit::logger_message_id;
    test_types_helper<logger_message_id::PTHREAD_CREATE_FAILED>();
    test_types_helper<logger_message_id::PTHREAD_JOIN_FAILED>();
    test_types_helper<logger_message_id::SYSTEM_ERROR>();
    test_types_helper<logger_message_id::IOT_ERROR>();
    test_types_helper<logger_message_id::TEST_INTEGER_LOGS>();
    test_types_helper<logger_message_id::TEST_IPV6ADDR_LOGS>();
    test_types_helper<logger_message_id::MAX_LOG>();
    test_types_what_type();
}

void test_readlog(rohit::logreader &log_reader) {
    auto logstr = log_reader.readnext();
    std::cout << "Log String: " << logstr << std::endl;
}

void test_logs() try {
    using rohit::logger;
    using rohit::logger_level;
    using rohit::logger_message_id;
    using rohit::log_verbose;

    std::cout << "Total type of logs count: " << rohit::logger_message_id_count << std::endl;
    const std::string log_filename = "/tmp/test_logs.txt";

    remove(log_filename.c_str());
    logger::init(log_filename);

    std::cout << "Writing log" << std::endl;
    log_verbose<logger_message_id::PTHREAD_JOIN_FAILED>(101lu);
    log_verbose<logger_message_id::TEST_FLOAT_LOGS>(101.0f, 102.0);

    size_t log_count = 201;
    for (size_t count = 0; count < log_count; ++count) {
        log_verbose<logger_message_id::TEST_INTEGER_LOGS>(101, 102l, 103ll,
            (int16_t)104, (int8_t)105, 201u, 202lu, 203llu, (uint16_t)204, (uint8_t)205);
    }

    rohit::ipv6_socket_addr_t ipv6sockaddr("eb::1", 8080);
    rohit::ipv6_addr_t ipv6addr = rohit::string_to_ipv6_addr_t("eb::1");
    rohit::ipv6_port_t ipv6port = 8080;

    log_verbose<logger_message_id::TEST_IPV6ADDR_LOGS>('v', ipv6sockaddr, ipv6sockaddr, ipv6addr, ipv6addr, ipv6port);
    log_verbose<logger_message_id::SYSTEM_ERROR>(EINVAL);
    log_verbose<logger_message_id::IOT_ERROR>(rohit::err_t::GUID_BAD_STRING_FAILURE);

    logger::flush();
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

    std::cout << "Read done " << std::endl;
    std::cout << std::endl;
} catch (rohit::exception_t exception) {
    std::cout << "test_logs failed with exception is " << exception << std::endl;
}

int main() {
    //test_err_t();
    //test_itoa();

    std::cout << "Test Logs " << std::endl;
    test_logs();
    std::cout << std::endl << std::endl;

    std::cout << "Test Types " << std::endl;
    test_types();
    std::cout << std::endl << std::endl;

    /*pthread_t pthreadIds[2];

    auto errCreate = rohit::err_t::pthread_create_ret(pthread_create(&pthreadIds[0], NULL, test_serversocket, NULL));
    if (errCreate.isFailure()) {
        errCreate.log();
        return EXIT_FAILURE;
    }

    errCreate = rohit::err_t::pthread_create_ret(pthread_create(&pthreadIds[1], NULL, test_serversocket_threaded, NULL));
    if (errCreate) {
        errCreate.log();
        return EXIT_FAILURE;
    }

    for (auto threadId: pthreadIds) {
        auto err = rohit::err_t::pthread_join_ret(pthread_join(threadId, NULL));
        if (err != rohit::err_t::SUCCESS) {
            err.log();
            return EXIT_FAILURE;
        }
    } */

    std::cout << "Success: " << success << ", Failed: " << failed << std::endl;

    return EXIT_SUCCESS;
}

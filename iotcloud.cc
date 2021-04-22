// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in)

// [START cloudrun_iotcloud_service]
// [START cloud_run_iotcloud]
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>

#include "iotcloud_session_handler.hpp"

namespace asio = boost::asio;
namespace po = boost::program_options;
using tcp = boost::asio::ip::tcp;

std::uint16_t get_default_port() {
    auto env = std::getenv("PORT");
    if (env == nullptr) return 8080;
    auto value = std::stoi(env);
    if (value > std::numeric_limits<std::uint16_t>::max()) {
      std::ostringstream os;
      os << "The PORT environment variable value (" << value
         << ") is out of range.";
      throw std::invalid_argument(std::move(os).str());
    }
    return static_cast<std::uint16_t>(value);
}

po::variables_map parse_args(int& argc, char* argv[]) {
  // Initialize the default port with the value from the "PORT" environment
  // variable or with 8080.
  auto port = get_default_port();

  // Parse the command-line options.
  po::options_description desc("Server configuration");
  desc.add_options()
      //
      ("help", "display help")
      //
      ("address", po::value<std::string>()->default_value("0.0.0.0"),
       "set listening IP address")
      //
      ("port", po::value<std::uint16_t>()->default_value(port),
       "set listening TCP port");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << "\n";
  }
  return vm;
}

int main(int argc, char* argv[]) try {
  po::variables_map vm = parse_args(argc, argv);

  if (vm.count("help")) return 0;

  auto address = asio::ip::make_address(vm["address"].as<std::string>());
  auto port = vm["port"].as<std::uint16_t>();
  std::cout << "Listening on " << address << ":" << port << std::endl;

  asio::io_context ioc{/*concurrency_hint=*/1};
  tcp::acceptor acceptor{ioc, {address, port}};
  for (;;) {
    auto socket = acceptor.accept(ioc);
    if (!socket.is_open()) break;
    // Run a thread per-session, transferring ownership of the socket
    std::thread{iotcloud_session_handler, std::move(socket)}.detach();
  }

  return 0;
} catch (std::exception const& ex) {
  std::cerr << "Standard exception caught " << ex.what() << '\n';
  return 1;
}
// [END cloud_run_iotcloud]
// [END cloudrun_iotcloud_service]

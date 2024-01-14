#include "deviceclient.hh"
#include <iot/message.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>
#include <iostream>
#include <cstdlib>

void rohit::DeviceClient::PowerOn(std::uint32_t componentIndex) {
    rohit::message::Command command { };
    command.add(guid, componentIndex, rohit::message::Operation::Code::SWITCH, rohit::message::Operation::Switch::ON);
    size_t actualSend { 0 };

    client_socket.write(reinterpret_cast<const void *>(&command), command.length(), actualSend);
}

void rohit::DeviceClient::PowerOff(std::uint32_t componentIndex) {
    rohit::message::Command command { };
    command.add(guid, componentIndex, rohit::message::Operation::Code::SWITCH, rohit::message::Operation::Switch::OFF);

    size_t actualSend { 0 };
    client_socket.write(reinterpret_cast<const void *>(&command), command.length(), actualSend);
}

void rohit::DeviceClient::Brigtness(std::uint32_t componentIndex, std::uint16_t value) {
    rohit::message::Command command { };
    command.add(guid, componentIndex, rohit::message::Operation::Code::LEVEL, value);
    size_t actualSend { 0 };

    client_socket.write(reinterpret_cast<const void *>(&command), command.length(), actualSend);
}

void rohit::DeviceClient::SelectColor(std::uint32_t componentIndex, std::uint16_t red, std::uint16_t green, std::uint16_t blue, std::uint16_t gamma) {
    rohit::message::Command command { };
    command.add(guid, componentIndex, rohit::message::Operation::Code::RED, red);
    command.add(guid, componentIndex, rohit::message::Operation::Code::GREEN, green);
    command.add(guid, componentIndex, rohit::message::Operation::Code::BLUE, blue);
    command.add(guid, componentIndex, rohit::message::Operation::Code::GAMMA, gamma);

    size_t actualSend { 0 };
    client_socket.write(reinterpret_cast<const void *>(&command), command.length(), actualSend);
}

std::ostream &operator<<(std::ostream &os, const rohit::DeviceModel model) {
    return os << model.GetString();
}

template <size_t size>
std::ostream &operator<<(std::ostream &os, const std::string (&strlist)[size]) {
    os << "{\n";
    for (size_t index { 1 }; index < size; ++index) {
        os << '\t' << index << ". " << strlist[index] << '\n';
    }
    os << "}\n";
    return os;
}

class DeviceClientParameter {
    rohit::ipv6_socket_addr_t ipv6addr;
    bool use_ssl { };
    bool display_version { };
    bool list_device { };
    bool valid { };
    
    rohit::DeviceModel model{ };
public:
    DeviceClientParameter(int argc, char *argv[]) {
        const char *_deviceModelName{ };
        rohit::commandline param_parser(
            "Device Client Example",
            "Device Client to make basic command to client server",
            {
                {'a', "address", "IPV6 address with port", "Device server address e.g. [::1]:8080", ipv6addr},
                {"use_ssl", "Device server SSL address", use_ssl},
                {'v', "version", "Display version", display_version},
                {'d', "device", "name or index", "Select Device model index or name", _deviceModelName},
                {'l', "devicelist", "Get list of devices", list_device}
            }
        );

        valid = param_parser.parser(argc, argv);

        if (!valid)
            std::cout << param_parser.usage() << std::endl;

        if (display_version) {
            std::cout << param_parser.get_name() << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
        }
        
        if (list_device) {
            std::cout << "Device List\n" << rohit::DeviceModel::devicestr << '\n';
        }

        if (_deviceModelName) {
            try {
                auto modelInt = std::atoi(_deviceModelName);
                if (modelInt) {
                    model = rohit::DeviceModel { modelInt }; 
                }
            } catch(const std::out_of_range &e) {

            }
            if (!model.IsValid()) {
                model = rohit::DeviceModel(std::string { _deviceModelName });
            }
            if (!model.IsValid()) {
                std::cout << "Select one of valid device type from below list:\n" << rohit::DeviceModel::devicestr << '\n';
                std::cout << '"' << _deviceModelName << "\" is not a valid device.\n";
            }
        } else {
            if (valid && !display_version && !list_device && !model.IsValid()) {
                std::cout << param_parser.usage() << std::endl;
            }
        }
    }

    auto GetIsDisplayVersion() const { return display_version; }
    auto GetIsListDevice() const { return list_device; }
    auto IsValid() const { return valid && model.IsValid(); }
    auto IsInfoOnlyParameter() const { return !IsValid() || display_version || list_device; }
    rohit::DeviceModel GetDeviceModel() const { return model; }

};

int main(int argc, char *argv[]) {
    DeviceClientParameter parameter{ argc, argv };
    if(parameter.IsInfoOnlyParameter()) return EXIT_SUCCESS;

    std::cout << "Configuring Model: " << parameter.GetDeviceModel() << '\n';

    return EXIT_SUCCESS;
}

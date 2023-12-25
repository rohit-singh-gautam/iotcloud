#pragma once
#include <cstdint>
#include <iot/core/types.hh>
#include <string_view>
#include <string>
#include <unordered_map>
#include <concepts>
#include <iot/net/serversocket.hh>

namespace rohit {

#define DEVICE_TYPE_LIST \
    DEVICE_TYPE_ENTRY(Invalid) \
    DEVICE_TYPE_ENTRY(Switch) \
    DEVICE_TYPE_ENTRY(Switch2Way) \
    DEVICE_TYPE_ENTRY(Switch3Way) \
    DEVICE_TYPE_ENTRY(Switch4Way) \
    DEVICE_TYPE_ENTRY(Switch5Way) \
    DEVICE_TYPE_ENTRY(Switch6Way) \
    DEVICE_TYPE_ENTRY(Switch7Way) \
    DEVICE_TYPE_ENTRY(Switch8Way) \
    DEVICE_TYPE_ENTRY(Switch9Way) \
    DEVICE_TYPE_ENTRY(Switch10Way) \
    DEVICE_TYPE_ENTRY(Switch11Way) \
    DEVICE_TYPE_ENTRY(Switch12Way) \
    DEVICE_TYPE_ENTRY(Switch13Way) \
    DEVICE_TYPE_ENTRY(Switch14Way) \
    DEVICE_TYPE_ENTRY(Switch15Way) \
    DEVICE_TYPE_ENTRY(Switch16Way) \
    DEVICE_TYPE_ENTRY(RGBLight) \
    DEVICE_TYPE_ENTRY(HighDefintionRGBLight) \
    LIST_DEFINITION_END

class DeviceModel {
public:
    enum DeviceModelEnum : std::uint32_t {
#define DEVICE_TYPE_ENTRY(x) x,
        DEVICE_TYPE_LIST
#undef DEVICE_TYPE_ENTRY
        OUT_OF_RANGE
    };

    static constexpr auto out_of_range{ static_cast<std::uint32_t>(OUT_OF_RANGE) };
    static const std::string devicestr[];

private:
    DeviceModelEnum model;

    static const std::unordered_map<std::string_view, DeviceModelEnum> devicemap;

    static DeviceModelEnum GetDeviceModelEnum(const std::string_view &str) {
        auto itr = devicemap.find(str);
        if (itr != std::end(devicemap)) {
            return itr->second;
        } else return DeviceModelEnum::Invalid;
    }

    static DeviceModelEnum GetDeviceModelEnum(const std::string &str) {
        std::string_view str_view {str.c_str(), str.size() };
        return GetDeviceModelEnum(str_view);
    }

    static constexpr DeviceModelEnum GetDeviceModelEnum(const std::integral auto value) {
        constexpr auto out_of_range = static_cast<decltype(value)>(DeviceModelEnum::OUT_OF_RANGE);
        if (value >= out_of_range) throw std::out_of_range(std::string("Device Model Value"));
        return static_cast<DeviceModelEnum>(value);
    }

public:
    constexpr DeviceModel() : model{ DeviceModelEnum::Invalid } { }
    constexpr DeviceModel(const std::integral auto value) : model{ GetDeviceModelEnum(value) } { }
    constexpr DeviceModel(const DeviceModelEnum &model) : model{ model } { }
    constexpr DeviceModel(const DeviceModel &model) : model{ model.model } { }
    DeviceModel(const std::string_view &str) : model{ GetDeviceModelEnum(str) } { }
    DeviceModel(const std::string &str) : model{ GetDeviceModelEnum(str) } { }

    constexpr const std::string_view GetStringView() const {
    switch(model) {
#define DEVICE_TYPE_ENTRY(x) case DeviceModel::x: return { #x }; break;
        DEVICE_TYPE_LIST
#undef DEVICE_TYPE_ENTRY
        default:
            return { "Invalid" };
        }
    }

    constexpr bool IsValid() const { return static_cast<std::uint32_t>(model) > 0 && static_cast<std::uint32_t>(model) < out_of_range; }
    constexpr DeviceModel &operator=(const DeviceModel& rhs) { model = rhs.model; return *this; }
    constexpr bool operator==(const DeviceModel& rhs) { return model == rhs.model; }

    const std::string &GetString() const;
};

const std::unordered_map<std::string_view, DeviceModel::DeviceModelEnum> DeviceModel::devicemap {
#define DEVICE_TYPE_ENTRY(x) { std::string_view { #x }, DeviceModel::x },
    DEVICE_TYPE_LIST
#undef DEVICE_TYPE_ENTRY
};

const std::string DeviceModel::devicestr[] = {
#define DEVICE_TYPE_ENTRY(x) { #x },
        DEVICE_TYPE_LIST
#undef DEVICE_TYPE_ENTRY
};

const std::string &DeviceModel::GetString() const {
    if (model < std::size(devicestr)) {
        return devicestr[model];
    }
    else {
        return devicestr[0];
    }
}

class DeviceClient {
    DeviceModel model;
    rohit::client_socket_t client_socket;
    
public:
    DeviceClient(DeviceModel model, const rohit::ipv6_socket_addr_t &ipv6addr) : model{ model }, client_socket{ ipv6addr } { }

    bool PowerOn(std::uint32_t componentIndex);
    bool PowerOff(std::uint32_t componentIndex);

    bool BrigtnessDecrease(std::uint32_t componentIndex, std::uint16_t value);
    bool BrigtnessIncrease(std::uint32_t componentIndex, std::uint16_t value);

    bool SelectColor(std::uint32_t componentIndex, std::uint16_t red, std::uint16_t green, std::uint16_t blue, std::uint16_t gamma);

};

} //namespace rohit
#pragma once

#include "devices.hh"
#include <unordered_map>

namespace rohit {
namespace iot {

class devicemanager {
    std::unordered_map<guid_t, std::shared_ptr<device>> devicelist{ };

public:
    const device *GetDevice(const guid_t &id) const noexcept {
        auto itr = devicelist.find(id);
        if (itr == devicelist.end()) {
            return nullptr;
        }
        return itr->second.get();
    }

    bool RemoveDevice(const guid_t &id) noexcept {
        return devicelist.erase(id) == 0;
    }

    void AddDevice(const guid_t &id, std::shared_ptr<device> &dev) noexcept {
        devicelist.insert(std::make_pair(id, dev));
    }

};


} // namespace iot
} // namespace rohit
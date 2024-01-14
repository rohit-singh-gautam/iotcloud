#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <concepts>
#include <chrono>
#include "iot/message.hh"
#include "iot/core/guid.hh"

namespace rohit {
namespace iot {

class component {
    const std::uint32_t id;
    const std::string name;
    const std::string operation;
public:
    template <typename NameT, typename OperationT>
    component(const std::uint32_t id, NameT &&name, OperationT &&operation) :
        id{ id }, name{ std::forward(name) }, operation{ std::forward(operation) } { }
    
    auto GetID() const { return id; }
    auto &GetName() const { return name; }
    auto &GetOperation() const { return operation; }
};

class model {
    const std::uint32_t id;
    const std::string name;
    const std::string spec;
    const std::vector<component> components;
public:
    template <typename NameT, typename SpecT, typename ComponentT>
    model(const std::uint32_t id, NameT &&name, SpecT &&spec, ComponentT &&components) :
        id{ id }, name{ std::forward(name) }, component{ std::forward(components) } { }

    auto GetID() const { return id; }
    auto &GetName() const { return name; }
    auto &GetSpec() const { return spec; }

    auto begin() const { return components.begin(); }
    auto end() const { return components.end(); }

    auto &operator[](std::integral auto index) { return components[index]; }
};

class device {
    const guid_t guid;
    const model &mModel;

    // Public Key rarely change
    std::shared_ptr<char *> publicKeyBinary;

    std::shared_ptr<char *> privateKeyBinary;
    
    // symmetric key
    std::chrono::system_clock symetrickey_time;

public:
    device(const guid_t guid, const model &model, std::shared_ptr<char *> &publicKeyBinary) : guid{ guid }, mModel{ model }, publicKeyBinary{ publicKeyBinary } { }

    auto GetID() const { return guid; }
    auto &GetModel() const { return mModel; }
};



} // namespace iot
} // namespace rohit
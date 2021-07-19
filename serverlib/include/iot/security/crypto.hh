////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/types.hh>
#include <iot/core/error.hh>
#include <openssl/crypto.h>

namespace rohit {
namespace crypto {

enum class encryption_id_t : uint16_t {
    aes_256_gsm
};

struct mem {
    void    *ptr;
    size_t  size;
    constexpr mem(void *ptr = nullptr, size_t size = 0) : ptr(ptr), size(size) {}
};

struct openssl_mem : public mem {
    inline ~openssl_mem() { 
        if (ptr != nullptr) OPENSSL_free(ptr);
    }
};

struct malloc_mem : public mem {
    inline ~malloc_mem() { 
        if (ptr != nullptr) free(ptr);
    }
};

struct key_t {
    encryption_id_t id;
    constexpr key_t(encryption_id_t id) : id(id) {}
} __attribute__((packed));

struct key_aes_256_gsm_t : key_t {
    constexpr key_aes_256_gsm_t() : key_t(encryption_id_t::aes_256_gsm) {}
    uint8_t symetric_key[256/8];
} __attribute__((packed));

struct encrypted_data_aes_256_gsm_t {
    uint8_t initialization_vector[96/8];
    uint8_t tag[96/8];
    uint16_t data_size;

    static constexpr size_t const_size = sizeof(initialization_vector) + sizeof(tag) + sizeof(data_size);
    uint8_t data[0];
} __attribute__((packed));

// Parameters:
// data [in]: Plain binary data
// encrypted_data [out]: Encrypted data, memory allocated by openssl
err_t encrypt(const key_t &key, const mem &data, openssl_mem &encrypted_data);

// Parameters:
// data [in]: Encrypted data
// decrypted_data [out]: Decrypted data, memory allocated
err_t decrypt(const key_t &key, const mem &encrypt_data, openssl_mem &decrypted_data);


} // namespace crypto
} // namespace rohit
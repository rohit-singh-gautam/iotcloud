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

#include <iot/core/config.hh>
#include <iot/core/types.hh>
#include <iot/core/error.hh>
#include <iot/core/memory_helper.hh>
#include <openssl/crypto.h>
#include <openssl/ec.h>

namespace rohit {
namespace crypto {

enum class encryption_id_t : uint16_t {
    aes_256_gsm
};
struct openssl_mem : public mem<void> {
    inline ~openssl_mem() {
        OPENSSL_free(ptr);
    }
    void free() {
        OPENSSL_free(ptr);
        ptr = nullptr;
        size = 0;
    }

    template <typename inT>
    constexpr void *operator=(inT *rhs) {
        if constexpr (config::debug) {
            if (ptr != nullptr) {
                throw exception_t(err_t::CRYPTO_MEMORY_BAD_ASSIGNMENT);
            }
        }
        return ptr = (void *)rhs;
    }
};

struct openssl_ec_key_mem {
    EC_KEY *key;
    constexpr openssl_ec_key_mem(EC_KEY *key = nullptr) : key(key) {}
    inline ~openssl_ec_key_mem() {
        if (key != nullptr) EC_KEY_free(key);
    }

    void free() {
        EC_KEY_free(key);
        key = nullptr;
    }

    constexpr EC_KEY *operator=(EC_KEY *rhs) {
        if constexpr (config::debug) {
            if (key != nullptr) {
                throw exception_t(err_t::CRYPTO_MEMORY_BAD_ASSIGNMENT);
            }
        }
        return key = rhs;
    }
    constexpr bool operator==(void *rhs) const { return key == rhs; }
    constexpr operator EC_KEY *() const { return key; }
};

std::ostream& operator<<(std::ostream& os, const openssl_ec_key_mem &key);

struct key_t {
    encryption_id_t id;
    constexpr key_t(encryption_id_t id) : id(id) {}
} __attribute__((packed));

struct key_aes_256_gsm_t : key_t {
    static constexpr size_t key_size_in_bytes = 256/8;
    constexpr key_aes_256_gsm_t() : key_t(encryption_id_t::aes_256_gsm) {}
    uint8_t symetric_key[key_size_in_bytes];

    constexpr uint8_t *begin() { return symetric_key; }
    constexpr uint8_t *end() { return symetric_key + key_size_in_bytes; }
    constexpr const uint8_t *begin() const { return symetric_key; }
    constexpr const uint8_t *end() const { return symetric_key + key_size_in_bytes; }
} __attribute__((packed));

std::ostream& operator<<(std::ostream& os, const key_aes_256_gsm_t &key);

struct encrypted_data_aes_256_gsm_t {
    uint8_t aad[16];
    uint8_t initialization_vector[96/8];
    uint8_t tag[96/8];
    uint16_t data_size;

    static constexpr size_t const_size =
            sizeof(aad) + sizeof(initialization_vector) +
            sizeof(tag) + sizeof(data_size);
    uint8_t data[0];
} __attribute__((packed));

// Parameters:
// data [in]: Plain binary data
// encrypted_data [out]: Encrypted data, memory allocated by openssl
err_t encrypt(const key_t &key, const guid_t &random, const mem<void> &data, openssl_mem &encrypted_data);

// Parameters:
// data [in]: Encrypted data
// decrypted_data [out]: Decrypted data, memory allocated
err_t decrypt(const key_t &key, const mem<void> &encrypted_data, openssl_mem &decrypted_data);

// Allocation of key must be done to contain id
err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const int curve,
    const mem<void> &private_ec_key,
    const mem<void> &peer_public_ec_key,
    key_t &key);

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const int curve,
    const openssl_ec_key_mem &ec_private_key, // This is optimization
    const mem<void> &peer_public_ec_key,
    key_t &key);

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const int curve,
    const openssl_ec_key_mem &private_ec_key,
    const openssl_ec_key_mem &peer_public_ec_key,
    key_t &key);

err_t generate_ec_key(const int curve, openssl_ec_key_mem &ec_private_key);

err_t get_public_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &private_ec_key);
err_t get_private_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &public_ec_key);

err_t ec_get_curve(const openssl_ec_key_mem &ec_key, int &curve);

} // namespace crypto
} // namespace rohit
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

#include <iot/core/math.hh>
#include <iot/security/crypto.hh>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/core_names.h>
#include <openssl/encoder.h>
#include <openssl/err.h>
#include <memory>

namespace rohit {
namespace crypto {

constexpr char key_encoding[] = "DER";
using evp_ctx_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype([](EVP_PKEY_CTX *ptr) { EVP_PKEY_CTX_free(ptr); })>;

std::ostream& operator<<(std::ostream& os, const openssl_ec_key_mem &key) {
    std::string curve { };
    auto ret = ec_get_curve(key, curve);
    if (ret != err_t::SUCCESS) {
        return os << "Unable to find curve";
    }
    
    os << "(curve:" << curve;

    openssl_mem public_ec_key;
    ret = get_public_key_binary(key, public_ec_key);
    if (ret == err_t::SUCCESS) {
        os << ";public:" << public_ec_key;
    }

    openssl_mem private_ec_key;
    ret = get_private_key_binary(key, private_ec_key);
    if (ret == err_t::SUCCESS) {
        os << ";private:" << private_ec_key;
    }

    return os << ")";
}

std::ostream& operator<<(std::ostream& os, const key_aes_256_gsm_t &key) {
    os << "aes_256_gsm:";
    for(auto value: key) {
        os << upper_case_numbers[value/16] << upper_case_numbers[value%16];
    }
    return os;
}

err_t encrypt_aes_256_gsm(const key_aes_256_gsm_t &key, const guid_t &random, const mem<void> &data, openssl_mem &encrypted_data);

err_t decrypt_aes_256_gsm(
    const key_aes_256_gsm_t &key,
    const encrypted_data_aes_256_gsm_t &encrypted_data,
    openssl_mem &decrypted_data);

err_t encrypt(const key_t &key, const guid_t &random, const mem<void> &data, openssl_mem &encrypted_data) {
    switch(key.id) {
    case encryption_id_t::aes_256_gsm: {
        const key_aes_256_gsm_t * const aes_key = (const key_aes_256_gsm_t *)&key;
        return encrypt_aes_256_gsm(*aes_key, random, data, encrypted_data);
    }

    default:
        return err_t::CRYPTO_UNKNOWN_ALGORITHM;
    }
}

err_t decrypt(const key_t &key, const mem<void> &encrypted_data, openssl_mem &decrypted_data) {
    switch(key.id) {
    case encryption_id_t::aes_256_gsm: {
        const key_aes_256_gsm_t *aes_key = (const key_aes_256_gsm_t *)&key;
        encrypted_data_aes_256_gsm_t *aes_encrypt_data = (encrypted_data_aes_256_gsm_t *)encrypted_data.ptr;
        return decrypt_aes_256_gsm(*aes_key, *aes_encrypt_data, decrypted_data);
    }

    default:
        return err_t::CRYPTO_UNKNOWN_ALGORITHM;
    }
}

err_t encrypt_aes_256_gsm(const key_aes_256_gsm_t &key, const guid_t &random, const mem<void> &data, openssl_mem &encrypted_data) {
    encrypted_data.size =
        sizeof(encrypted_data_aes_256_gsm_t) + ((data.size + 0xf) & ~0xf);
    encrypted_data.ptr = OPENSSL_malloc(encrypted_data.size);

    if (encrypted_data.ptr == nullptr) return err_t::CRYPTO_MEMORY_FAILURE;

    encrypted_data_aes_256_gsm_t *encrypt_data = (encrypted_data_aes_256_gsm_t *)encrypted_data.ptr;

    err_t ret = err_t::SUCCESS;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) ret = err_t::CRYPTO_CREATE_CONTEXT_FAILED;

    if (ret == err_t::SUCCESS) {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) == 0)
            ret = err_t::CRYPTO_INIT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        RAND_bytes(encrypt_data->initialization_vector, sizeof(encrypted_data_aes_256_gsm_t::initialization_vector));
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, key.symetric_key, encrypt_data->initialization_vector) == 0)
            ret = err_t::CRYPTO_INIT_AES_FAILED;
    }

    int length;
    if (ret == err_t::SUCCESS) {
        std::copy((uint8_t *)&random, (uint8_t *)&random + sizeof(guid_t), encrypt_data->aad);
        if (EVP_EncryptUpdate(ctx, NULL, &length, encrypt_data->aad, sizeof(encrypted_data_aes_256_gsm_t::aad)) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        if (EVP_EncryptUpdate(ctx, encrypt_data->get_data(), &length, (const uint8_t *)data.ptr, data.size) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        encrypt_data->data_size = length;

        if (EVP_EncryptFinal_ex(ctx, (encrypt_data->get_data() + length), &length) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        encrypt_data->data_size += length;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(encrypted_data_aes_256_gsm_t::tag), encrypt_data->tag) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    EVP_CIPHER_CTX_free(ctx);
    if (ret != err_t::SUCCESS) {
        encrypted_data.free();
    }

    return ret;
}

err_t decrypt_aes_256_gsm(
    const key_aes_256_gsm_t &key,
    const encrypted_data_aes_256_gsm_t &encrypted_data,
    openssl_mem &decrypted_data)
{
    decrypted_data.ptr = (uint8_t *)OPENSSL_malloc(encrypted_data.data_size);
    if (decrypted_data.ptr == nullptr)  return err_t::CRYPTO_MEMORY_FAILURE;

    err_t ret = err_t::SUCCESS;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) ret = err_t::CRYPTO_CREATE_CONTEXT_FAILED;

    if (ret == err_t::SUCCESS) {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) == 0)
            ret = err_t::CRYPTO_INIT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        if (EVP_DecryptInit_ex(ctx, NULL, NULL, key.symetric_key, encrypted_data.initialization_vector) == 0)
            ret = err_t::CRYPTO_INIT_AES_FAILED;
    }

    int length;
    if (ret == err_t::SUCCESS) {
        if (EVP_DecryptUpdate(ctx, NULL, &length, encrypted_data.aad, sizeof(encrypted_data_aes_256_gsm_t::aad)) == 0)
            ret = err_t::CRYPTO_DECRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        if (EVP_DecryptUpdate(
                ctx,
                (uint8_t *)decrypted_data.ptr,
                &length,
                (const uint8_t *)encrypted_data.get_data(),
                encrypted_data.data_size) == 0)
            ret = err_t::CRYPTO_DECRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        decrypted_data.size = length;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, sizeof(encrypted_data_aes_256_gsm_t::tag), (void *)encrypted_data.tag) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        if (EVP_DecryptFinal_ex(ctx, ((uint8_t *)decrypted_data.ptr + length), &length) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        decrypted_data.size += length;
    }

    EVP_CIPHER_CTX_free(ctx);
    if (ret != err_t::SUCCESS) {
        decrypted_data.free();
    }

    return ret;
}

err_t ec_generate_private_key(const mem<void> &prikey_bin, openssl_ec_key_mem &private_key) {
    const unsigned char *ptr {reinterpret_cast<const unsigned char *>(prikey_bin.ptr)};
    private_key = d2i_AutoPrivateKey(nullptr, &ptr, prikey_bin.size);
    if (private_key == nullptr) {
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    return err_t::SUCCESS;
}

err_t ec_generate_public_key(const mem<void> &pubkey_bin, openssl_ec_key_mem &public_key) {
    const unsigned char *ptr {reinterpret_cast<const unsigned char *>(pubkey_bin.ptr)};
    public_key = d2i_PUBKEY(nullptr, &ptr, pubkey_bin.size);
    if (!public_key) {
        ERR_print_errors_fp(stdout);
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    return err_t::SUCCESS;
}

err_t get_aes_256_gsm_key_from_ec(
    const openssl_ec_key_mem &ec_private_key,
    const openssl_ec_key_mem &peer_public_ec_key,
    key_aes_256_gsm_t &key)
{
    evp_ctx_ptr ctx { EVP_PKEY_CTX_new(ec_private_key, nullptr) };
    if (!ctx) {
        ERR_print_errors_fp(stdout);
        return err_t::CRYPTO_BAD_PRIVATE_KEY;
    }

    if (EVP_PKEY_derive_init(ctx.get()) <= 0) {
        return err_t::CRYPTO_BAD_PRIVATE_KEY;
    }

    if (EVP_PKEY_derive_set_peer(ctx.get(), peer_public_ec_key) <= 0) {
        return err_t::CRYPTO_BAD_PUBLIC_KEY;
    }

    size_t symetric_key_len { };
    if (EVP_PKEY_derive(ctx.get(), nullptr, &symetric_key_len) <= 0) {
        return err_t::CRYPTO_BAD_SYMETRIC_KEY;
    }

    if (symetric_key_len != key_aes_256_gsm_t::key_size_in_bytes) {
        return err_t::CRYPTO_BAD_SYMETRIC_KEY;
    }

    if (EVP_PKEY_derive(ctx.get(), key.symetric_key, &symetric_key_len) <= 0) {
        return err_t::CRYPTO_BAD_SYMETRIC_KEY;
    }

    return err_t::SUCCESS;
}

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const openssl_ec_key_mem &private_ec_key, // This is optimization
    const openssl_ec_key_mem &peer_public_ec_key,
    key_t &key)
{
    switch(id) {
    case encryption_id_t::aes_256_gsm: {
        key_aes_256_gsm_t &aes_key = (key_aes_256_gsm_t &)key;
        return get_aes_256_gsm_key_from_ec(private_ec_key, peer_public_ec_key, aes_key);
    }

    default:
        return err_t::CRYPTO_UNKNOWN_ALGORITHM;
    }
}

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const mem<void> &private_ec_key,
    const mem<void> &peer_public_ec_key,
    key_t &key)
{
    openssl_ec_key_mem public_key;
    err_t ret = ec_generate_public_key(peer_public_ec_key, public_key);
    if (ret != err_t::SUCCESS) return ret;
    
    openssl_ec_key_mem private_key;
    ret = ec_generate_private_key(private_ec_key, private_key);
    if (ret != err_t::SUCCESS) return ret;

    return get_symmetric_key_from_ec(id, private_key, public_key, key);

}

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const openssl_ec_key_mem &private_ec_key, // This is optimization
    const mem<void> &peer_public_ec_key,
    key_t &key)
{
    openssl_ec_key_mem public_key;
    err_t ret = ec_generate_public_key(peer_public_ec_key, public_key);
    if (ret != err_t::SUCCESS) return ret;

    return get_symmetric_key_from_ec(id, private_ec_key, public_key, key);
}

err_t generate_ec_key(const char *curve, openssl_ec_key_mem &ec_private_key) {
    ec_private_key = EVP_EC_gen(curve);
    if (ec_private_key == nullptr) {
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }
    
    return err_t::SUCCESS;
}

err_t get_private_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &private_ec_key)
{
    // i2d_PrivateKey can be used here
    std::unique_ptr<OSSL_ENCODER_CTX, decltype([](OSSL_ENCODER_CTX *ptr) { OPENSSL_free(ptr); })>
        ossl_ctx { OSSL_ENCODER_CTX_new_for_pkey(
                        ec_key, EVP_PKEY_KEYPAIR,
                        key_encoding, nullptr, nullptr ) };
    
    if (!ossl_ctx) return err_t::CRYPTO_BAD_PUBLIC_KEY;

    if (!OSSL_ENCODER_to_data(
            ossl_ctx.get(),
            reinterpret_cast<unsigned char**>(&private_ec_key.ptr),
            &private_ec_key.size))
        return err_t::CRYPTO_BAD_PUBLIC_KEY;

    return err_t::SUCCESS;
}

err_t get_public_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &public_ec_key) {
    // i2d_PublicKey can be used here
    std::unique_ptr<OSSL_ENCODER_CTX, decltype([](OSSL_ENCODER_CTX *ptr) { OPENSSL_free(ptr); })>
        ossl_ctx { OSSL_ENCODER_CTX_new_for_pkey(
                        ec_key, EVP_PKEY_PUBLIC_KEY,
                        key_encoding, nullptr, nullptr ) };
    
    if (!ossl_ctx) return err_t::CRYPTO_BAD_PUBLIC_KEY;

    if (!OSSL_ENCODER_to_data(
            ossl_ctx.get(),
            reinterpret_cast<unsigned char**>(&public_ec_key.ptr),
            &public_ec_key.size))
        return err_t::CRYPTO_BAD_PUBLIC_KEY;

    return err_t::SUCCESS;
}

err_t ec_get_curve(const openssl_ec_key_mem &ec_key, std::string &curve) {
    char curve_name[64];
    size_t len { };
    if (!EVP_PKEY_get_utf8_string_param(ec_key, OSSL_PKEY_PARAM_GROUP_NAME,
                                    curve_name, sizeof(curve_name), &len)) {
        return err_t::CRYPTO_BAD_KEY;
    }

    curve.assign(curve_name, len);

    return err_t::SUCCESS;
}

} // namespace crypto
} // namespace rohit
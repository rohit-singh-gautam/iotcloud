////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/math.hh>
#include <iot/security/crypto.hh>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace rohit {
namespace crypto {

std::ostream& operator<<(std::ostream& os, const openssl_ec_key_mem &key) {
    int curve;
    auto ret = ec_get_curve(key, curve);
    if (ret != err_t::SUCCESS) {
        return os << "Unable to find curve";
    }
    
    os << "(curve:" << OBJ_nid2sn(curve);

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
        encrypted_data_aes_256_gsm_t::const_size + ((data.size + 0xf) & ~0xf);
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
        if (EVP_EncryptUpdate(ctx, encrypt_data->data, &length, (const uint8_t *)data.ptr, data.size) == 0)
            ret = err_t::CRYPTO_ENCRYPT_AES_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        encrypt_data->data_size = length;

        if (EVP_EncryptFinal_ex(ctx, (encrypt_data->data + length), &length) == 0)
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
                (const uint8_t *)encrypted_data.data,
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

err_t ec_generate_private_key(const int curve, const mem<void> &prikey_bin, openssl_ec_key_mem &private_key) {
    BIGNUM *pri_bn = BN_bin2bn((uint8_t *)prikey_bin.ptr, prikey_bin.size, nullptr);
    if (pri_bn == nullptr) {
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    err_t ret = err_t::SUCCESS;
    private_key = EC_KEY_new_by_curve_name(curve);
    if (private_key == nullptr) {
        ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    if (ret == err_t::SUCCESS) {
        if (!EC_KEY_set_private_key(private_key, pri_bn)) ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    BN_clear_free(pri_bn);
    
    if (ret != err_t::SUCCESS) {
        EC_KEY_free(private_key);
        private_key = nullptr;
    }

    return ret;
}

err_t ec_generate_public_key(const int curve, const mem<void> &pubkey_bin, openssl_ec_key_mem &public_key) {
    BIGNUM *pub_bn = BN_bin2bn((uint8_t *)pubkey_bin.ptr, pubkey_bin.size, nullptr);
    if (pub_bn == nullptr) {
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    err_t ret = err_t::SUCCESS;
    public_key = EC_KEY_new_by_curve_name(curve);
    if (public_key == nullptr) {
        ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    const EC_GROUP *ec_group = nullptr;
    if (ret == err_t::SUCCESS) {
        ec_group = EC_KEY_get0_group(public_key);
        if (ec_group == nullptr) {
            ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
        }
    }

    EC_POINT *pubkey_point = nullptr;
    if (ret == err_t::SUCCESS) {
        pubkey_point = EC_POINT_bn2point(ec_group, pub_bn, nullptr, nullptr);
        if (pubkey_point == nullptr) {
            ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
        }
    }

    if (ret == err_t::SUCCESS) {
        if (!EC_KEY_set_public_key(public_key, pubkey_point)) ret = err_t::CRYPTO_KEY_GENERATION_FAILED;
    }

    BN_clear_free(pub_bn);
    EC_POINT_free(pubkey_point);

    if (ret != err_t::SUCCESS) {
        EC_KEY_free(public_key);
        public_key = nullptr;
    }

    return ret;
}

err_t get_aes_256_gsm_key_from_ec(
    const openssl_ec_key_mem &ec_private_key,
    const openssl_ec_key_mem &peer_public_ec_key,
    key_aes_256_gsm_t &key)
{
    err_t ret = err_t::SUCCESS;

    const EC_POINT *peer_pubkey_point = nullptr;
    if (ret == err_t::SUCCESS) {
        peer_pubkey_point = EC_KEY_get0_public_key(peer_public_ec_key);
        if (peer_pubkey_point == nullptr) {
            ret = err_t::CRYPTO_BAD_PUBLIC_KEY;
        }
    }

    if (ret == err_t::SUCCESS) {
        auto length = ECDH_compute_key(
            key.symetric_key,
            sizeof(key_aes_256_gsm_t::symetric_key),
            peer_pubkey_point,
            ec_private_key,
            nullptr);
    }

    return ret;

}

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const int curve,
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
    const int curve,
    const mem<void> &private_ec_key,
    const mem<void> &peer_public_ec_key,
    key_t &key)
{
    openssl_ec_key_mem private_key;
    err_t ret = ec_generate_private_key(curve, private_ec_key, private_key);
    if (ret != err_t::SUCCESS) return ret;

    openssl_ec_key_mem public_key;
    ret = ec_generate_public_key(curve,peer_public_ec_key, public_key);
    if (ret != err_t::SUCCESS) return ret;

    return get_symmetric_key_from_ec(id, curve, private_key, public_key, key);

}

err_t get_symmetric_key_from_ec(
    const encryption_id_t id,
    const int curve,
    const openssl_ec_key_mem &private_ec_key, // This is optimization
    const mem<void> &peer_public_ec_key,
    key_t &key)
{
    openssl_ec_key_mem public_key;
    err_t ret = ec_generate_public_key(curve, peer_public_ec_key, public_key);
    if (ret != err_t::SUCCESS) return ret;

    return get_symmetric_key_from_ec(id, curve, private_ec_key, public_key, key);
}

err_t generate_ec_key(const int curve, openssl_ec_key_mem &ec_private_key) {
    ec_private_key = EC_KEY_new_by_curve_name(curve);
    if (ec_private_key == nullptr) {
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }
    if (!EC_KEY_generate_key(ec_private_key)) {
        EC_KEY_free(ec_private_key);
        ec_private_key = nullptr;
        return err_t::CRYPTO_KEY_GENERATION_FAILED;
    }
    return err_t::SUCCESS;
}

err_t get_private_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &private_ec_key)
{
    const BIGNUM * const prikey_num = EC_KEY_get0_private_key(ec_key);
    if (prikey_num == nullptr) {
        return err_t::CRYPTO_BAD_PRIVATE_KEY;
    }

    private_ec_key.size = BN_num_bytes(prikey_num);
    private_ec_key = OPENSSL_malloc(private_ec_key.size);

    err_t ret = err_t::SUCCESS;
    if (private_ec_key == nullptr) {
        ret = err_t::CRYPTO_MEMORY_FAILURE;
    }

    if (ret == err_t::SUCCESS) {
        if (BN_bn2bin(prikey_num, (uint8_t *)private_ec_key.ptr) != private_ec_key.size) {
            ret = err_t::CRYPTO_KEY_ENCODE_FAIL;
        }
    }

    if (ret != err_t::SUCCESS) {
        private_ec_key.free();
    }

    return ret;
}

err_t get_public_key_binary(const openssl_ec_key_mem &ec_key, openssl_mem &public_ec_key) {
    const EC_GROUP *ec_group   = EC_KEY_get0_group(ec_key);
    const EC_POINT *pub        = EC_KEY_get0_public_key(ec_key);

    if (ec_group == nullptr || pub == nullptr) {
        return err_t::CRYPTO_BAD_PUBLIC_KEY;
    }

    BN_CTX *pub_bn_ctx = BN_CTX_new();
    if (pub_bn_ctx == nullptr) return err_t::CRYPTO_MEMORY_FAILURE;
    
    BN_CTX_start(pub_bn_ctx);

    BIGNUM *pub_bn = EC_POINT_point2bn(ec_group, pub, POINT_CONVERSION_UNCOMPRESSED,
                        nullptr, pub_bn_ctx);

    err_t ret = err_t::SUCCESS;
    if (pub_bn == nullptr) ret = err_t::CRYPTO_MEMORY_FAILURE;

    if (ret == err_t::SUCCESS) {
        public_ec_key.size = BN_num_bytes(pub_bn);
        public_ec_key = (uint8_t *)OPENSSL_malloc(public_ec_key.size);
        if (public_ec_key == nullptr) ret = err_t::CRYPTO_MEMORY_FAILURE;
    }

    if (ret == err_t::SUCCESS) {
        if (BN_bn2bin(pub_bn, public_ec_key) != public_ec_key.size) {
            ret = err_t::CRYPTO_KEY_ENCODE_FAIL;
        }
    }

    BN_CTX_end(pub_bn_ctx);
    BN_CTX_free(pub_bn_ctx);
    if (pub_bn != nullptr) BN_clear_free(pub_bn);

    if (ret != err_t::SUCCESS) {
        public_ec_key.free();
    }

    return ret;
}

err_t ec_get_curve(const openssl_ec_key_mem &ec_key, int &curve) {
    const EC_GROUP *ec_group   = EC_KEY_get0_group(ec_key);
    if (ec_group == nullptr) {
        return err_t::CRYPTO_BAD_KEY;
    }

    curve = EC_GROUP_get_curve_name(ec_group);
    if (!curve) return err_t::CRYPTO_CURVE_NOT_FOUND;

    return err_t::SUCCESS;
}

} // namespace crypto
} // namespace rohit
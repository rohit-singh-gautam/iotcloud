////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/security/crypto.hh>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace rohit {
namespace crypto {

// Parameters:
// data [in]: Plain binary data
// encrypted_data [out]: Encrypted data, memory allocated by openssl
err_t encrypt_aes_256_gsm(const key_aes_256_gsm_t &key, const guid_t &random, const mem &data, openssl_mem &encrypted_data);

// Parameters:
// data [in]: Encrypted data
// decrypted_data [out]: Decrypted data, memory allocated
err_t decrypt_aes_256_gsm(
    const key_aes_256_gsm_t &key,
    const encrypted_data_aes_256_gsm_t &encrypted_data,
    openssl_mem &decrypted_data);

err_t encrypt(const key_t &key, const guid_t &random, const mem &data, openssl_mem &encrypted_data) {
    switch(key.id) {
    case encryption_id_t::aes_256_gsm: {
        const key_aes_256_gsm_t * const aes_key = (const key_aes_256_gsm_t *)&key;
        return encrypt_aes_256_gsm(*aes_key, random, data, encrypted_data);
    }

    default:
        return err_t::CRYPTO_UNKNOWN_ALGORITHM;
    }
}

err_t decrypt(const key_t &key, const mem &encrypted_data, openssl_mem &decrypted_data) {
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

err_t encrypt_aes_256_gsm(const key_aes_256_gsm_t &key, const guid_t &random, const mem &data, openssl_mem &encrypted_data) {
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

    if (ctx != nullptr) EVP_CIPHER_CTX_free(ctx);
    if (ret != err_t::SUCCESS) {
        OPENSSL_free(encrypted_data.ptr);
        encrypted_data.ptr = nullptr;
        encrypted_data.size = 0;
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

    if (ctx != nullptr) EVP_CIPHER_CTX_free(ctx);
    if (ret != err_t::SUCCESS) {
        OPENSSL_free(decrypted_data.ptr);
        decrypted_data.ptr = nullptr;
        decrypted_data.size = 0;
    }

    return ret;
}

} // namespace crypto
} // namespace rohit
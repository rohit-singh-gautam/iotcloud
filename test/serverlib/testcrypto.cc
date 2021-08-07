////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/security/crypto.hh>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/pem.h>

uint16_t success = 0;
uint16_t failure = 0;

void test_aes_256_gsm() {
    std::cout << "=========== Testing AES 256 GSM" << std::endl;
    rohit::crypto::key_aes_256_gsm_t key;
    uint8_t random_value[16];
    
    // Creating random symmetric key
    RAND_bytes(key.symetric_key, sizeof(key.symetric_key));
    RAND_bytes(random_value, sizeof(random_value));
    rohit::guid_t random_guid(random_value);

    constexpr char message[] = "This is a test.";
    rohit::mem data = {(void *)message, sizeof(message)};

    rohit::crypto::openssl_mem encrypted_data;
    rohit::err_t ret = encrypt(key, random_guid, data, encrypted_data);

    if (ret != rohit::err_t::SUCCESS) {
        std::cout << "Failed to encrypt data with err: " << ret;
        ++failure;
        return;
    }

    rohit::crypto::openssl_mem decrypted_data;
    ret = decrypt(key, encrypted_data, decrypted_data);

    if (ret != rohit::err_t::SUCCESS) {
        std::cout << "Failed to decrypt data with err: " << ret;
        ++failure;
        return;
    }

    std::cout << "Decrypted data: " << (char *)decrypted_data.ptr << std::endl;
    ++success;
}

void test_ec_aes_256_gsm() {
    std::cout << "=========== Testing EC_KEY AES 256 GSM" << std::endl;
    using namespace rohit;
    using namespace rohit::crypto;

    constexpr char curve_name[] = "prime256v1";
    //const int curve = EC_curve_nist2nid(curve_name);
    const int curve = OBJ_txt2nid(curve_name);

    if (curve == 0) {
        ERR_print_errors_fp(stderr);
        std::cout << "Failed to get curve" << std::endl;
        ++failure;
        return;
    }

    std::cout << "Curve:" << curve << std::endl;

    openssl_ec_key_mem server_key;
    auto ret = generate_ec_key(curve, server_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to create server key error: " << ret << std::endl;
        ++failure;
        return;
    }

    openssl_mem server_public_key;
    ret = get_public_key_binary(server_key, server_public_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to retrieve public key from server key error: " << ret << std::endl;
        ++failure;
        return;
    }

    openssl_mem server_private_key;
    ret = get_private_key_binary(server_key, server_private_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to retrieve private key from server key error: " << ret << std::endl;
        ++failure;
        return;
    }

    openssl_ec_key_mem client_key;
    ret = generate_ec_key(curve, client_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to create client key error: " << ret << std::endl;
        ++failure;
        return;
    }

    openssl_mem client_public_key;
    ret = get_public_key_binary(client_key, client_public_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to retrieve public key from client key error: " << ret << std::endl;
        ++failure;
        return;
    }

    openssl_mem client_private_key;
    ret = get_private_key_binary(client_key, client_private_key);
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to retrieve private key from client key error: " << ret << std::endl;
        ++failure;
        return;
    }

    std::cout << "Server certificate: " << server_key << std::endl;
    std::cout << "Client certificate: " << client_key << std::endl;

    key_aes_256_gsm_t client_symmetric_key;
    ret = get_symmetric_key_from_ec(
        encryption_id_t::aes_256_gsm,
        curve,
        client_private_key,
        server_public_key,
        client_symmetric_key
    );
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to create client symmetric key error: " << ret << std::endl;
        ++failure;
        return;
    }
    std::cout << "Client symmetric Key - " << client_symmetric_key << std::endl;

    key_aes_256_gsm_t server_symmetric_key;
    ret = get_symmetric_key_from_ec(
        encryption_id_t::aes_256_gsm,
        curve,
        server_private_key,
        client_public_key,
        server_symmetric_key
    );
    if (ret != err_t::SUCCESS) {
        std::cout << "Failed to create server symmetric key error: " << ret << std::endl;
        ++failure;
        return;
    }
    std::cout << "Server symmetric Key - " << server_symmetric_key << std::endl;

    uint8_t random_value[16];
    RAND_bytes(random_value, sizeof(random_value));
    rohit::guid_t random_guid(random_value);

    constexpr char message[] = "This is a EC_KEY test.";
    rohit::mem data = {(void *)message, sizeof(message)};

    rohit::crypto::openssl_mem encrypted_data;
    ret = encrypt(client_symmetric_key, random_guid, data, encrypted_data);

    if (ret != rohit::err_t::SUCCESS) {
        std::cout << "Failed to encrypt data with err: " << ret;
        ++failure;
        return;
    }

    rohit::crypto::openssl_mem decrypted_data;
    ret = decrypt(server_symmetric_key, encrypted_data, decrypted_data);

    if (ret != rohit::err_t::SUCCESS) {
        std::cout << "Failed to decrypt data with err: " << ret;
        ++failure;
        return;
    }

    std::cout << "Decrypted data: " << (char *)decrypted_data.ptr << std::endl;

    
    ++success;
}




int main() {
    test_aes_256_gsm();
    test_ec_aes_256_gsm();

    std::cout << "Summary: success(" << success << "), failure(" << failure << ")" << std::endl;
    return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/security/crypto.hh>
#include <openssl/rand.h>

uint16_t success = 0;
uint16_t failure = 0;

void test_aes_256_gsm() {
    rohit::crypto::key_aes_256_gsm_t key;
    
    // Creating random symetric key
    RAND_bytes(key.symetric_key, sizeof(key.symetric_key));

    constexpr char message[] = "This is a test.";
    rohit::crypto::mem data = {(void *)message, sizeof(message)};

    rohit::crypto::openssl_mem encrypted_data;
    rohit::err_t ret = encrypt(key, data, encrypted_data);

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


int main() {
    test_aes_256_gsm();

    std::cout << "Summary: success(" << success << "), failure(" << failure << ")" << std::endl;
    return EXIT_SUCCESS;
}
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

    std::cout << "Curve:" << curve_name << std::endl;

    openssl_ec_key_mem server_key;
    auto ret = generate_ec_key(curve_name, server_key);
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
    ret = generate_ec_key(curve_name, client_key);
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
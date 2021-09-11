////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <hpack.hh>

namespace rohit::http::v2 {

const node *huffman_root = created_huffman_tree();

node *created_huffman_tree() {
    node *root = new node();
    constexpr uint16_t symbol_count = sizeof(static_huffman)/sizeof(static_huffman[0]);
    for(uint16_t symbol = 0; symbol < symbol_count; ++symbol) {
        auto symbol_entry = static_huffman[symbol];
        uint32_t code = symbol_entry.code;
        node *curr = root;
        for(uint32_t bit = 1 << (symbol_entry.code_len - 1); bit; bit >>= 1) {
            if ((code & bit) == 0) {
                if (curr->left == nullptr) {
                    curr->left = new node();
                }
                curr = curr->left;
            } else {
                if (curr->right == nullptr) {
                    curr->right = new node();
                }
                curr = curr->right;
            }
        }
        if (curr->left != nullptr || curr->right != nullptr) {
            std::cout << "Leaf node not null symbol " << symbol << std::endl;
        }
        curr->set_leaf(symbol);
    }

    return root;
}

std::string get_huffman_string(const uint8_t *pstart, const uint8_t *pend) {
    std::string value;
    const node *curr = huffman_root;
    while(pstart < pend) {
        for(uint8_t bit = 128; bit; bit >>= 1) {
            if ((*pstart & bit) == 0) {
                curr = curr-> left;
            } else {
                curr = curr->right;
            }

            if ((curr->left == nullptr || curr->right == nullptr) && !(curr->left == nullptr && curr->right == nullptr)) {
                std::cout << "Only one of left or right is nullptr " << std::endl;
            }

            if (curr->is_leaf()) {
                if (curr->is_eos()) return value;
                value.push_back(curr->get_symbol());

                curr = huffman_root;
            }
        }

        ++pstart;
    }

    return value;
}

} // namespace rohit::http::v2
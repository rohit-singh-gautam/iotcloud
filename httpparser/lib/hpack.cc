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

uint8_t *add_huffman_string(uint8_t *pstart, const uint8_t *pvalue_start, const uint8_t *const pvalue_end) {
    constexpr uint8_t mask[] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
    uint8_t bit_index = 0;
    *pstart = 0; // Clean all existing value
    for(;pvalue_start < pvalue_end; ++pvalue_start) {
        const auto &entry = static_huffman[*pvalue_start];
        auto len_left = entry.code_len;
        auto code = entry.code;
        while (true) {
            if (bit_index + len_left <= 8) {
                *pstart += (code << (8 - bit_index - len_left)) & mask[bit_index];
                bit_index += len_left;
                if (bit_index == 8) {
                    bit_index = 0;
                    ++pstart;
                    *pstart = 0;
                }
                break;
            } else {
                // This will copy atleast one bit
                *pstart += (code >> (len_left + bit_index - 8)) & mask[bit_index];
                len_left -= 8 - bit_index;
                bit_index = 0;
                ++pstart;
                *pstart = 0;
            }
        }
    }
    
    if (bit_index) {
        *pstart += mask[bit_index];
        pstart++;
    }

    return pstart;
}

const static_table_t static_table = {
#define HTTP2_STATIC_TABLE_ENTRY(x, y, z) {y, z},
    HTTP2_STATIC_TABLE_LIST
#undef HTTP2_STATIC_TABLE_ENTRY
};

} // namespace rohit::http::v2
////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/core/memory_helper.hh>
#include <iot/core/math.hh>

namespace rohit {

std::ostream& operator<<(std::ostream& os, const mem &binary) {
    for(auto value: binary) {
        os << upper_case_numbers[value/16] << upper_case_numbers[value%16];
    }
    return os;
}

} // namespace rohit
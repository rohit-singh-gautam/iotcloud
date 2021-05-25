#include <iot/core/error.hh>

namespace rohit {

const error_str error_t::displayString[] {
#define ERROR_T_ENTRY(x, y) {#x, y},
    ERROR_T_SUCCESSTYPE
    ERROR_T_FAILURETYPE
#undef ERROR_T_ENTRY

    {"MAX_FAILURE", "Max Failure no entry must be made beyond this"},
};

}
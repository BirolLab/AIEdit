#include "str_utils.hpp"

#include <math.h>
#include <sstream>
#include <string>

namespace str_utils {

std::string human_readable(size_t bytes)
{
    unsigned o = 0;
    std::ostringstream ss;
    double mantissa = bytes;
    while (mantissa >= 1024) {
        mantissa /= 1024.;
        ++o;
    }
    ss << std::ceil(mantissa * 10.) / 10. << "BKMGTPE"[o];
    ss << (o > 0 ? "B" : "");
    return ss.str();
}

};  // namespace str_utils
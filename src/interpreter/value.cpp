#include <sstream>

#include "interpreter/value.h"

namespace Asampl::Interpreter {

std::string Tuple::to_string() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < values.size(); i++) {
        ss << values[i]->to_string();
        if (i != values.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

bool Tuple::operator==(const Tuple& other) const {
    if (values.size() != other.values.size()) {
        return false;
    }

    for (size_t i = 0; i < values.size(); i++) {
        if (*values[i] != *other.values[i]) {
            return false;
        }
    }

    return true;
}

std::string Map::to_string() const {
    return "<MAP> (TODO)";
}

std::string ByteArray::to_string() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < data.size(); i++) {
        ss << data[i];
        if (i != data.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

}

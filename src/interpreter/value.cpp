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

void Map::set(const ValuePtr& key, const ValuePtr& value) {
    if (key->is<String>()) {
        set(key->get<String>().value, value);
    } else {
        throw InterpreterException("Invalid key " + key->to_string() + ": keys other than strings are not currently supported");
    }
}

ValuePtr& Map::get(const ValuePtr& key) {
    if (key->is<String>()) {
        return get(key->get<String>().value);
    } else {
        throw InterpreterException("Invalid key " + key->to_string() + ": keys other than strings are not currently supported");
    }
}

bool Map::has(const ValuePtr& key) {
    if (key->is<String>()) {
        return has(key->get<String>().value);
    } else {
        throw InterpreterException("Invalid key " + key->to_string() + ": keys other than strings are not currently supported");
    }
}

void Map::set(const std::string& key, const ValuePtr& value) {
    string_map[key] = value;
}

ValuePtr& Map::get(const std::string& key) {
    return string_map[key];
}

ValuePtr& Map::strict_get(const std::string& key) {
    const auto it = string_map.find(key);
    if (it == string_map.end()) {
        throw InterpreterException("Map has no required key " + key);
    }

    return it->second;
}

bool Map::has(const std::string& key) {
    return string_map.find(key) != string_map.end();
}

void Map::set(const char* key, const ValuePtr& value) {
    set(std::string{key}, value);
}

ValuePtr& Map::get(const char* key) {
    return get(std::string{key});
}

bool Map::has(const char* key) {
    return has(std::string{key});
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

#pragma once

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace Asampl::Utils {

struct SliceOutOfRange : public std::range_error {
    SliceOutOfRange()
        : std::range_error{"slice index out of range"}
    {
    }
};

template< typename T >
class Slice {
public:
    Slice(T* ptr, size_t size)
        : ptr{ptr}
        , _size{size}
    {
    }

    Slice(std::vector<T>& vec)
        : ptr{vec.data()}
        , _size{vec.size()}
    {
    }

    size_t size() const {
        return _size;
    }

    T& operator[](size_t i) {
        if (i < size()) {
            return ptr[i];
        } else {
            throw SliceOutOfRange{};
        }
    }

    T& head() {
        return (*this)[0];
    }

    Slice tail() {
        return slice_from(1);
    }

    Slice slice(size_t start, size_t end) {
        assert(end >= start);
        if (start <= size() && end <= size()) {
            return Slice{ptr + start, end - start};
        } else {
            throw SliceOutOfRange();
        }
    }

    Slice slice_from(size_t start) {
        return slice(start, size());
    }

    Slice slice_until(size_t end) {
        return slice(0, end);
    }

    T* begin() {
        return ptr;
    }

    T* end() {
        return ptr + _size;
    }

    void copy_to_vector(std::vector<T>& vec)
    {
        vec.clear();
        for (auto& elem : *this)
        {
            vec.push_back(elem);
        }
    }

private:
    T* ptr;
    size_t _size;
};

}

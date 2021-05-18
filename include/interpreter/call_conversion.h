#pragma once

#include <vector>

#include "interpreter/exception.h"
#include "interpreter/value.h"

namespace Asampl::Interpreter::FuncCall {

template< typename T >
struct TupleOf {
    std::vector<T> values;
};

template< typename Request >
struct CallConversion {
    using Result = Request&;

    Result operator()(Request& in) {
        return in;
    }

    template< typename T >
    Result operator()(T& in) {
        throw InterpreterException("aaa");
    }
};

//template< typename T >
//struct CallConversion<TupleOf<T>> {
    //using Result = TupleOf<T>;

    //static Result convert() {
        //auto& tuple = in.
    //}
//};

template< typename Conversion >
decltype(auto) convert_argument(ValuePtr& x) {
    if constexpr (std::is_same_v<std::decay_t<Conversion>, ValuePtr>) {
        return x;
    } else {
        Conversion conversion{};
        return x->apply(conversion);
    }
}

}

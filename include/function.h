#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <vector>
#include <memory>
#include <tuple>

#include <iostream>

#include "interpreter.h"

using Function = std::function<ValuePtr(const std::vector<ValuePtr>&)>;

template<std::size_t first_arg, typename ArgsArray, typename T, typename... Ts>
std::tuple<T&, Ts&...> convert_arguments(ArgsArray& args) {
    if constexpr (sizeof...(Ts) == 0) {
        if constexpr (std::is_same_v<T, ValuePtr>) {
            return std::tie(args[first_arg]);
        } else {
            return std::tie(args[first_arg]->template try_get<T>());
        }
    } else {
        return std::tuple_cat(
            std::tie((args[first_arg])),
            convert_arguments<first_arg + 1, ArgsArray, Ts...>(args)
        );
    }
}

template<typename R, typename... Args>
Function make_asampl_function(std::function<R(Args...)> func) {
    return [func](const std::vector<ValuePtr>& args) -> ValuePtr {
        assert(args.size() >= sizeof...(Args));

        std::array<ValuePtr, sizeof...(Args)> args_array;
        for (std::size_t i = 0; i < sizeof...(Args); i++) {
            args_array[i] = std::move(args[i]);
        }

        auto args_tuple = convert_arguments<0, decltype(args_array), Args...>(args_array);
        if constexpr (std::is_same_v<R, void>) {
            std::apply(func, std::move(args_tuple));
            return std::make_shared<UndefinedValue>();
        } else if constexpr (std::is_same_v<R, ValuePtr>) {
            return std::apply(func, std::move(args_tuple));
        } else {
            return std::make_shared<Value<R>>(std::apply(func, std::move(args_tuple)));
        }
    };
}

#pragma once

#include <interpreter/value.h>

extern "C" {
#include <asampl-ffi/types.h>
}


AsaValueContainer convert_to_ffi(const ValuePtr& value);
ValuePtr convert_from_ffi(const AsaValueContainer& value);

#ifndef _VT_H
#define _VT_H

enum class variant_type {
    INT = 0,
    STRING,
};

using vt = variant_type;

#define get_vt(t, member) ((tvt)((t)->member.index()))
#define get_str_val(t, member) (std::get<std::string>((t)->member))
#define get_int_val(t, member) (std::get<int>((t)->member))

#endif /* _VT_H */
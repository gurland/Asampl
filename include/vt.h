#ifndef _VT_H
#define _VT_H

enum class variant_type {
    INT = 0,
    STRING,
    FLOAT,
};

using vt = variant_type;

#define get_vt(t, member) ((vt)((t)->member.index()))
#define get_str_val(t, member) (std::get<std::string>((t)->member))
#define get_int_val(t, member) (std::get<long>((t)->member))
#define get_flt_val(t, member) (std::get<double>((t)->member))

#endif /* _VT_H */

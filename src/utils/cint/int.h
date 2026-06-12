#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#ifndef JF_INT_TYPEDEFS_DEFINED
#define JF_INT_TYPEDEFS_DEFINED
using i8  = int8_t;   using ui8  = uint8_t;
using i16 = int16_t;  using ui16 = uint16_t;
using i32 = int32_t;  using ui32 = uint32_t;
using i64 = int64_t;  using ui64 = uint64_t;
using i128  = __int128_t;
using ui128 = __uint128_t;
#endif

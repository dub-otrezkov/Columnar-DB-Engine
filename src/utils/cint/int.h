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


template <typename T>
class TIntPrinter {
public:
    static std::string IntToJfStr(T i) {
        std::string ans(sizeof(T), 'k');
        char mas[sizeof(T)];
        memcpy(mas, &i, sizeof(T));
        for (ui8 j = 0; j < sizeof(T); j++) {
            ans[j] = mas[j];
        }
        return ans;
    }

    static T JfStrToInt(std::string_view in) {
        T ans = 0;
        char mas[sizeof(T)];
        for (ui8 i = 0; i < sizeof(T); i++) {
            mas[i] = in[i];
        }
        memcpy(&ans, mas, sizeof(T));
        return ans;
    }
};


inline auto I8ToJfStr  = TIntPrinter<i8 >::IntToJfStr;
inline auto I16ToJfStr = TIntPrinter<i16>::IntToJfStr;
inline auto I32ToJfStr = TIntPrinter<i32>::IntToJfStr;
inline auto I64ToJfStr = TIntPrinter<i64>::IntToJfStr;

inline auto JfStrToI8  = TIntPrinter<i8 >::JfStrToInt;
inline auto JfStrToI16 = TIntPrinter<i16>::JfStrToInt;
inline auto JfStrToI32 = TIntPrinter<i32>::JfStrToInt;
inline auto JfStrToI64 = TIntPrinter<i64>::JfStrToInt;

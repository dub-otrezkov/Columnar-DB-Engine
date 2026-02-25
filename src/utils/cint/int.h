#pragma once

#include <iostream>
#include <sstream>

#include <cstring>
#include <cstdint>

using i8 = int8_t;
using ui8 = uint8_t;

using i16 = int16_t;
using ui16 = uint16_t;

using i32 = int32_t;
using ui32 = uint32_t;

using i64 = int64_t;
using ui64 = uint64_t;


template <typename T>
class TIntPrinter {
public:
    static void PutInt(std::ostream& out, T i) {
        static char mas[sizeof(T)];
        memcpy(mas, &i, sizeof(T));
        for (ui8 i = 0; i < sizeof(T); i++) {
            out << mas[i];
        }
    }

    static std::string IntToJFStr(T i) {
        std::string ans(sizeof(T), 'k');
        static char mas[sizeof(T)];
        memcpy(mas, &i, sizeof(T));
        for (ui8 i = 0; i < sizeof(T); i++) {
            ans[i] = mas[i];
        }

        return std::move(ans);
    }

    static T JFStrToInt(std::string_view in) {
        T ans = 0;
        static char mas[sizeof(T)];
        for (ui8 i = 0; i < sizeof(T); i++) {
            mas[i] = in[i];
        }
        memcpy(&ans, mas, sizeof(T));
        return ans;
    }

    static T ReadInt(std::istream& in) {
        T ans = 0;
        static char mas[sizeof(T)];
        for (ui8 i = 0; i < sizeof(T); i++) {
            mas[i] = in.get();
        }
        memcpy(&ans, mas, sizeof(T));
        return ans;
    }
};

static auto PutI8 = TIntPrinter<i8>::PutInt;
static auto ReadI8 = TIntPrinter<i8>::ReadInt;
static auto I8ToJFStr = TIntPrinter<i8>::IntToJFStr;
static auto JFStrToI8 = TIntPrinter<i8>::JFStrToInt;

static auto PutI16 = TIntPrinter<i16>::PutInt;
static auto ReadI16 = TIntPrinter<i16>::ReadInt;
static auto I16ToJFStr = TIntPrinter<i16>::IntToJFStr;
static auto JFStrToI16 = TIntPrinter<i16>::JFStrToInt;

static auto PutI32 = TIntPrinter<i32>::PutInt;
static auto ReadI32 = TIntPrinter<i32>::ReadInt;
static auto I32ToJFStr = TIntPrinter<i32>::IntToJFStr;
static auto JFStrToI32 = TIntPrinter<i32>::JFStrToInt;

static auto PutI64 = TIntPrinter<i64>::PutInt;
static auto ReadI64 = TIntPrinter<i64>::ReadInt;
static auto I64ToJFStr = TIntPrinter<i64>::IntToJFStr;
static auto JFStrToI64 = TIntPrinter<i64>::JFStrToInt;

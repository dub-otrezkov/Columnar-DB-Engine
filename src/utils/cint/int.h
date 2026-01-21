#pragma once

#include <iostream>

using i64 = long long;
using i32 = int;

using ui64 = unsigned long long;
using ui32 = unsigned int;

void PutI64(std::ostream& out, i64 i);
std::string I64ToJFStr(i64 i);
i64 JFStrToI64(const std::string& i);
i64 ReadI64(std::istream& in);

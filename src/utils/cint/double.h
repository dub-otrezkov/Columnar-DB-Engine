#pragma once

#include "int.h"

#include <iostream>
#include <sstream>

using ld = long double;

void PutDouble(std::ostream& out, ld i);
std::string DoubleToJfStr(ld i);
ld ReadDouble(std::istream& in);
ld JfStrToDouble(const std::string& i);

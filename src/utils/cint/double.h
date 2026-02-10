#pragma once

#include "int.h"

#include <iostream>
#include <sstream>

using ld = long double;

void PutDouble(std::ostream& out, ld i);
std::string DoubleToJFStr(ld i);
ld ReadDouble(std::istream& in);
ld JFStrToDouble(const std::string& i);

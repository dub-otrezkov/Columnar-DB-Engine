#include "../operators.h"
#include "../agregator.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string_view>

namespace JFEngine::Testing {

struct AgregationsTest : testing::Test {
    std::string scheme = R"(red,string
hot,int64
chili,string
peppers,int32
what,int16
once,int8
was,double
)";
    std::string data = R"(josh,1,klinghoffer,2,1,2,0
john,3,frusciante,4,-10,-1,1.2
stadium,5,arcadium,6,0,0,-1.2
"i,could,have,lied",6,919,0,5,-5,82
cant,14,stop,52,-4,11,18
the,9,afterglow,9,40,81,1.8
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
dot,19,hacker,-10,-10,-1,-1.125
)";
};

} // namespace JFEngine::Testing

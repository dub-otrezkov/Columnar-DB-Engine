#include "../executor.h"
#include "ios_factory/ios_factory.h"

#include <gtest/gtest.h>

#include <memory>
#include <string_view>

namespace JfEngine::Testing {

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

    std::shared_ptr<std::stringstream> out_scheme;
    std::shared_ptr<std::stringstream> out_data;

    void SetUp() override {
        TIoFactory::RegisterSStreamIo("scheme", ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo("data", ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo("josh", ETypeFile::kJfFile);

        *TIoFactory::GetIo("scheme") << scheme;
        *TIoFactory::GetIo("data") << data;

        TIoFactory::RegisterSStreamIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo(kResultData, ETypeFile::kCsvFile);

        out_scheme = std::dynamic_pointer_cast<std::stringstream>(
            TIoFactory::GetIo(kResultScheme)
        );
        out_data = std::dynamic_pointer_cast<std::stringstream>(
            TIoFactory::GetIo(kResultData)
        );
    }

    void TearDown() override {
        TIoFactory::UnregisterIo("scheme");
        TIoFactory::UnregisterIo("data");
        TIoFactory::UnregisterIo("josh");

        TIoFactory::UnregisterIo(kResultData);
        TIoFactory::UnregisterIo(kResultScheme);
    }
};

} // namespace JfEngine::Testing

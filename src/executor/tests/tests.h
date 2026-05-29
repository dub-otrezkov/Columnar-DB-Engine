#include "../executor.h"
#include "ios_factory/ios_factory.h"
#include "utils/mmap_input/mmap_input.h"

#include <gtest/gtest.h>

#include <memory>

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

    std::shared_ptr<TStringStreamFileOutput> out_scheme;
    std::shared_ptr<TStringStreamFileOutput> out_data;

    void SetUp() override {
        TIoFactory::RegisterCustomInput("scheme", std::make_shared<TStringStreamFileInput>(scheme));
        TIoFactory::RegisterCustomInput("data",   std::make_shared<TStringStreamFileInput>(data));
        TIoFactory::RegisterSStreamIo("josh", ETypeFile::kJfFile);

        out_scheme = std::make_shared<TStringStreamFileOutput>();
        out_data   = std::make_shared<TStringStreamFileOutput>();
        TIoFactory::RegisterCustomOutput(kResultScheme, out_scheme);
        TIoFactory::RegisterCustomOutput(kResultData,   out_data);
    }

    void TearDown() override {
        TIoFactory::UnregisterIo("scheme");
        TIoFactory::UnregisterIo("data");
        TIoFactory::UnregisterIo("josh");

        TIoFactory::UnregisterIo(kResultData);
        TIoFactory::UnregisterIo(kResultScheme);
    }
};

struct SumOverflowTest : testing::Test {
    std::string scheme = "val,int64\n";
    // 2 * 2^62 = 2^63 = INT64_MAX + 1, overflows int64 but fits in int128
    std::string data = "4611686018427387904\n4611686018427387904\n";

    std::shared_ptr<TStringStreamFileOutput> out_scheme;
    std::shared_ptr<TStringStreamFileOutput> out_data;

    void SetUp() override {
        TIoFactory::RegisterCustomInput("scheme", std::make_shared<TStringStreamFileInput>(scheme));
        TIoFactory::RegisterCustomInput("data",   std::make_shared<TStringStreamFileInput>(data));
        TIoFactory::RegisterSStreamIo("ovf", ETypeFile::kJfFile);

        out_scheme = std::make_shared<TStringStreamFileOutput>();
        out_data   = std::make_shared<TStringStreamFileOutput>();
        TIoFactory::RegisterCustomOutput(kResultScheme, out_scheme);
        TIoFactory::RegisterCustomOutput(kResultData,   out_data);
    }

    void TearDown() override {
        TIoFactory::UnregisterIo("scheme");
        TIoFactory::UnregisterIo("data");
        TIoFactory::UnregisterIo("ovf");
        TIoFactory::UnregisterIo(kResultData);
        TIoFactory::UnregisterIo(kResultScheme);
    }
};

} // namespace JfEngine::Testing

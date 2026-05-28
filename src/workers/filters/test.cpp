#include "filter.h"

#include "engine/engine.h"
#include "utils/mmap_input/mmap_input.h"

#include <gtest/gtest.h>

namespace JfEngine::Testing {

struct FiltersTest : testing::Test {
    std::string scheme = R"(red,string
hot,int64
chili,string
peppers,int32
what,int16
once,int8
was,double
)";
    std::string data = R"(josh,1,klinghoffer,2,1,2,0
john,9,frusciante,4,-10,-1,1.2
stadium,5,arcadium,6,0,0,-1.2
"i,could,have,lied",6,919,0,5,-5,82
cant,14,stop,52,-4,11,18
the,9,afterglow,9,40,81,1.8
dot,19,hacker,10,82,82,0
dot,9,hacker,10,92,2,-1
dot,19,hacker,-10,-10,-1,-1.125
)";
    const ui64 kIter = 50000;
};


TEST_F(FiltersTest, EqTest) {
    auto jf_out = std::make_shared<TStringStreamFileOutput>();
    {
        auto scheme_in = std::make_shared<TStringStreamFileInput>(scheme);
        auto data_in = std::make_shared<TStringStreamFileInput>();
        for (ui64 i = 0; i < kIter; i++) {
            data_in->Stream() << data;
        }

        auto [eng, err] = MakeEngineFromCsv(scheme_in.get(), data_in.get());

        if (err) {
            std::cout << "errdsd!" << std::endl;
            throw -1;
        }
        ASSERT_FALSE(err);

        auto err2 = eng.WriteTableToJf(jf_out.get());

        if (err2.HasError()) {
            std::cout << "err!" << std::endl;
            throw -1;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());

        TFilterQuery query{
            std::vector<TFilterOp>{
                TFilterOp{
                    /*column_name=*/"hot",
                    /*op=*/         EFilterType::kEq,
                    /*value=*/      "9"
                }
            }
        };
        auto jf_table_in = std::make_shared<TJfTableInput>(jf_in.get());
        auto agr = std::make_shared<TFilter>(jf_table_in, query);

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        TStringStreamFileOutput data;

        auto res = engine.WriteDataToCsv(&data);

        EXPECT_EQ(data.Str(), R"(john,9,frusciante,4,-10,-1,1.2
the,9,afterglow,9,40,81,1.8
dot,9,hacker,10,92,2,-1
)");
    }
}

} // namespace JfEngine::Testing

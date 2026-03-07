#include "filter.h"

#include <gtest/gtest.h>

#include "engine/engine.h"

#include <string_view>

namespace JFEngine::Testing {

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
};


TEST_F(FiltersTest, EqTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCSV(scheme_in, data_in);

        if (err) {
            std::cout << "errdsd!" << std::endl;
            throw -1;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJF(*jf_table);
        
        if (err2.HasError()) {
            std::cout << "err!" << std::endl;
            throw -1;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TFilterQuery query{
            std::vector<TFilterOp>{
                TFilterOp{
                    /*column_name=*/"hot",
                    /*op=*/         EFilterType::kEq,
                    /*value=*/      "9"
                }
            }
        };
        auto jf_in = std::make_shared<TJFTableInput>(jf_table);
        auto agr = std::make_shared<TFilter>(jf_in, query);

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCSV(data);

        // if (res.HasError()) {
        //     std::cout << "errrrr" << std::endl;
        // }

        // std::cout << "!! " << data.str() << std::endl;\
        
        EXPECT_EQ(data.str(), R"(john,9,frusciante,4,-10,-1,1.2
the,9,afterglow,9,40,81,1.8
dot,9,hacker,10,92,2,-1
)");
    }
}

} // namespace JFEngine::Testing

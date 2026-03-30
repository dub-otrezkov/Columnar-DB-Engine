#include "../groupby.h"
#include "engine/engine.h"

#include <gtest/gtest.h>

namespace JFEngine::Testing {

struct GroupByTest : testing::Test {
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
dot,19,hacker,10,92,2,-1
"i,could,have,lied",6,919,0,5,-5,82
the,14,sides,52,-4,11,18
the,9,afterglow,9,40,81,1.8
dot,19,hacker,10,82,82,0
dot,19,hacker,-10,-10,-1,-1.125
)";
};

TEST_F(GroupByTest, Basic) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCSV(scheme_in, data_in);

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJF(*jf_table);
        
        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_in = std::make_shared<TJFTableInput>(jf_table);

        TGroupByQuery gq{std::vector<std::shared_ptr<IAgregation>>{std::make_shared<TColumnAgr>("red")}};

        TGlobalAgregationQuery aq;
        auto group_name = std::make_shared<TColumnAgr>("red");
        auto column_name = std::make_shared<TColumnAgr>("what");
        auto cnt_agr = std::make_shared<TCountAgr>();
        auto sum_agr = std::make_shared<TSumAgr>();
        cnt_agr->AddArg(column_name);
        sum_agr->AddArg(column_name);
        aq.cols.push_back(group_name);
        aq.cols.push_back(cnt_agr);
        aq.cols.push_back(sum_agr);
        
        auto agr = std::make_shared<TGroupBy>(jf_in, gq, aq);

        agr->SetupColumnsScheme();

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCSV(data);

        EXPECT_EQ(data.str(), R"(dot,3,164
"i,could,have,lied",1,5
john,1,-10
josh,1,1
stadium,1,0
the,2,36
)");
    }
}

} // namespace JFEngine::Testing
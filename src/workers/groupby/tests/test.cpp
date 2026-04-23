#include "../groupby.h"
#include "engine/engine.h"

#include <gtest/gtest.h>

namespace JfEngine::Testing {

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

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng.WriteTableToJf(*jf_table);

        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);

        TGroupByQuery gq;
        {
            auto c = std::make_unique<TColumnOp>("red");
            c->is_final = true;
            gq.cols.push_back(std::move(c));
        }

        TAoQuery aq;
        aq.args.push_back(std::make_unique<TColumnOp>("what"));    // 0
        aq.args.push_back(std::make_unique<TColumnOp>("red"));     // 1
        aq.args.push_back(std::make_unique<TCountAgr>());          // 2
        aq.args.push_back(std::make_unique<TSumAgr>());            // 3
        aq.edges = {
            {2, 0},
            {3, 0}
        };
        aq.args[1]->is_final = true;
        aq.args[2]->is_final = true;
        aq.args[3]->is_final = true;

        auto agr = std::make_shared<TGroupBy>(jf_in, std::move(gq), std::move(aq));

        agr->SetupColumnsScheme();

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCsv(data);

        std::string a = data.str();
        std::cout << a << std::endl;
        std::sort(a.begin(), a.end());
        std::string b = R"(dot,3,164
"i,could,have,lied",1,5
john,1,-10
josh,1,1
stadium,1,0
the,2,36
)";
        std::sort(b.begin(), b.end());

        EXPECT_EQ(a, b);
    }
}

TEST_F(GroupByTest, Stress) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        constexpr ui64 iter = 500000;
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>();

        for (ui64 i = 0; i < iter; i++) {
            (*data_in) << data;
        }

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng.WriteTableToJf(*jf_table);

        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);

        TGroupByQuery gq;
        {
            auto c = std::make_unique<TColumnOp>("red");
            c->is_final = true;
            gq.cols.push_back(std::move(c));
        }

        TAoQuery aq;
        aq.args.push_back(std::make_unique<TColumnOp>("what"));    // 0
        aq.args.push_back(std::make_unique<TColumnOp>("red"));     // 1
        aq.args.push_back(std::make_unique<TCountAgr>());          // 2
        aq.args.push_back(std::make_unique<TSumAgr>());            // 3
        aq.edges = {
            {2, 0},
            {3, 0}
        };
        aq.args[1]->is_final = true;
        aq.args[2]->is_final = true;
        aq.args[3]->is_final = true;

        auto agr = std::make_shared<TGroupBy>(jf_in, std::move(gq), std::move(aq));

        agr->SetupColumnsScheme();

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCsv(data);

        ASSERT_FALSE(res.HasError());
    }
}

} // namespace JfEngine::Testing

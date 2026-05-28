#include "../groupby.h"
#include "engine/engine.h"
#include "utils/mmap_input/mmap_input.h"

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
    TStringStreamFileOutput jf_out;
    {
        auto scheme_in = std::make_shared<TStringStreamFileInput>(scheme);
        auto data_in   = std::make_shared<TStringStreamFileInput>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in.get(), data_in.get());

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng.WriteTableToJf(&jf_out);

        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_storage = std::make_shared<TStringStreamFileInput>(jf_out.Str());
        auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

        TGroupByQuery gq;
        gq.cols.push_back("red");

        TAoQuery aq;
        aq.args.push_back(std::make_unique<TColumnOp>("what"));
        aq.args.push_back(std::make_unique<TColumnOp>("red"));
        aq.args.push_back(std::make_unique<TCountAgr>());
        aq.args.push_back(std::make_unique<TSumAgr>());
        std::vector<std::pair<ui64, ui64>> edges = {
            {2, 0},
            {3, 0}
        };
        aq.args[1]->is_final = true;
        aq.args[2]->is_final = true;
        aq.args[3]->is_final = true;
        for (const auto& [i, j] : edges) {
            aq.args[i]->AddArg(aq.args[j].get());
        }

        auto agr = std::make_shared<TGroupBy>(jf_in, std::move(gq), std::move(aq));

        agr->SetupColumnsScheme();

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        TStringStreamFileOutput data;

        auto res = engine.WriteDataToCsv(&data);

        std::string a = data.Str();
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
    TStringStreamFileOutput jf_out;
    {
        constexpr ui64 iter = 500000;
        auto scheme_in = std::make_shared<TStringStreamFileInput>(scheme);
        auto data_in   = std::make_shared<TStringStreamFileInput>();

        for (ui64 i = 0; i < iter; i++) {
            data_in->Stream() << data;
        }

        auto [eng, err] = MakeEngineFromCsv(scheme_in.get(), data_in.get());

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng.WriteTableToJf(&jf_out);

        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_storage = std::make_shared<TStringStreamFileInput>(jf_out.Str());
        auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

        TGroupByQuery gq;
        gq.cols.push_back("red");

        TAoQuery aq;
        aq.args.push_back(std::make_unique<TColumnOp>("what"));
        aq.args.push_back(std::make_unique<TColumnOp>("red"));
        aq.args.push_back(std::make_unique<TCountAgr>());
        aq.args.push_back(std::make_unique<TSumAgr>());
        std::vector<std::pair<ui64, ui64>> edges = {
            {2, 0},
            {3, 0}
        };
        aq.args[1]->is_final = true;
        aq.args[2]->is_final = true;
        aq.args[3]->is_final = true;
        for (const auto& [i, j] : edges) {
            aq.args[i]->AddArg(aq.args[j].get());
        }

        auto agr = std::make_shared<TGroupBy>(jf_in, std::move(gq), std::move(aq));

        agr->SetupColumnsScheme();

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        TStringStreamFileOutput data;

        auto res = engine.WriteDataToCsv(&data);

        ASSERT_FALSE(res.HasError());
    }
}

} // namespace JfEngine::Testing

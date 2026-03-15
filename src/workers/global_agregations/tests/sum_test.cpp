#include "agregations_test.h"

#include "engine/engine.h"

namespace JFEngine::Testing {

TEST_F(AgregationsTest, SelectTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCSV(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJF(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TGlobalAgregationQuery query;
        auto column_name = std::make_shared<TColumnAgr>("what");
        auto sum_agr = std::make_shared<TSumAgr>();
        sum_agr->AddArg(column_name);
        auto jf_in = std::make_shared<TJFTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        auto agr = std::make_shared<TAgregator>(jf_in, TGlobalAgregationQuery{{column_name}});

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCSV(data);
    
        EXPECT_EQ(data.str(), R"(1
-10
0
5
-4
40
82
92
-10
)");
    }
}

TEST_F(AgregationsTest, SumTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCSV(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJF(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TGlobalAgregationQuery query;
        auto column_name = std::make_shared<TColumnAgr>("what");
        auto sum_agr = std::make_shared<TSumAgr>();
        sum_agr->AddArg(column_name);
        auto jf_in = std::make_shared<TJFTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        auto agr = std::make_shared<TAgregator>(jf_in, TGlobalAgregationQuery{{sum_agr}});

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCSV(data);
    
        EXPECT_EQ(data.str(), "196\n");
    }
}

TEST_F(AgregationsTest, CountTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCSV(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJF(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TGlobalAgregationQuery query;
        auto column_name = std::make_shared<TColumnAgr>("what");
        auto sum_agr = std::make_shared<TCountAgr>();
        sum_agr->AddArg(column_name);
        auto jf_in = std::make_shared<TJFTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        auto agr = std::make_shared<TAgregator>(jf_in, TGlobalAgregationQuery{{sum_agr}});

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCSV(data);
    
        EXPECT_EQ(data.str(), "9\n");
    }
}

} // namespace JFEngine::Testing

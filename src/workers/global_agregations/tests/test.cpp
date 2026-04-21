#include "agregations_test.h"
#include "engine/engine.h"

namespace JfEngine::Testing {

TEST_F(AgregationsTest, FullTableTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJf(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        auto agr = std::make_shared<TAgregator>(jf_in);

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data_;

        auto res = engine->WriteDataToCsv(data_);
    
        // std::cout << data.str() << std::endl;
        EXPECT_EQ(data_.str(), data);
    }
}

TEST_F(AgregationsTest, SelectTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJf(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TOperatorQuery query;
        auto column_name = std::make_shared<TColumnOp>("what");
        column_name->is_final = true;
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        TAoQuery q;
        q.args.push_back(std::move(column_name));
        auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCsv(data);
    
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

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJf(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TAgregationQuery query;
        auto column_name = std::make_shared<TColumnOp>("what");
        auto sum_agr = std::make_shared<TSumAgr>();
        sum_agr->is_final = true;
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        TAoQuery q;
        q.args.push_back(std::move(column_name));
        q.args.push_back(std::move(sum_agr));
        q.edges = {
            {1, 0}
        };
        auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCsv(data);
    
        EXPECT_EQ(data.str(), "196\n");
    }
}

TEST_F(AgregationsTest, CountTest) {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        ASSERT_FALSE(err);

        auto err2 = eng->WriteTableToJf(*jf_table);
        
        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        ASSERT_FALSE(err2.HasError());
    }

    {
        TAgregationQuery query;
        auto column_name = std::make_shared<TColumnOp>("what");
        auto sum_agr = std::make_shared<TCountAgr>();
        sum_agr->is_final = true;
        auto jf_in = std::make_shared<TJfTableInput>(jf_table);
        {
            auto err = jf_in->SetupColumnsScheme();
            if (err.HasError()) {
                std::cout << err.GetError() << std::endl;
            }
        }
        TAoQuery q;
        q.args.push_back(std::move(column_name));
        q.args.push_back(std::move(sum_agr));
        q.edges = {
            {1, 0}
        };
        q.tp = EAoEngineType::kAgregation;

        auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

        auto [engine, err] = MakeEngineFromWorker(agr);

        ASSERT_FALSE(err);

        std::stringstream data;

        auto res = engine->WriteDataToCsv(data);
    
        EXPECT_EQ(data.str(), "9\n");
    }
}

// TEST_F(AgregationsTest, Distinct) {
//     auto jf_table = std::make_shared<std::stringstream>();
//     {
//         auto scheme_in = std::make_shared<std::stringstream>(scheme);
//         auto data_in = std::make_shared<std::stringstream>(data);

//         auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

//         if (err) {
//             std::cout << err << std::endl;
//         }
//         ASSERT_FALSE(err);

//         auto err2 = eng->WriteTableToJf(*jf_table);
        
//         if (err2.HasError()) {
//             std::cout << err2.GetError() << std::endl;
//         }
//         ASSERT_FALSE(err2.HasError());
//     }

//     {
//         TAgregationQuery query;
//         auto column_name = std::make_shared<TColumnOp>("peppers");
//         auto sum_agr = std::make_shared<TDistinctOp>();
//         sum_agr->AddArg(column_name);
//         auto jf_in = std::make_shared<TJfTableInput>(jf_table);
//         {
//             auto err = jf_in->SetupColumnsScheme();
//             if (err.HasError()) {
//                 std::cout << err.GetError() << std::endl;
//             }
//         }
//         TAoQuery q;
//         q.args = std::vector<std::shared_ptr<IOa>>({sum_agr});
//         auto agr = std::make_shared<TAgregator>(jf_in, q);

//         auto [engine, err] = MakeEngineFromWorker(agr);

//         ASSERT_FALSE(err);

//         std::stringstream data;

//         auto res = engine->WriteDataToCsv(data);
    
//         EXPECT_EQ(data.str(), R"(2
// 4
// 6
// 0
// 52
// 9
// 10
// -10
// )");
//         // std::cout << data.str() << std::endl;
//     }
// }

// TEST_F(AgregationsTest, DistinctCount) {
//     auto jf_table = std::make_shared<std::stringstream>();
//     {
//         auto scheme_in = std::make_shared<std::stringstream>(scheme);
//         auto data_in = std::make_shared<std::stringstream>(data);

//         auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

//         if (err) {
//             std::cout << err << std::endl;
//         }
//         ASSERT_FALSE(err);

//         auto err2 = eng->WriteTableToJf(*jf_table);
        
//         if (err2.HasError()) {
//             std::cout << err2.GetError() << std::endl;
//         }
//         ASSERT_FALSE(err2.HasError());
//     }

//     {
//         TAgregationQuery query;
//         auto column_name = std::make_shared<TColumnOp>("peppers");
//         auto dst_agr = std::make_shared<TDistinctOp>();
//         auto cnt_agr = std::make_shared<TCountAgr>();
//         cnt_agr->AddArg(dst_agr);
//         dst_agr->AddArg(column_name);
//         auto jf_in = std::make_shared<TJfTableInput>(jf_table);
//         {
//             auto err = jf_in->SetupColumnsScheme();
//             if (err.HasError()) {
//                 std::cout << err.GetError() << std::endl;
//             }
//         }
//         TAoQuery q;
//         q.args = std::vector<std::shared_ptr<IOa>>({cnt_agr});
//         auto agr = std::make_shared<TAgregator>(jf_in, q);

//         auto [engine, err] = MakeEngineFromWorker(agr);

//         ASSERT_FALSE(err);

//         std::stringstream data;

//         auto res = engine->WriteDataToCsv(data);
    
//         EXPECT_EQ(data.str(), "8\n");
//         // std::cout << data.str() << std::endl;
//     }
// }

} // namespace JfEngine::Testing

#include "agregations_test.h"
#include "engine/engine.h"

namespace JfEngine::Testing {

std::shared_ptr<TJfTableInput> AgregationsTest::MakeJfIn() {
    auto jf_table = std::make_shared<std::stringstream>();
    {
        auto scheme_in = std::make_shared<std::stringstream>(scheme);
        auto data_in = std::make_shared<std::stringstream>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in, data_in);

        if (err) {
            std::cout << err << std::endl;
        }
        EXPECT_FALSE(err);

        auto err2 = eng.WriteTableToJf(*jf_table);

        if (err2.HasError()) {
            std::cout << err2.GetError() << std::endl;
        }
        EXPECT_FALSE(err2.HasError());
    }

    auto jf_in = std::make_shared<TJfTableInput>(jf_table);
    {
        auto err = jf_in->SetupColumnsScheme();
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
    }
    return jf_in;
}

TEST_F(AgregationsTest, FullTableTest) {
    auto jf_in = MakeJfIn();
    auto agr = std::make_shared<TAgregator>(jf_in);

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data_;

    auto res = engine.WriteDataToCsv(data_);

    EXPECT_EQ(data_.str(), data);
}

TEST_F(AgregationsTest, SelectTest) {
    auto jf_in = MakeJfIn();

    auto column_name = std::make_unique<TColumnOp>("what");
    column_name->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(column_name));
    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

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

TEST_F(AgregationsTest, SumTest) {
    auto jf_in = MakeJfIn();

    auto column_name = std::make_unique<TColumnOp>("what");
    auto sum_agr = std::make_unique<TSumAgr>();
    sum_agr->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(column_name));
    q.args.push_back(std::move(sum_agr));
    std::vector<std::pair<ui64, ui64>> edges = {
        {1, 0}
    };

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), "196\n");
}

TEST_F(AgregationsTest, CountTest) {
    auto jf_in = MakeJfIn();

    auto column_name = std::make_unique<TColumnOp>("what");
    auto sum_agr = std::make_unique<TCountAgr>();
    sum_agr->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(column_name));
    q.args.push_back(std::move(sum_agr));
    std::vector<std::pair<ui64, ui64>> edges = {
        {1, 0}
    };
    q.tp = EAoEngineType::kAgregation;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), "9\n");
}

TEST_F(AgregationsTest, MultipleAgrTest) {
    auto jf_in = MakeJfIn();

    auto column_name = std::make_unique<TColumnOp>("what");
    auto sum_agr = std::make_unique<TSumAgr>();
    sum_agr->is_final = true;
    auto count_agr = std::make_unique<TCountAgr>();
    count_agr->is_final = true;
    auto avg_agr = std::make_unique<TAvgAgr>();
    avg_agr->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(column_name));
    q.args.push_back(std::move(sum_agr));
    q.args.push_back(std::move(count_agr));
    q.args.push_back(std::move(avg_agr));
    std::vector<std::pair<ui64, ui64>> edges = {
        {1, 0},
        {2, 0},
        {3, 0}
    };
    q.tp = EAoEngineType::kAgregation;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), "196,9,21\n");
}

TEST_F(AgregationsTest, MultipleOpTest) {
    auto jf_in = MakeJfIn();

    auto column_name = std::make_unique<TColumnOp>("what");
    auto sum_agr = std::make_unique<TSumAgr>();
    sum_agr->is_final = true;
    auto count_agr = std::make_unique<TCountAgr>();
    count_agr->is_final = true;
    auto avg_agr = std::make_unique<TAvgAgr>();
    avg_agr->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(column_name));
    q.args.push_back(std::move(sum_agr));
    q.args.push_back(std::move(count_agr));
    q.args.push_back(std::move(avg_agr));
    std::vector<std::pair<ui64, ui64>> edges = {
        {1, 0},
        {2, 0},
        {3, 0}
    };
    q.tp = EAoEngineType::kAgregation;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), "196,9,21\n");
}

TEST_F(AgregationsTest, PlusConstTest) {
    auto jf_in = MakeJfIn();

    auto what_col = std::make_unique<TColumnOp>("what");
    what_col->is_final = true;
    auto what_arg = std::make_unique<TColumnOp>("what");
    auto const_1 = std::make_unique<TColumnOp>("1");
    auto plus = std::make_unique<TPlusOp>();
    plus->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(what_col));
    q.args.push_back(std::move(what_arg));
    q.args.push_back(std::move(const_1));
    q.args.push_back(std::move(plus));
    std::vector<std::pair<ui64, ui64>> edges = {
        {3, 1},
        {3, 2}
    };
    q.tp = EAoEngineType::kOperator;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), R"(1,2
-10,-9
0,1
5,6
-4,-3
40,41
82,83
92,93
-10,-9
)");
}

TEST_F(AgregationsTest, StringOpsTest) {
    auto jf_in = MakeJfIn();

    auto red_col = std::make_unique<TColumnOp>("red");
    red_col->is_final = true;
    auto red_for_len = std::make_unique<TColumnOp>("red");
    auto length = std::make_unique<TLengthOp>();
    length->is_final = true;
    auto red_for_regex = std::make_unique<TColumnOp>("red");
    auto pattern = std::make_unique<TColumnOp>("o");
    auto replacement = std::make_unique<TColumnOp>("O");
    auto regex = std::make_unique<TRegexpReplaceOp>();
    regex->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(red_col));
    q.args.push_back(std::move(red_for_len));
    q.args.push_back(std::move(length));
    q.args.push_back(std::move(red_for_regex));
    q.args.push_back(std::move(pattern));
    q.args.push_back(std::move(replacement));
    q.args.push_back(std::move(regex));
    std::vector<std::pair<ui64, ui64>> edges = {
        {2, 1},
        {6, 3},
        {6, 4},
        {6, 5}
    };
    q.tp = EAoEngineType::kOperator;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), R"(josh,4,jOsh
john,4,jOhn
stadium,7,stadium
"i,could,have,lied",17,"i,cOuld,have,lied"
cant,4,cant
the,3,the
dot,3,dOt
dot,3,dOt
dot,3,dOt
)");
}

TEST_F(AgregationsTest, ComplexAgrTest) {
    auto jf_in = MakeJfIn();

    auto red_for_len = std::make_unique<TColumnOp>("red");
    auto length = std::make_unique<TLengthOp>();
    auto avg_len = std::make_unique<TAvgAgr>();
    avg_len->is_final = true;

    auto what_arg = std::make_unique<TColumnOp>("what");
    auto const_1 = std::make_unique<TColumnOp>("1");
    auto plus = std::make_unique<TPlusOp>();
    auto sum_plus = std::make_unique<TSumAgr>();
    sum_plus->is_final = true;

    auto red_for_count_distinct = std::make_unique<TColumnOp>("red");
    auto count_distinct = std::make_unique<TCountDistinctAgr>();
    count_distinct->is_final = true;

    TAoQuery q;
    q.args.push_back(std::move(red_for_len));            // 0
    q.args.push_back(std::move(length));                 // 1
    q.args.push_back(std::move(avg_len));                // 2
    q.args.push_back(std::move(what_arg));               // 3
    q.args.push_back(std::move(const_1));                // 4
    q.args.push_back(std::move(plus));                   // 5
    q.args.push_back(std::move(sum_plus));               // 6
    q.args.push_back(std::move(red_for_count_distinct)); // 7
    q.args.push_back(std::move(count_distinct));         // 8
    std::vector<std::pair<ui64, ui64>> edges = {
        {1, 0},
        {2, 1},
        {5, 3},
        {5, 4},
        {6, 5},
        {8, 7}
    };
    q.tp = EAoEngineType::kAgregation;

    for (auto [u, v] : edges) {
        q.args.at(u)->AddArg(q.args.at(v).get());
    }

    auto agr = std::make_shared<TAgregator>(jf_in, std::move(q));

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    std::stringstream data;

    auto res = engine.WriteDataToCsv(data);

    EXPECT_EQ(data.str(), "5,205,7\n");
}

} // namespace JfEngine::Testing

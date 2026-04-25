#include "tests.h"

namespace JfEngine::Testing {

TEST_F(AgregationsTest, GetColumnsTest) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT hot, red, peppers FROM josh");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(hot,int64
red,string
peppers,int32
)");
    EXPECT_EQ(out_data->str(), R"(1,josh,2
3,john,4
5,stadium,6
6,"i,could,have,lied",0
14,cant,52
9,the,9
19,dot,10
19,dot,10
19,dot,-10
)");
}

TEST_F(AgregationsTest, GetColumnsSumTest) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT SUM(hot), SUM(red) FROM josh");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(SUM(hot),int128
SUM(red),string
)");
    EXPECT_EQ(out_data->str(), R"(95,"joshjohnstadiumi,could,have,liedcantthedotdotdot"
)");
}

TEST_F(AgregationsTest, GetCountTest) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh");
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(9
)");
}

TEST_F(AgregationsTest, GetAvgTest) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT AVG(what), AVG(once), AVG(was), AVG(hot) AS ddd FROM josh");
        if (err.HasError()) {
            // std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(AVG(what),int128
AVG(once),int128
AVG(was),int128
ddd,int128
)");
    EXPECT_EQ(out_data->str(), R"(21,19,11,10
)");
}

TEST_F(SumOverflowTest, Int64Overflow) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE ovf FROM scheme, data");
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT SUM(val) FROM ovf");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), "SUM(val),int128\n");
    // 2 * 4611686018427387904 = 9223372036854775808 > INT64_MAX (9223372036854775807)
    EXPECT_EQ(out_data->str(), "9223372036854775808\n");
}

}
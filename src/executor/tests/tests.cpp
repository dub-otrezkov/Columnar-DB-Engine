#include "tests.h"

namespace JFEngine::Testing {

TEST_F(AgregationsTest, GetColumnsTest) {
    JFEngine::TExecutor exec;
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
column 1,string
column 2,int32
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
    JFEngine::TExecutor exec;
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
column 1,string
)");
    EXPECT_EQ(out_data->str(), R"(95,"joshjohnstadiumi,could,have,liedcantthedotdotdot"
)");
}

TEST_F(AgregationsTest, GetCountTest) {
    JFEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
)");
    EXPECT_EQ(out_data->str(), R"(9
)");
}

TEST_F(AgregationsTest, GetAvgTest) {
    JFEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT AVG(what), AVG(once), AVG(was), AVG(hot) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(column 0,double
column 1,double
column 2,double
column 3,double
)");
    EXPECT_EQ(out_data->str(), R"(21.777778,19,11.075,10.555556
)");
}

}
#include "engine.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string_view>

using namespace JFEngine;

struct EngineTest : testing::Test {
    std::string scheme = R"(red,string
hot,int64
chili,string
peppers,int64
)";
    std::string data = R"(josh,1,klinghoffer,2
john,3,frusciante,4
stadium,5,arcadium,6
"i,could,have,lied",6,919,0
cant,14,stop,52
the,9,afterglow,9
dot,19,hacker,10
dot,19,hacker,10
dot,19,hacker,-10
)";

    std::stringstream scheme_ss;
    std::stringstream data_ss;

    void SetUp() override {
        scheme_ss << scheme;
        data_ss << data;
    }

    void TearDown() override {
        scheme_ss.clear();
        data_ss.clear();
    }
};

TEST_F(EngineTest, CSVToCSV) {

    auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

    ASSERT_FALSE(err);

    {
        std::stringstream out;
        err = eng->WriteSchemeToCSV(out).GetError();
        
        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), scheme);
    }
    {
        std::stringstream out;
        err = eng->WriteDataToCSV(out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), data);
    }
}

TEST_F(EngineTest, JFBasic) {
    std::stringstream out;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJF(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCSV(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCSV(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), data);
        }
    }
}

TEST_F(EngineTest, JFSmallRowGroupSize) {
    std::stringstream out;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss, /*row_group_size=*/1);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJF(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCSV(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCSV(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), data);
        }
    }
}

#include "engine.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string_view>

struct EngineTest : testing::Test {
    std::string scheme = R"(john,string
anthony,string
frusciante,string
)";
    std::string data = R"(please,dont,remember
me,for,what
i,did,last
night,please,dont
remember,me,its
only,19,80
its,only,1983
)";

    // std::stringstream data_jf("-\0\0\0\0\0\0\0john,string\nanthony,string\nfrusciante,string\ng\0\0\0\0\0\0\0please,me,i,night,remember,only,its\ndont,for,did,please,me,19,only\nremember,what,last,dont,its,80,1983\n");

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
        err = eng->WriteSchemeToCSV(out);
        
        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), scheme);
    }
    {
        std::stringstream out;
        err = eng->WriteDataToCSV(out);

        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), data);
    }
}

TEST_F(EngineTest, CSVToJF) {
    std::stringstream out;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(out);

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJF(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCSV(ans);
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCSV(ans);
            if (err) {
                std::cout << err->Print() << std::endl;
            }
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), data);
        }
    }
}
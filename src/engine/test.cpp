#include "engine.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string_view>

namespace JFEngine::Testing {

struct EngineTest : testing::Test {
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
"i,could,have,lied",6,919,0,5,-5,82
cant,14,stop,52,-4,11,18
the,9,afterglow,9,40,81,1.8
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
dot,19,hacker,-10,-10,-1,-1.125
)";

    std::shared_ptr<std::stringstream> scheme_ss = std::make_shared<std::stringstream>();
    std::shared_ptr<std::stringstream> data_ss = std::make_shared<std::stringstream>();

    void SetUp() override {
        *scheme_ss << scheme;
        *data_ss << data;
    }

    void TearDown() override {
        scheme_ss->clear();
        data_ss->clear();
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
    auto out = std::make_shared<std::stringstream>();
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(*out).GetError();

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
            if (err) {
                std::cout << err->Print() << std::endl;
            }
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), data);
        }
    }
}

TEST_F(EngineTest, JFSmallRowGroupSize) {
    auto out = std::make_shared<std::stringstream>();
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss, /*row_group_size=*/1);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(*out).GetError();

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


TEST_F(EngineTest, JFTimeColumns) {
    std::string time_scheme = R"(necessities,timestamp
)";
    auto time_scheme_ss = std::make_shared<std::stringstream>(time_scheme);
    std::string time_data = R"(2006-10-21 00:00:05
1234-03-05 14:09:38
1234-03-05 14:09:38
)";
    auto time_data_ss = std::make_shared<std::stringstream>(time_data);

    auto out = std::make_shared<std::stringstream>();

    {
        auto [eng, err] = MakeEngineFromCSV(time_scheme_ss, time_data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(*out).GetError();

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
            EXPECT_EQ(ans.str(), time_scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCSV(ans).GetError();
            if (err) {
                std::cout << err->Print() << std::endl;
            }
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), time_data);
        }
    }
}

} // namespace JFEngine::Testing

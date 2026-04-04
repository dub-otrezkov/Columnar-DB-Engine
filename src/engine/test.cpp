#include "engine.h"

#include <gtest/gtest.h>

#include <string_view>

namespace JfEngine::Testing {

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

    std::shared_ptr<std::stringstream> scheme_ss;
    std::shared_ptr<std::stringstream> data_ss;

    std::string ans_data = "";

    const ui64 kIter = 50000;

    void SetUp() override {
        scheme_ss = std::make_shared<std::stringstream>(scheme);
        data_ss = std::make_shared<std::stringstream>();
        ans_data = "";
        for (ui64 i = 0; i < kIter; i++) {
            (*data_ss) << data;
            ans_data += data;
        }
    }

    void TearDown() override {
    }
};

TEST_F(EngineTest, CsvToCsv) {

    auto [eng, err] = MakeEngineFromCsv(scheme_ss, data_ss);

    ASSERT_FALSE(err);

    {
        std::stringstream out;
        err = eng->WriteSchemeToCsv(out).GetError();
        
        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), scheme);
    }
    {
        std::stringstream out;
        err = eng->WriteDataToCsv(out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), ans_data);
    }
}

TEST_F(EngineTest, EmptyData) {
    auto empty_data = std::make_shared<std::stringstream>("\n");
    auto [eng, err] = MakeEngineFromCsv(scheme_ss, empty_data);

    ASSERT_FALSE(err);

    {
        std::stringstream out;
        err = eng->WriteSchemeToCsv(out).GetError();
        
        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), scheme);
    }
    {
        std::stringstream out;
        err = eng->WriteDataToCsv(out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.str(), "");
    }
}

TEST_F(EngineTest, JfBasic) {
    auto out = std::make_shared<std::stringstream>();
    {
        auto [eng, err] = MakeEngineFromCsv(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJf(*out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJf(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), ans_data);
        }
    }
}

TEST_F(EngineTest, JfEmpty) {
    auto out = std::make_shared<std::stringstream>();
    {
        auto empty_data = std::make_shared<std::stringstream>("\n");
        auto [eng, err] = MakeEngineFromCsv(scheme_ss, empty_data);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJf(*out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJf(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCsv(ans).GetError();
            ASSERT_FALSE(err);
            // EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCsv(ans).GetError();
            if (err) {
            }
            ASSERT_FALSE(err);
            // EXPECT_EQ(ans.str(), "");
        }
    }
}

TEST_F(EngineTest, JfSmallRowGroupSize) {
    auto out = std::make_shared<std::stringstream>();
    {
        auto [eng, err] = MakeEngineFromCsv(scheme_ss, data_ss, /*row_group_size=*/1);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJf(*out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJf(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), ans_data);
        }
    }
}

TEST_F(EngineTest, JfDateColumns) {
    std::string time_scheme = R"(necessities,date
)";
    auto time_scheme_ss = std::make_shared<std::stringstream>(time_scheme);
    std::string time_data = R"(2006-08-21
2022-02-24
1234-03-05
)";
    auto time_data_ss = std::make_shared<std::stringstream>(time_data);

    auto out = std::make_shared<std::stringstream>();

    {
        auto [eng, err] = MakeEngineFromCsv(time_scheme_ss, time_data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJf(*out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJf(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), time_scheme);
        }

        {
            std::stringstream ans;
            auto err2 = eng->WriteDataToCsv(ans).GetError();
            if (err) {
                std::cout << err << std::endl;
            }
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), time_data);
        }
    }
}

TEST_F(EngineTest, JfTimeColumns) {
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
        auto [eng, err] = MakeEngineFromCsv(time_scheme_ss, time_data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJf(*out).GetError();

            ASSERT_FALSE(err);
        }
    }
    {
        auto [eng, err] = MakeEngineFromJf(out);

        ASSERT_FALSE(err);
        {
            std::stringstream ans;
            err = eng->WriteSchemeToCsv(ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), time_scheme);
        }

        {
            std::stringstream ans;
            err = eng->WriteDataToCsv(ans).GetError();
            if (err) {
                std::cout << err << std::endl;
            }
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.str(), time_data);
        }
    }
}

} // namespace JfEngine::Testing

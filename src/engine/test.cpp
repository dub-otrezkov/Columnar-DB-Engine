#include "engine.h"

#include "utils/mmap_input/mmap_input.h"

#include <gtest/gtest.h>

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

    std::shared_ptr<TStringStreamFileInput> scheme_ss;
    std::shared_ptr<TStringStreamFileInput> data_ss;

    std::string ans_data = "";

    const ui64 kIter = 1;

    void SetUp() override {
        scheme_ss = std::make_shared<TStringStreamFileInput>(scheme);
        data_ss = std::make_shared<TStringStreamFileInput>();
        ans_data = "";
        for (ui64 i = 0; i < kIter; i++) {
            data_ss->Stream() << data;
            ans_data += data;
        }
    }

    void TearDown() override {
    }
};

TEST_F(EngineTest, CsvToCsv) {

    auto [eng, err] = MakeEngineFromCsv(scheme_ss.get(), data_ss.get());

    ASSERT_FALSE(err);

    {
        TStringStreamFileOutput out;
        err = eng.WriteSchemeToCsv(&out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.Str(), scheme);
    }
    {
        TStringStreamFileOutput out;
        err = eng.WriteDataToCsv(&out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.Str(), ans_data);
    }
}

TEST_F(EngineTest, EmptyData) {
    auto empty_data = std::make_shared<TStringStreamFileInput>("\n");
    auto [eng, err] = MakeEngineFromCsv(scheme_ss.get(), empty_data.get());

    ASSERT_FALSE(err);

    {
        TStringStreamFileOutput out;
        err = eng.WriteSchemeToCsv(&out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.Str(), scheme);
    }
    {
        TStringStreamFileOutput out;
        err = eng.WriteDataToCsv(&out).GetError();

        ASSERT_FALSE(err);
        EXPECT_EQ(out.Str(), "");
    }
}

TEST_F(EngineTest, JfBasic) {
    auto jf_out = std::make_shared<TStringStreamFileOutput>();
    {
        auto [eng, err] = MakeEngineFromCsv(scheme_ss.get(), data_ss.get());

        ASSERT_FALSE(err);

        {
            err = eng.WriteTableToJf(jf_out.get()).GetError();

            ASSERT_FALSE(err);
        }
    }
    auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());
    {
        auto [eng, err] = MakeEngineFromJf(jf_in.get());

        ASSERT_FALSE(err);
        {
            TStringStreamFileOutput ans;
            err = eng.WriteSchemeToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), scheme);
        }

        {
            TStringStreamFileOutput ans;
            err = eng.WriteDataToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), ans_data);
        }
    }
}

TEST_F(EngineTest, JfEmpty) {
    auto jf_out = std::make_shared<TStringStreamFileOutput>();
    {
        auto empty_data = std::make_shared<TStringStreamFileInput>("\n");
        auto [eng, err] = MakeEngineFromCsv(scheme_ss.get(), empty_data.get());

        ASSERT_FALSE(err);

        {
            err = eng.WriteTableToJf(jf_out.get()).GetError();

            ASSERT_FALSE(err);
        }
    }
    auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());
    {
        auto [eng, err] = MakeEngineFromJf(jf_in.get());

        ASSERT_FALSE(err);
        {
            TStringStreamFileOutput ans;
            err = eng.WriteSchemeToCsv(&ans).GetError();
            ASSERT_FALSE(err);
        }

        {
            TStringStreamFileOutput ans;
            err = eng.WriteDataToCsv(&ans).GetError();
            ASSERT_FALSE(err);
        }
    }
}

TEST_F(EngineTest, JfSmallRowGroupSize) {
    auto jf_out = std::make_shared<TStringStreamFileOutput>();
    {
        auto [eng, err] = MakeEngineFromCsv(scheme_ss.get(), data_ss.get(), 1);

        ASSERT_FALSE(err);

        {
            err = eng.WriteTableToJf(jf_out.get()).GetError();

            ASSERT_FALSE(err);
        }
    }
    auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());
    {
        auto [eng, err] = MakeEngineFromJf(jf_in.get());

        ASSERT_FALSE(err);
        {
            TStringStreamFileOutput ans;
            err = eng.WriteSchemeToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), scheme);
        }

        {
            TStringStreamFileOutput ans;
            err = eng.WriteDataToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), ans_data);
        }
    }
}

TEST_F(EngineTest, JfDateColumns) {
    std::string time_scheme = R"(necessities,date
)";
    auto time_scheme_ss = std::make_shared<TStringStreamFileInput>(time_scheme);
    std::string time_data = R"(2006-08-21
2022-02-24
1234-03-05
)";
    auto time_data_ss = std::make_shared<TStringStreamFileInput>(time_data);

    auto jf_out = std::make_shared<TStringStreamFileOutput>();

    {
        auto [eng, err] = MakeEngineFromCsv(time_scheme_ss.get(), time_data_ss.get());

        ASSERT_FALSE(err);

        {
            err = eng.WriteTableToJf(jf_out.get()).GetError();

            ASSERT_FALSE(err);
        }
    }
    auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());
    {
        auto [eng, err] = MakeEngineFromJf(jf_in.get());

        ASSERT_FALSE(err);
        {
            TStringStreamFileOutput ans;
            err = eng.WriteSchemeToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), time_scheme);
        }

        {
            TStringStreamFileOutput ans;
            auto err2 = eng.WriteDataToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), time_data);
        }
    }
}

TEST_F(EngineTest, JfTimeColumns) {
    std::string time_scheme = R"(necessities,timestamp
)";
    auto time_scheme_ss = std::make_shared<TStringStreamFileInput>(time_scheme);
    std::string time_data = R"(2006-10-21 00:00:05
1234-03-05 14:09:38
1234-03-05 14:09:38
)";
    auto time_data_ss = std::make_shared<TStringStreamFileInput>(time_data);

    auto jf_out = std::make_shared<TStringStreamFileOutput>();

    {
        auto [eng, err] = MakeEngineFromCsv(time_scheme_ss.get(), time_data_ss.get());

        ASSERT_FALSE(err);

        {
            err = eng.WriteTableToJf(jf_out.get()).GetError();

            ASSERT_FALSE(err);
        }
    }
    auto jf_in = std::make_shared<TStringStreamFileInput>(jf_out->Str());
    {
        auto [eng, err] = MakeEngineFromJf(jf_in.get());

        ASSERT_FALSE(err);
        {
            TStringStreamFileOutput ans;
            err = eng.WriteSchemeToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), time_scheme);
        }

        {
            TStringStreamFileOutput ans;
            err = eng.WriteDataToCsv(&ans).GetError();
            ASSERT_FALSE(err);
            EXPECT_EQ(ans.Str(), time_data);
        }
    }
}

}

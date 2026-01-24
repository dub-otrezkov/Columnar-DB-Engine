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

TEST_F(EngineTest, SelectColumnsTest) {
    std::stringstream jf_file;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss, 2);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(jf_file).GetError();

            ASSERT_FALSE(err);
        }
    }
    std::vector<std::string> cols{"red", "hot"};
    auto [eng, err] = MakeSelectEngine(jf_file, {cols});

    {
        std::stringstream data;

        auto res = eng->WriteSchemeToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(red,string
hot,int64
)");
    }
    {
        std::stringstream data;

        auto res = eng->WriteDataToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(josh,1
john,3
stadium,5
"i,could,have,lied",6
cant,14
the,9
dot,19
dot,19
dot,19
)");
    }
}

TEST_F(EngineTest, SelectColumnsOrderingTest) {
    std::stringstream jf_file;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(jf_file).GetError();

            ASSERT_FALSE(err);
        }
    }
    std::vector<std::string> cols{"hot", "red", "peppers"};
    auto [eng, err] = MakeSelectEngine(jf_file, {cols});
    
    {
        std::stringstream data;

        auto res = eng->WriteSchemeToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(hot,int64
red,string
peppers,int64
)");
    }
    {
        std::stringstream data;

        auto res = eng->WriteDataToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(1,josh,2
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
}

TEST_F(EngineTest, SelectColumnsWithAliasesTest) {
    std::stringstream jf_file;
    {
        auto [eng, err] = MakeEngineFromCSV(scheme_ss, data_ss);

        ASSERT_FALSE(err);

        {
            err = eng->WriteTableToJF(jf_file).GetError();

            ASSERT_FALSE(err);
        }
    }
    std::vector<std::string> cols{"red", "hot"};
    std::unordered_map<std::string, std::string> aliases{{"red", "john"}};
    auto [eng, err] = MakeSelectEngine(jf_file, TSelectQuery{cols, aliases});

    {
        std::stringstream data;

        auto res = eng->WriteSchemeToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(john,string
hot,int64
)");
    }
    {
        std::stringstream data;

        auto res = eng->WriteDataToCSV(data);
        
        ASSERT_FALSE(res.HasError());

        ASSERT_EQ(data.str(), R"(josh,1
john,3
stadium,5
"i,could,have,lied",6
cant,14
the,9
dot,19
dot,19
dot,19
)");
    }
}

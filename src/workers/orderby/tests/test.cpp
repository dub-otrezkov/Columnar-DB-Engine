#include "../orderby.h"
#include "engine/engine.h"
#include "utils/mmap_input/mmap_input.h"

#include <gtest/gtest.h>

namespace JfEngine::Testing {

struct OrderByTest : testing::Test {
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
dot,19,hacker,10,92,2,-1
"i,could,have,lied",6,919,0,5,-5,82
the,14,sides,52,-4,11,18
the,9,afterglow,9,40,81,1.8
dot,19,hacker,10,82,82,0
dot,19,hacker,-10,-10,-1,-1.125
)";
};

static std::shared_ptr<TStringStreamFileInput> BuildJf(const std::string& scheme, const std::string& data) {
    TStringStreamFileOutput jf_out;
    {
        auto scheme_in = std::make_shared<TStringStreamFileInput>(scheme);
        auto data_in   = std::make_shared<TStringStreamFileInput>(data);

        auto [eng, err] = MakeEngineFromCsv(scheme_in.get(), data_in.get());

        if (err) {
            std::cout << "! " << err << std::endl;
        }
        EXPECT_FALSE(err);

        auto err2 = eng.WriteTableToJf(&jf_out);

        if (err2.HasError()) {
            std::cout << "! " << err2.GetError() << std::endl;
        }
        EXPECT_FALSE(err2.HasError());
    }
    return std::make_shared<TStringStreamFileInput>(jf_out.Str());
}

TEST_F(OrderByTest, Basic) {
    auto jf_storage = BuildJf(scheme, data);
    auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

    TOrderByQuery oq{std::vector<std::string>{"hot", "peppers"}};

    auto agr = std::make_shared<TOrderBy>(jf_in, oq);

    agr->SetupColumnsScheme();

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    TStringStreamFileOutput data_;

    auto res = engine.WriteDataToCsv(&data_);

    EXPECT_EQ(data_.Str(), R"(josh,1,klinghoffer,2,1,2,0
john,3,frusciante,4,-10,-1,1.2
stadium,5,arcadium,6,0,0,-1.2
"i,could,have,lied",6,919,0,5,-5,82
the,9,afterglow,9,40,81,1.8
the,14,sides,52,-4,11,18
dot,19,hacker,-10,-10,-1,-1.125
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
)");
}

TEST_F(OrderByTest, Offset) {
    auto jf_storage = BuildJf(scheme, data);
    auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

    TOrderByQuery oq{std::vector<std::string>{"hot", "peppers"}};
    oq.offset = 2;

    auto agr = std::make_shared<TOrderBy>(jf_in, oq);

    agr->SetupColumnsScheme();

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    TStringStreamFileOutput data_;

    auto res = engine.WriteDataToCsv(&data_);

    EXPECT_EQ(data_.Str(), R"(stadium,5,arcadium,6,0,0,-1.2
"i,could,have,lied",6,919,0,5,-5,82
the,9,afterglow,9,40,81,1.8
the,14,sides,52,-4,11,18
dot,19,hacker,-10,-10,-1,-1.125
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
)");
}

TEST_F(OrderByTest, Reverse) {
    auto jf_storage = BuildJf(scheme, data);
    auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

    TOrderByQuery oq{std::vector<std::string>{"hot"}, true};

    auto agr = std::make_shared<TOrderBy>(jf_in, oq);

    agr->SetupColumnsScheme();

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    TStringStreamFileOutput data_;

    auto res = engine.WriteDataToCsv(&data_);

    EXPECT_EQ(data_.Str(), R"(dot,19,hacker,-10,-10,-1,-1.125
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
the,14,sides,52,-4,11,18
the,9,afterglow,9,40,81,1.8
"i,could,have,lied",6,919,0,5,-5,82
stadium,5,arcadium,6,0,0,-1.2
john,3,frusciante,4,-10,-1,1.2
josh,1,klinghoffer,2,1,2,0
)");
}

TEST_F(OrderByTest, Limit) {
    auto jf_storage = BuildJf(scheme, data);
    auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

    TOrderByQuery oq{std::vector<std::string>{"hot", "peppers"}, false, 3};

    auto agr = std::make_shared<TOrderBy>(jf_in, oq);

    agr->SetupColumnsScheme();

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    TStringStreamFileOutput data_;

    auto res = engine.WriteDataToCsv(&data_);

    EXPECT_EQ(data_.Str(), R"(josh,1,klinghoffer,2,1,2,0
john,3,frusciante,4,-10,-1,1.2
stadium,5,arcadium,6,0,0,-1.2
)");
}

TEST_F(OrderByTest, ReverseLimit) {
    auto jf_storage = BuildJf(scheme, data);
    auto jf_in = std::make_shared<TJfTableInput>(jf_storage.get());

    TOrderByQuery oq{std::vector<std::string>{"hot"}, true, 4};

    auto agr = std::make_shared<TOrderBy>(jf_in, oq);

    agr->SetupColumnsScheme();

    auto [engine, err] = MakeEngineFromWorker(agr);

    ASSERT_FALSE(err);

    TStringStreamFileOutput data_;

    auto res = engine.WriteDataToCsv(&data_);

    EXPECT_EQ(data_.Str(), R"(dot,19,hacker,-10,-10,-1,-1.125
dot,19,hacker,10,82,82,0
dot,19,hacker,10,92,2,-1
the,14,sides,52,-4,11,18
)");
}

} // namespace JfEngine::Testing

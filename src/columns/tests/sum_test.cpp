#include "../operators/operators.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string_view>

namespace JFEngine::Testing {

struct OperatorsTest : testing::Test {};

TEST_F(OperatorsTest, i64SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::Ei64Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 1);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::Ei64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i32SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::Ei32Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 1);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::Ei64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i16SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::Ei16Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 1);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::Ei64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i8SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::Ei32Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 1);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::Ei64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, StringSumTest) {
    {
        std::vector<std::string> data = {"john", "*", "frusciante", "10"};
        auto m = MakeColumn(data, EColumn::EStringColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 1);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::EStringColumn);
        auto res = std::dynamic_pointer_cast<TStringColumn>(r.GetShared());

        EXPECT_EQ(res->GetData()[0], "john*frusciante10");
    }
}

TEST_F(OperatorsTest, DateSumTest) {
    {
        std::vector<std::string> data = {"2022-02-24", "2025-14-02"};
        auto m = MakeColumn(data, EColumn::EDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_TRUE(r.HasError());
    }
}

TEST_F(OperatorsTest, TimestampSumTest) {
    {
        std::vector<std::string> data = {"2022-02-24 05:19:10", "2025-14-02 11:09:01"};
        auto m = MakeColumn(data, EColumn::ETimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetShared());

        ASSERT_TRUE(r.HasError());
    }
}

} // namespace JFEngine::Testing

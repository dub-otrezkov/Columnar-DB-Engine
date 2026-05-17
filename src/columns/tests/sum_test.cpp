#include "tests.h"

namespace JfEngine::Testing {

TEST_F(OperatorsTest, i64SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::ki64Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetRes()->GetSize() == 1);
        ASSERT_TRUE(r.GetRes()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetRes());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i32SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::ki32Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetRes()->GetSize() == 1);
        ASSERT_TRUE(r.GetRes()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetRes());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i16SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::ki16Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetRes()->GetSize() == 1);
        ASSERT_TRUE(r.GetRes()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetRes());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, i8SumTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9"};
        auto m = MakeColumn(data, EColumn::ki32Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetRes()->GetSize() == 1);
        ASSERT_TRUE(r.GetRes()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetRes());

        EXPECT_EQ(res->GetData()[0], 27);
    }
}

TEST_F(OperatorsTest, StringSumTest) {
    {
        std::vector<std::string> data = {"john", "*", "frusciante", "10"};
        auto m = MakeColumn(data, EColumn::kStringColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetRes()->GetSize() == 1);
        ASSERT_TRUE(r.GetRes()->GetType() == EColumn::kStringColumn);
        auto res = std::dynamic_pointer_cast<TStringColumn>(r.GetRes());

        EXPECT_EQ(res->GetData()[0], "john*frusciante10");
    }
}

TEST_F(OperatorsTest, DateSumTest) {
    {
        std::vector<std::string> data = {"2022-02-24", "2025-14-02"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_TRUE(r.HasError());
    }
}

TEST_F(OperatorsTest, TimestampSumTest) {
    {
        std::vector<std::string> data = {"2022-02-24 05:19:10", "2025-14-02 11:09:01"};
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OSum>(m.GetRes());

        ASSERT_TRUE(r.HasError());
    }
}

} // namespace JfEngine::Testing

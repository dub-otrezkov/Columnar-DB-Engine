#include "tests.h"

namespace JFEngine::Testing {

TEST_F(OperatorsTest, i64VerticalSumTest) {
    {
        std::vector<std::string> data1 = {"-1", "10", "9", "-9"};
        std::vector<std::string> data2 = {"10", "5", "-3", "-2"};
        auto a = MakeColumn(data1, EColumn::ki64Column);
        auto b = MakeColumn(data2, EColumn::ki64Column);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 4);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());
        std::vector<i64> ans{9, 15, 6, -11};
        for (ui64 i = 0; i < 4; i++) {
            EXPECT_EQ(res->GetData()[i], ans[i]);
        }
    }
}


TEST_F(OperatorsTest, i32VerticalSumTest) {
    {
        std::vector<std::string> data1 = {"-1", "10", "9", "-9"};
        std::vector<std::string> data2 = {"10", "5", "-3", "-2"};
        auto a = MakeColumn(data1, EColumn::ki32Column);
        auto b = MakeColumn(data2, EColumn::ki32Column);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 4);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());
        std::vector<i64> ans{9, 15, 6, -11};
        for (ui64 i = 0; i < 4; i++) {
            EXPECT_EQ(res->GetData()[i], ans[i]);
        }
    }
}

TEST_F(OperatorsTest, i16VerticalSumTest) {
    {
        std::vector<std::string> data1 = {"-1", "10", "9", "-9"};
        std::vector<std::string> data2 = {"10", "5", "-3", "-2"};
        auto a = MakeColumn(data1, EColumn::ki16Column);
        auto b = MakeColumn(data2, EColumn::ki16Column);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 4);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());
        std::vector<i64> ans{9, 15, 6, -11};
        for (ui64 i = 0; i < 4; i++) {
            EXPECT_EQ(res->GetData()[i], ans[i]);
        }
    }
}

TEST_F(OperatorsTest, i8VerticalSumTest) {
    {
        std::vector<std::string> data1 = {"-1", "10", "9", "-9"};
        std::vector<std::string> data2 = {"10", "5", "-3", "-2"};
        auto a = MakeColumn(data1, EColumn::ki8Column);
        auto b = MakeColumn(data2, EColumn::ki8Column);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 4);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::ki64Column);
        auto res = std::dynamic_pointer_cast<Ti64Column>(r.GetShared());
        std::vector<i64> ans{9, 15, 6, -11};
        for (ui64 i = 0; i < 4; i++) {
            EXPECT_EQ(res->GetData()[i], ans[i]);
        }
    }
}

TEST_F(OperatorsTest, i64DoubleSumTest) {
    {
        std::vector<std::string> data1 = {"-1.5", "10", "9.2", "-9.13"};
        std::vector<std::string> data2 = {"10", "5.2", "-3.1", "-2.91"};
        auto a = MakeColumn(data1, EColumn::kDoubleColumn);
        auto b = MakeColumn(data2, EColumn::kDoubleColumn);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 4);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::kDoubleColumn);
        auto res = std::dynamic_pointer_cast<TDoubleColumn>(r.GetShared());
        std::vector<ld> ans{8.5, 15.2, 6.1, -12.04};
        for (ui64 i = 0; i < 4; i++) {
            EXPECT_FLOAT_EQ(res->GetData()[i], ans[i]);
        }
    }
}

TEST_F(OperatorsTest, StringVerticalSumTest) {
    {
        std::vector<std::string> data1 = {"john", "josh"};
        std::vector<std::string> data2 = {"frusciante", "klinghoffer"};
        auto a = MakeColumn(data1, EColumn::kStringColumn);
        auto b = MakeColumn(data2, EColumn::kStringColumn);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        if (r.HasError()) {
            // std::cout << r.GetError()->Print() << std::endl;
        }
        ASSERT_FALSE(r.HasError());

        ASSERT_TRUE(r.GetShared()->GetSize() == 2);
        ASSERT_TRUE(r.GetShared()->GetType() == EColumn::kStringColumn);
        auto res = std::dynamic_pointer_cast<TStringColumn>(r.GetShared());
        std::vector<std::string> ans{"johnfrusciante", "joshklinghoffer"};
        for (ui64 i = 0; i < ans.size(); i++) {
            EXPECT_EQ(res->GetData()[i], ans[i]);
        }
    }
}

TEST_F(OperatorsTest, DateVerticalSumTest) {
    {
        std::vector<std::string> data1 = {"2022-02-24", "2025-14-02"};
        std::vector<std::string> data2 = {"2022-02-24", "2025-14-02"};
        auto a = MakeColumn(data1, EColumn::kDateColumn);
        auto b = MakeColumn(data2, EColumn::kDateColumn);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        ASSERT_TRUE(r.HasError());
    }
}

TEST_F(OperatorsTest, TimeStampVerticalSumTest) {
    {
        std::vector<std::string> data1 = {"2022-02-24 05:19:10", "2022-02-24 05:19:10"};
        std::vector<std::string> data2 = {"2022-02-24 05:19:10", "2022-02-24 05:19:10"};
        auto a = MakeColumn(data1, EColumn::kTimestampColumn);
        auto b = MakeColumn(data2, EColumn::kTimestampColumn);

        ASSERT_FALSE(a.HasError());
        ASSERT_FALSE(b.HasError());

        auto r = Do<OVerticalSum>(a.GetShared(), b.GetShared());

        ASSERT_TRUE(r.HasError());
    }
}

} // namespace JFEngine::Testing

#include "tests.h"

#include "../operators/filter.h"

namespace JfEngine::Testing {

TEST_F(OperatorsTest, FilterEqTest) {
    auto test = [](EColumn t) -> void {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, t);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kEq, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 2);
    };

    test(EColumn::ki8Column);
    test(EColumn::ki16Column);
    test(EColumn::ki32Column);
    test(EColumn::ki64Column);

    {
        std::vector<std::string> data = {"2022-02-24", "2019-10-09", "2025-01-31", "1970-01-01", "2000-12-31"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kEq, "2022-02-24");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 1);
    }

    {
        std::vector<std::string> data = {
            "2022-02-24 04:04:04",
            "2019-10-09 05:50:59",
            "2025-01-31 10:00:00",
            "1970-01-01 14:47:08",
            "2022-02-24 04:04:06"
        };
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kEq, "2022-02-24 04:04:04");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 1);
    }
}

TEST_F(OperatorsTest, FilterLessTest) {
    auto test = [](EColumn t) -> void {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, t);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLess, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 2);
    };

    test(EColumn::ki8Column);
    test(EColumn::ki16Column);
    test(EColumn::ki32Column);
    test(EColumn::ki64Column);

        {
        std::vector<std::string> data = {"2022-02-24", "2019-10-09", "2025-01-31", "1970-01-01", "2000-12-31"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLess, "2022-02-24");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 3);
    }

    {
        std::vector<std::string> data = {
            "2022-02-24 04:04:04",
            "2019-10-09 05:50:59",
            "2025-01-31 10:00:00",
            "1970-01-01 14:47:08",
            "2022-02-24 04:04:06"
        };
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLess, "2022-02-24 04:04:04");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 2);
    }
}

TEST_F(OperatorsTest, FilterLeqTest) {
    auto test = [](EColumn t) -> void {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, t);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLeq, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 4);
    };

    test(EColumn::ki8Column);
    test(EColumn::ki16Column);
    test(EColumn::ki32Column);
    test(EColumn::ki64Column);

    {
        std::vector<std::string> data = {"2022-02-24", "2019-10-09", "2025-01-31", "1970-01-01", "2000-12-31"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLeq, "2022-02-24");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 4);
    }

    {
        std::vector<std::string> data = {
            "2022-02-24 04:04:04",
            "2019-10-09 05:50:59",
            "2025-01-31 10:00:00",
            "1970-01-01 14:47:08",
            "2022-02-24 04:04:06"
        };
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLeq, "2022-02-24 04:04:04");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 3);
    }
}

TEST_F(OperatorsTest, FilterGreaterTest) {
    auto test = [](EColumn t) -> void {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, t);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGreater, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 0);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 1);
    };

    test(EColumn::ki8Column);
    test(EColumn::ki16Column);
    test(EColumn::ki32Column);
    test(EColumn::ki64Column);

    {
        std::vector<std::string> data = {"2022-02-24", "2019-10-09", "2025-01-31", "1970-01-01", "2000-12-31"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGreater, "2022-02-24");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 1);
    }

    {
        std::vector<std::string> data = {
            "2022-02-24 04:04:04",
            "2019-10-09 05:50:59",
            "2025-01-31 10:00:00",
            "1970-01-01 14:47:08",
            "2022-02-24 04:04:06"
        };
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGreater, "2022-02-24 04:04:04");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 2);
    }
}

TEST_F(OperatorsTest, FilterGeqTest) {
    auto test = [](EColumn t) -> void {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, t);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGeq, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 0);
        EXPECT_EQ(mask.at(1), 1);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 1);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 3);
    };

    test(EColumn::ki8Column);
    test(EColumn::ki16Column);
    test(EColumn::ki32Column);
    test(EColumn::ki64Column);

    {
        std::vector<std::string> data = {"2022-02-24", "2019-10-09", "2025-01-31", "1970-01-01", "2000-12-31"};
        auto m = MakeColumn(data, EColumn::kDateColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGeq, "2022-02-24");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 0);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 2);
    }

    {
        std::vector<std::string> data = {
            "2022-02-24 04:04:04",
            "2019-10-09 05:50:59",
            "2025-01-31 10:00:00",
            "1970-01-01 14:47:08",
            "2022-02-24 04:04:06"
        };
        auto m = MakeColumn(data, EColumn::kTimestampColumn);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kGeq, "2022-02-24 04:04:04");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetRes();

        ASSERT_TRUE(mask.size() == 5);

        EXPECT_EQ(mask.at(0), 1);
        EXPECT_EQ(mask.at(1), 0);
        EXPECT_EQ(mask.at(2), 1);
        EXPECT_EQ(mask.at(3), 0);
        EXPECT_EQ(mask.at(4), 1);

        auto e = Do<OFilter>(m.GetRes(), mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetRes();
        ASSERT_EQ(ans->GetSize(), 3);
    }
}

void CheckVectorsEq(
    const StringVector& a,
    const std::vector<std::string>& b
) {
    EXPECT_EQ(a.size(), b.size());
    for (ui64 i = 0; i < a.size(); i++) {
        EXPECT_EQ(a[i], b[i]);
    }
}

TEST_F(OperatorsTest, FilterLikeTest) {
    {
        std::vector<std::string> data = {
            "john",
            "johnfrusciante",
            "josh",
            "9",
            "8",
            "joshklinghoffer",
            "rhcp rhcp rhcp",
            "the getaway",
            "detroit",
            "dark neccessities",
            "encore",
            "feasting on the flowers"
        };
        auto m = MakeColumn(data, EColumn::kStringColumn);
        ASSERT_FALSE(m.HasError());

        {
            auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLike, "9");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetRes();

            ASSERT_TRUE(mask.size() == data.size());
            
            auto e = Do<OFilter>(m.GetRes(), mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetRes();
            ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
            CheckVectorsEq(static_cast<TStringColumn*>(ans.get())->GetData(), std::vector<std::string>{
                "9"
            });
        }

        
        {
            auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kLike, "%h%");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetRes();

            ASSERT_TRUE(mask.size() == data.size());
            
            auto e = Do<OFilter>(m.GetRes(), mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetRes();
            ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
            CheckVectorsEq(static_cast<TStringColumn*>(ans.get())->GetData(), std::vector<std::string>{
                "john",
                "johnfrusciante",
                "josh",
                "joshklinghoffer",
                "rhcp rhcp rhcp",
                "the getaway",
                "feasting on the flowers"
            });
        }
    }
}


TEST_F(OperatorsTest, FilterNLikeTest) {
    {
        std::vector<std::string> data = {
            "john",
            "johnfrusciante",
            "josh",
            "9",
            "8",
            "joshklinghoffer",
            "rhcp rhcp rhcp",
            "the getaway",
            "detroit",
            "dark neccessities",
            "encore",
            "feasting on the flowers"
        };
        auto m = MakeColumn(data, EColumn::kStringColumn);
        ASSERT_FALSE(m.HasError());

        {
            auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kNLike, "9");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetRes();

            ASSERT_TRUE(mask.size() == data.size());
            
            auto e = Do<OFilter>(m.GetRes(), mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetRes();
            ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
            CheckVectorsEq(static_cast<TStringColumn*>(ans.get())->GetData(), std::vector<std::string>{
                "john",
                "johnfrusciante",
                "josh",
                "8",
                "joshklinghoffer",
                "rhcp rhcp rhcp",
                "the getaway",
                "detroit",
                "dark neccessities",
                "encore",
                "feasting on the flowers"
            });
        }

        
        {
            auto r = Do<OFilterCheck>(m.GetRes(), EFilterType::kNLike, "%h%");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetRes();

            ASSERT_TRUE(mask.size() == data.size());
            
            auto e = Do<OFilter>(m.GetRes(), mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetRes();
            ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
            CheckVectorsEq(static_cast<TStringColumn*>(ans.get())->GetData(), std::vector<std::string>{
                "9",
                "8",
                "detroit",
                "dark neccessities",
                "encore"
            });
        }
    }
}

} // namespace JfEngine::Testing
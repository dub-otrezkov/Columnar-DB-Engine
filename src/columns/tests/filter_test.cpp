#include "tests.h"

#include "../operators/filter.h"

namespace JFEngine::Testing {


TEST_F(OperatorsTest, FilterEqTest) {
    {
        std::vector<std::string> data = {"-1", "10", "9", "9", "8"};
        auto m = MakeColumn(data, EColumn::ki64Column);

        ASSERT_FALSE(m.HasError());

        auto r = Do<OFilterCheck>(m.GetShared(), EFilterType::kEq, "9");

        ASSERT_FALSE(r.HasError());

        auto mask = r.GetShared();

        ASSERT_TRUE(mask->size() == 5);

        EXPECT_EQ(mask->at(0), 0);
        EXPECT_EQ(mask->at(1), 0);
        EXPECT_EQ(mask->at(2), 1);
        EXPECT_EQ(mask->at(3), 1);
        EXPECT_EQ(mask->at(4), 0);

        auto e = Do<OFilter>(m.GetShared(), *mask);
        ASSERT_FALSE(e.HasError());
        auto ans = e.GetShared();
        ASSERT_EQ(ans->GetSize(), 2);
    }
}

void CheckVectorsEq(
    const std::vector<std::string>& a,
    const std::vector<std::string>& b
) {
    EXPECT_EQ(a.size(), b.size());
    for (ui64 i = 0; i < a.size(); i++) {
        EXPECT_EQ(a[i], b[i]);
    }
}

TEST_F(OperatorsTest, FilteLikeTest) {
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
            auto r = Do<OFilterCheck>(m.GetShared(), EFilterType::kLike, "9");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetShared();

            ASSERT_TRUE(mask->size() == data.size());
            
            auto e = Do<OFilter>(m.GetShared(), *mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetShared();
            ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
            CheckVectorsEq(static_cast<TStringColumn*>(ans.get())->GetData(), std::vector<std::string>{
                "9"
            });
        }

        
        {
            auto r = Do<OFilterCheck>(m.GetShared(), EFilterType::kLike, "%h%");

            ASSERT_FALSE(r.HasError());

            auto mask = r.GetShared();

            ASSERT_TRUE(mask->size() == data.size());
            
            auto e = Do<OFilter>(m.GetShared(), *mask);
            ASSERT_FALSE(e.HasError());
            auto ans = e.GetShared();
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

} // namespace JFEngine::Testing
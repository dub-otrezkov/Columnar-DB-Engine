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


} // namespace JFEngine::Testing
#include "tests.h"

#include "../operators/filter.h"

namespace JfEngine::Testing {

TEST_F(OperatorsTest, IfElseMixedMask) {
    std::vector<std::string> data = {"a", "b", "c", "d", "e"};
    auto m = MakeColumn(data, EColumn::kStringColumn);
    ASSERT_FALSE(m.HasError());

    std::vector<bool> mask = {1, 0, 1, 0, 1};

    auto r = Do<OIfElse>(m.GetRes(), std::string{"ELSE"}, mask);
    ASSERT_FALSE(r.HasError());

    auto ans = r.GetRes();
    ASSERT_EQ(ans->GetType(), EColumn::kStringColumn);
    ASSERT_EQ(ans->GetSize(), 5);

    auto* sc = static_cast<TStringColumn*>(ans.get());
    EXPECT_EQ(std::string(sc->GetData().ro_at(0)), "a");
    EXPECT_EQ(std::string(sc->GetData().ro_at(1)), "ELSE");
    EXPECT_EQ(std::string(sc->GetData().ro_at(2)), "c");
    EXPECT_EQ(std::string(sc->GetData().ro_at(3)), "ELSE");
    EXPECT_EQ(std::string(sc->GetData().ro_at(4)), "e");
}

TEST_F(OperatorsTest, IfElseAllTrue) {
    std::vector<std::string> data = {"x", "y", "z"};
    auto m = MakeColumn(data, EColumn::kStringColumn);
    ASSERT_FALSE(m.HasError());

    std::vector<bool> mask = {1, 1, 1};

    auto r = Do<OIfElse>(m.GetRes(), std::string{"fallback"}, mask);
    ASSERT_FALSE(r.HasError());

    auto ans = r.GetRes();
    ASSERT_EQ(ans->GetSize(), 3);

    auto* sc = static_cast<TStringColumn*>(ans.get());
    EXPECT_EQ(std::string(sc->GetData().ro_at(0)), "x");
    EXPECT_EQ(std::string(sc->GetData().ro_at(1)), "y");
    EXPECT_EQ(std::string(sc->GetData().ro_at(2)), "z");
}

TEST_F(OperatorsTest, IfElseAllFalse) {
    std::vector<std::string> data = {"p", "q", "r"};
    auto m = MakeColumn(data, EColumn::kStringColumn);
    ASSERT_FALSE(m.HasError());

    std::vector<bool> mask = {0, 0, 0};

    auto r = Do<OIfElse>(m.GetRes(), std::string{"NA"}, mask);
    ASSERT_FALSE(r.HasError());

    auto ans = r.GetRes();
    ASSERT_EQ(ans->GetSize(), 3);

    auto* sc = static_cast<TStringColumn*>(ans.get());
    EXPECT_EQ(std::string(sc->GetData().ro_at(0)), "NA");
    EXPECT_EQ(std::string(sc->GetData().ro_at(1)), "NA");
    EXPECT_EQ(std::string(sc->GetData().ro_at(2)), "NA");
}

TEST_F(OperatorsTest, IfElseMaskLengthMismatch) {
    std::vector<std::string> data = {"a", "b", "c"};
    auto m = MakeColumn(data, EColumn::kStringColumn);
    ASSERT_FALSE(m.HasError());

    std::vector<bool> mask = {1, 0};

    auto r = Do<OIfElse>(m.GetRes(), std::string{"X"}, mask);
    EXPECT_TRUE(r.HasError());
    EXPECT_EQ(r.GetError(), EError::BadArgsErr);
}

TEST_F(OperatorsTest, IfElseEmptyColumn) {
    std::vector<std::string> data;
    auto m = MakeColumn(data, EColumn::kStringColumn);
    ASSERT_FALSE(m.HasError());

    std::vector<bool> mask;

    auto r = Do<OIfElse>(m.GetRes(), std::string{"X"}, mask);
    ASSERT_FALSE(r.HasError());

    auto ans = r.GetRes();
    EXPECT_EQ(ans->GetSize(), 0);
}

} // namespace JfEngine::Testing

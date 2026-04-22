#include "tests.h"

#include "../operators/order.h"

namespace JfEngine::Testing {

TEST_F(OperatorsTest, i64SumTest) {
    {
        std::vector<std::string> data1 = {"-1", "9", "9", "10"};
        std::vector<std::string> data2 = {"-2", "9", "8", "1", "5"};
        auto m1 = MakeColumn(data1, EColumn::ki64Column).GetShared();
        auto m2 = MakeColumn(data2, EColumn::ki64Column).GetShared();

        {
            auto [order, err] = Do<OMergeSort>(m1, m2);

            ASSERT_TRUE(!err);

            std::cout << "res:";
            for (auto& el : *order) {
                std::cout << " " << el;
            }

            std::cout << std::endl;

            auto [sorted, err2] = Do<OApply2>(m1, m2, *order);

            ASSERT_TRUE(!err2);

            auto t = static_cast<Ti64Column*>(sorted.get())->GetData();

            std::cout << "res:";
            for (auto& el : t) {
                std::cout << " " << el;
            }

            std::cout << std::endl;
        }
    }
    {
        std::vector<std::string> data1 = {"10", "9", "9", "-1"};
        std::vector<std::string> data2 = {"-2", "9", "8", "1", "5"};
        auto m1 = MakeColumn(data1, EColumn::ki64Column).GetShared();
        auto m2 = MakeColumn(data2, EColumn::ki64Column).GetShared();

        {
            auto [order, err] = Do<OMergeSort>(m1, m2, OMergeSort::kUnlimited, /*rev=*/true);

            ASSERT_TRUE(!err);

            std::cout << "res:";
            for (auto& el : *order) {
                std::cout << " " << el;
            }

            std::cout << std::endl;

            auto [sorted, err2] = Do<OApply2>(m1, m2, *order);

            ASSERT_TRUE(!err2);

            auto t = static_cast<Ti64Column*>(sorted.get())->GetData();

            std::cout << "res:";
            for (auto& el : t) {
                std::cout << " " << el;
            }

            std::cout << std::endl;
        }
    }
    {
        std::vector<std::string> data1 = {"10", "9", "9", "-1"};
        std::vector<std::string> data2 = {"-2", "9", "8", "1", "5"};
        auto m1 = MakeColumn(data1, EColumn::ki64Column).GetShared();
        auto m2 = MakeColumn(data2, EColumn::ki64Column).GetShared();

        {
            auto [order, err] = Do<OMergeSort>(m1, m2, /*limit=*/6, /*rev=*/true);

            ASSERT_TRUE(!err);

            std::cout << "res:";
            for (auto& el : *order) {
                std::cout << " " << el;
            }

            std::cout << std::endl;

            auto [sorted, err2] = Do<OApply2>(m1, m2, *order);

            ASSERT_TRUE(!err2);

            auto t = static_cast<Ti64Column*>(sorted.get())->GetData();

            std::cout << "res:";
            for (auto& el : t) {
                std::cout << " " << el;
            }

            std::cout << std::endl;
        }
    }
}

} // namespace JfEngine::Testing
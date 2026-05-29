#include "csv_reader.h"
#include "csv_writer.h"
#include "utils/mmap_input/mmap_input.h"

#include <gtest/gtest.h>

namespace JfEngine::Testing {

struct IoTests : ::testing::Test {
    // correct
    std::string basic = R"(john;frusciante
josh;klinghoffer
)";
    std::string advanced = R"("Scar ""Tissue""","Calif""ornica""tion","the,""Zephyr song"
",","""by the way""","the,""adventures"",of,""rain"" dance maggie"
)";
    std::string extra = R"(;hey;,,;)";

    // incorrect
    std::string unclosedQuote = R"(",remain)";
    std::string badQuote = R"(slow,"cheet"ah)";
};

TEST_F(IoTests, BasicRead) {
    TStringStreamFileInput in(basic);

    TCsvReader rr(&in, ';');
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 2);
        EXPECT_EQ(d.at(0), "john");
        EXPECT_EQ(d.at(1), "frusciante");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 2);
        EXPECT_EQ(d.at(0), "josh");
        EXPECT_EQ(d.at(1), "klinghoffer");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_TRUE(Is<EError::EofErr>(err));
    }
}

TEST_F(IoTests, BasicWrite) {
    TStringStreamFileOutput out;

    TCsvWriter rr(&out, ';');
    std::vector<std::string> i;
    i = {"john", "frusciante"};
    rr.WriteRow(i);
    i = {"josh", "klinghoffer"};
    rr.WriteRow(i);

    EXPECT_EQ(basic, out.Str());
}

TEST_F(IoTests, AdvancedRead) {
    TStringStreamFileInput in(advanced);

    TCsvReader rr(&in);
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 3);
        EXPECT_EQ(d.at(0), R"(Scar "Tissue")");
        EXPECT_EQ(d.at(1), R"(Calif"ornica"tion)");
        EXPECT_EQ(d.at(2), R"(the,"Zephyr song)");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 3);
        EXPECT_EQ(d.at(0), R"(,)");
        EXPECT_EQ(d.at(1), R"("by the way")");
        EXPECT_EQ(d.at(2), R"(the,"adventures",of,"rain" dance maggie)");
    }
}

TEST_F(IoTests, AdvancedWrite) {
    TStringStreamFileOutput out;

    TCsvWriter rr(&out);
    std::vector<std::string> i;
    i = {R"(Scar "Tissue")", R"(Calif"ornica"tion)", R"(the,"Zephyr song)"};
    rr.WriteRow(i);
    i = {R"(,)", R"("by the way")",R"(the,"adventures",of,"rain" dance maggie)"};
    rr.WriteRow(i);

    EXPECT_EQ(advanced, out.Str());
}

TEST_F(IoTests, EdgeCasesRead) {
    TStringStreamFileInput in(extra);

    TCsvReader rr(&in, ';');
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 4);
        EXPECT_EQ(d.at(0), "");
        EXPECT_EQ(d.at(1), "hey");
        EXPECT_EQ(d.at(2), ",,");
        EXPECT_EQ(d.at(3), "");
    }
}

TEST_F(IoTests, UnclosedQuoteRead) {
    TStringStreamFileInput in(unclosedQuote);

    TCsvReader rr(&in);

    auto [_, err] = rr.ReadRow();

    ASSERT_TRUE(Is<EError::EofErr>(err));
}

TEST_F(IoTests, BadQuoteRead) {
    TStringStreamFileInput in(badQuote);

    TCsvReader rr(&in);

    auto [_, err] = rr.ReadRow();

    ASSERT_TRUE(Is<EError::EofErr>(err));
}

TEST_F(IoTests, OptimizedRead) {
    TStringStreamFileInput in(advanced);

    TCsvOptimizedReader rr(&in);
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 3);
        EXPECT_EQ(d.at(0), R"(Scar "Tissue")");
        EXPECT_EQ(d.at(1), R"(Calif"ornica"tion)");
        EXPECT_EQ(d.at(2), R"(the,"Zephyr song)");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d.size(), 3);
        EXPECT_EQ(d.at(0), R"(,)");
        EXPECT_EQ(d.at(1), R"("by the way")");
        EXPECT_EQ(d.at(2), R"(the,"adventures",of,"rain" dance maggie)");
    }
}


TEST_F(IoTests, ExperimenatalBufferedRead) {
    {
        TStringStreamFileInput in(advanced);

        TCsvOptimizedReader rr(&in);
        TVectorString2d out;
        {
            auto err = rr.ReadRow(out);
            ASSERT_FALSE(err.HasError());
        }
        {
            auto err = rr.ReadRow(out);
            ASSERT_FALSE(err.HasError());
        }
        std::string target;
        out.At(0, 0, &target);
        EXPECT_EQ(target, R"(Scar "Tissue")");
        out.At(0, 1, &target);
        EXPECT_EQ(target, R"(Calif"ornica"tion)");
        out.At(0, 2, &target);
        EXPECT_EQ(target, R"(the,"Zephyr song)");
        out.At(1, 0, &target);
        EXPECT_EQ(target, R"(,)");
        out.At(1, 1, &target);
        EXPECT_EQ(target, R"("by the way")");
        out.At(1, 2, &target);
        EXPECT_EQ(target, R"(the,"adventures",of,"rain" dance maggie)");
    }
}

TEST_F(IoTests, EmptyCases) {
    {
        TStringStreamFileInput in("");
        TCsvReader rr(&in);
        auto [d, err] = rr.ReadRow();
        ASSERT_TRUE(Is<EError::EofErr>(err));
    }
    {
        TStringStreamFileInput in(R"(
)");
        TCsvReader rr(&in);
        auto [d, err] = rr.ReadRow();
        ASSERT_TRUE(Is<EError::EofErr>(err));
    }

}

} // namespace JfEngine::Testing

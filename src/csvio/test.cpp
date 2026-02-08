#include "csv_reader.h"
#include "csv_writer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace JFEngine::Testing {

struct IOTests : ::testing::Test {
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

TEST_F(IOTests, BasicRead) {
    std::stringstream in;
    in << basic;

    TCSVReader rr(in, kUnlimitedBuffer, ';');
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d->size(), 2);
        EXPECT_EQ(d->at(0), "john");
        EXPECT_EQ(d->at(1), "frusciante");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d->size(), 2);
        EXPECT_EQ(d->at(0), "josh");
        EXPECT_EQ(d->at(1), "klinghoffer");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_TRUE(Is<EofErr>(err));
    }
}

TEST_F(IOTests, BasicWrite) {
    std::stringstream out;

    TCSVWriter rr(out, ';');
    std::vector<std::string> i;
    i = {"john", "frusciante"};
    rr.WriteRow(i);
    i = {"josh", "klinghoffer"};
    rr.WriteRow(i);

    EXPECT_EQ(basic, out.str());
}

TEST_F(IOTests, AdvancedRead) {
    std::stringstream in;
    in << advanced;

    TCSVReader rr(in);
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d->size(), 3);
        EXPECT_EQ(d->at(0), R"(Scar "Tissue")");
        EXPECT_EQ(d->at(1), R"(Calif"ornica"tion)");
        EXPECT_EQ(d->at(2), R"(the,"Zephyr song)");
    }
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d->size(), 3);
        EXPECT_EQ(d->at(0), R"(,)");
        EXPECT_EQ(d->at(1), R"("by the way")");
        EXPECT_EQ(d->at(2), R"(the,"adventures",of,"rain" dance maggie)");
    }
}

TEST_F(IOTests, AdvancedWrite) {
    std::stringstream out;

    TCSVWriter rr(out);
    std::vector<std::string> i;
    i = {R"(Scar "Tissue")", R"(Calif"ornica"tion)", R"(the,"Zephyr song)"};
    rr.WriteRow(i);
    i = {R"(,)", R"("by the way")",R"(the,"adventures",of,"rain" dance maggie)"};
    rr.WriteRow(i);

    EXPECT_EQ(advanced, out.str());
}

TEST_F(IOTests, EdgeCasesRead) {
    std::stringstream in;
    in << extra;

    TCSVReader rr(in, kUnlimitedBuffer, ';');
    {
        auto [d, err] = rr.ReadRow();
        ASSERT_FALSE(err);
        ASSERT_EQ(d->size(), 4);
        EXPECT_EQ(d->at(0), "");
        EXPECT_EQ(d->at(1), "hey");
        EXPECT_EQ(d->at(2), ",,");
        EXPECT_EQ(d->at(3), "");
    }
}

TEST_F(IOTests, UnclosedQuoteRead) {
    std::stringstream in;
    in << unclosedQuote;

    TCSVReader rr(in);

    auto [_, err] = rr.ReadRow();

    ASSERT_TRUE(Is<EofErr>(err));
}

TEST_F(IOTests, BadQuoteRead) {
    std::stringstream in;
    in << badQuote;

    TCSVReader rr(in);

    auto [_, err] = rr.ReadRow();

    ASSERT_TRUE(Is<EofErr>(err));
}

} // namespace JFEngine::Testing

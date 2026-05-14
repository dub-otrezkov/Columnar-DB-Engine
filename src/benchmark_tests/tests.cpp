#include "executor/executor.h"
#include "ios_factory/ios_factory.h"
#include "workers/base.h"

#include <gtest/gtest.h>

#include <memory>
#include <string_view>

namespace JfEngine::Testing {

struct BenchTest : testing::Test {

    std::string scheme = R"(what,int64
once,int32
was,string
hers,string
ste,int16
audun,string
getaway,int8
empty,string
low,date
beam,timestamp
)";
    std::string data = R"(1,2,josh,rip,4,laading,1,"",2023-05-15,2023-05-15 14:30:22
3,4,john,rip,5,she needs him,1,dolores,2022-11-08,2021-11-08 09:45:17
5,6,frusciante,forever,6,harvey,2,dolores,2022-03-27,2022-03-27 23:18:43
7,8,klinghoffer,alive,6,"low,beam",2,"",2020-09-12,2020-09-12 07:52:09
5,8,john,alive,8,dorothy,2,"",2024-01-30,2024-01-30 16:40:55
1,4,klinghoffer,forever,1,if you know what's right,4,dolores2,2022-07-19,2019-07-19 11:33:41
7,2,frusciante,forever,1,speed racer,2,dolores1,2022-12-03,2023-12-03 20:15:28
3,7,josh,forever,1,cool with u,1,dolores1,2022-06-21,2022-06-21 05:08:14
)";

    static constexpr ui64 iter = 50000;

    std::shared_ptr<std::stringstream> out_scheme;
    std::shared_ptr<std::stringstream> out_data;

    void SetUp() override {
        TIoFactory::RegisterSStreamIo("scheme", ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo("data", ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo("josh", ETypeFile::kJfFile);
        TIoFactory::RegisterSStreamIo("tmp1", ETypeFile::kJfFile);
        TIoFactory::RegisterSStreamIo("tmp2", ETypeFile::kJfFile);

        *TIoFactory::GetIo("scheme") << scheme;
        for (ui64 i = 0; i < iter; i++) {
            *TIoFactory::GetIo("data") << data;
        }

        TIoFactory::RegisterSStreamIo(kResultScheme, ETypeFile::kCsvFile);
        TIoFactory::RegisterSStreamIo(kResultData, ETypeFile::kCsvFile);

        out_scheme = std::dynamic_pointer_cast<std::stringstream>(
            TIoFactory::GetIo(kResultScheme)
        );
        out_data = std::dynamic_pointer_cast<std::stringstream>(
            TIoFactory::GetIo(kResultData)
        );
    }

    void TearDown() override {
        TIoFactory::Clear();

        TIoFactory::UnregisterIo("scheme");
        TIoFactory::UnregisterIo("data");
        TIoFactory::UnregisterIo("josh");
        TIoFactory::UnregisterIo("tmp1");
        TIoFactory::UnregisterIo("tmp2");

        TIoFactory::UnregisterIo(kResultData);
        TIoFactory::UnregisterIo(kResultScheme);

        TMemoryArena::Instance().Reset();
    }
};

void prolog(JfEngine::TExecutor& exec) {
    auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
    if (err.HasError()) {
        std::cout << err.GetError() << std::endl;
    }
    ASSERT_FALSE(err.HasError());
}

TEST_F(BenchTest, _0) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), "400000\n");
}

TEST_F(BenchTest, _1) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh WHERE what <> 1");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), "300000\n");
}

TEST_F(BenchTest, _2) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT SUM(what), COUNT(*), AVG(once) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int128
COUNT(*),int64
AVG(once),int128
)");
    EXPECT_EQ(out_data->str(), "1600000,400000,5\n");
}

TEST_F(BenchTest, _3) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT AVG(once) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(AVG(once),int128
)");
    EXPECT_EQ(out_data->str(), "5\n");
}

TEST_F(BenchTest, _4) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT_DISTINCT(once) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT_DISTINCT(once),int64
)");
    EXPECT_EQ(out_data->str(), "5\n");
}

TEST_F(BenchTest, _5) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT_DISTINCT(was) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT_DISTINCT(was),int64
)");
    EXPECT_EQ(out_data->str(), "4\n");
}

TEST_F(BenchTest, _6) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT MIN(low), MAX(low) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(MIN(low),date
MAX(low),date
)");
    EXPECT_EQ(out_data->str(), R"(2020-09-12,2024-01-30
)");
}

TEST_F(BenchTest, _7) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT once, COUNT(*) FROM josh WHERE once <> 2 GROUP BY once ORDER BY 'COUNT(*)' DESC, once");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(once,int32
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(8,100000
4,100000
7,50000
6,50000
)");
}

TEST_F(BenchTest, _8) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, COUNT_DISTINCT(what) AS u FROM josh GROUP BY hers ORDER BY u DESC, hers LIMIT 2");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(hers,string
u,int64
)");
    EXPECT_EQ(out_data->str(), R"(forever,4
rip,2
)");
}

TEST_F(BenchTest, _9) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, SUM(what), COUNT(*) AS c, AVG(once), COUNT_DISTINCT(was) FROM josh GROUP BY hers ORDER BY c DESC, hers LIMIT 2");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(hers,string
SUM(what),int128
c,int64
AVG(once),int128
COUNT_DISTINCT(was),int64
)");
    EXPECT_EQ(out_data->str(), R"(forever,800000,200000,4,3
rip,200000,100000,3,2
)");
}

TEST_F(BenchTest, _10) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, COUNT_DISTINCT(was) AS u FROM josh WHERE hers <> 'rip' GROUP BY hers ORDER BY u DESC, hers LIMIT 1");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(hers,string
u,int64
)");
    EXPECT_EQ(out_data->str(), R"(forever,3
)");
}

TEST_F(BenchTest, _11) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, hers, COUNT_DISTINCT(what) AS u FROM josh WHERE what <> 3 GROUP BY was, hers ORDER BY u DESC, was, hers LIMIT 4");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
hers,string
u,int64
)");
    EXPECT_EQ(out_data->str(), R"(frusciante,forever,2
klinghoffer,forever,1
klinghoffer,alive,1
josh,rip,1
)");
}

TEST_F(BenchTest, _12) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS c FROM josh WHERE was <> '' GROUP BY was ORDER BY c DESC, was LIMIT 5");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,100000
josh,100000
john,100000
frusciante,100000
)");
}

TEST_F(BenchTest, _13) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT_DISTINCT(was) AS u FROM josh WHERE was <> 'josh' GROUP BY was ORDER BY u DESC, was LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
u,int64
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,1
john,1
frusciante,1
)");
}

TEST_F(BenchTest, _14) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT getaway, was, COUNT(*) AS c FROM josh WHERE was <> 'josh' GROUP BY getaway, was ORDER BY c DESC, getaway, was LIMIT 3");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(getaway,int8
was,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(2,frusciante,100000
4,klinghoffer,50000
2,klinghoffer,50000
)");
}

TEST_F(BenchTest, _15) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT getaway, COUNT(*) FROM josh GROUP BY getaway ORDER BY 'COUNT(*)' DESC, getaway LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(getaway,int8
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(2,200000
1,150000
4,50000
)");
}

TEST_F(BenchTest, _16) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) FROM josh GROUP BY was, getaway ORDER BY 'COUNT(*)' DESC, was, getaway LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
getaway,int8
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(josh,1,100000
frusciante,2,100000
klinghoffer,4,50000
klinghoffer,2,50000
john,2,50000
john,1,50000
)");
}

TEST_F(BenchTest, _17) {
    // EXTRACT_MINUTE в GROUP BY.

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT what, EXTRACT_MINUTE(beam) AS m, was, COUNT(*) AS c "
            "FROM josh "
            "GROUP BY what, m, was "
            "ORDER BY c, what, m, was DESC LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Все 8 (what, m, was) тройки уникальны, c=iter=50000 каждая.
    // ORDER BY c, what, m, was DESC (все DESC): ties по c, сортируем what DESC, m DESC, was DESC.
    EXPECT_EQ(out_scheme->str(), R"(what,int64
m,int64
was,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(7,52,klinghoffer,50000
7,15,frusciante,50000
5,40,john,50000
5,18,frusciante,50000
3,45,john,50000
3,8,josh,50000
1,33,klinghoffer,50000
1,30,josh,50000
)");
}

TEST_F(BenchTest, _18) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT what FROM josh WHERE what = 5");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(what,int64
)");
    // 2 rows per block match (rows with what=5), output "5\n" each
    EXPECT_EQ(out_data->str().size(), iter * 2 * 2);
}

TEST_F(BenchTest, _19) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT getaway FROM josh WHERE getaway = 2");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(getaway,int8
)");
    EXPECT_EQ(out_data->str().size(), 400000);
}

TEST_F(BenchTest, _20) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh WHERE was LIKE '%f%'");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), "200000\n");
}

TEST_F(BenchTest, _21) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, MIN(hers), COUNT(*) AS c FROM josh WHERE was LIKE '%f%' AND was <> '' GROUP BY was ORDER BY c DESC, was LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
MIN(hers),string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,alive,100000
frusciante,forever,100000
)");
}

TEST_F(BenchTest, _22) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, MIN(what), MIN(once), COUNT(*) AS c, COUNT_DISTINCT(what) FROM josh WHERE was LIKE '%o%' AND was NOT LIKE 'k%' AND was <> '' GROUP BY was ORDER BY c DESC, was LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
MIN(what),int64
MIN(once),int32
c,int64
COUNT_DISTINCT(what),int64
)");
    EXPECT_EQ(out_data->str(), R"(josh,1,2,100000,2
john,3,4,100000,2
)");
}

TEST_F(BenchTest, _23) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT what, was, low FROM josh WHERE was LIKE '%o%' ORDER BY low LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Earliest `low` among rows matching LIKE '%o%' is 2020-09-12 (row 4: klinghoffer, what=7).
    // Повторяется iter раз подряд, LIMIT 10 все попадают в эту группу.
    EXPECT_EQ(out_scheme->str(), R"(what,int64
was,string
low,date
)");
    EXPECT_EQ(out_data->str(), R"(7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
7,klinghoffer,2020-09-12
)");
}

TEST_F(BenchTest, _24) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT empty FROM josh WHERE empty <> '' ORDER BY beam LIMIT 10;");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(empty,string
)");
    EXPECT_EQ(out_data->str(), R"(dolores2
dolores2
dolores2
dolores2
dolores2
dolores2
dolores2
dolores2
dolores2
dolores2
)");
}

TEST_F(BenchTest, _25) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT empty FROM josh WHERE empty <> '' ORDER BY was LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(empty,string
)");
    EXPECT_EQ(out_data->str(), R"(dolores1
dolores
dolores1
dolores
dolores1
dolores
dolores1
dolores
dolores1
dolores
)");
}

TEST_F(BenchTest, _26) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT empty FROM josh WHERE empty <> '' ORDER BY empty LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(empty,string
)");
    EXPECT_EQ(out_data->str(), R"(dolores
dolores
dolores
dolores
dolores
dolores
dolores
dolores
dolores
dolores
)");
}

TEST_F(BenchTest, _27) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT ste, AVG(LENGTH(was)) AS l, COUNT(*) FROM josh WHERE empty <> '' GROUP BY ste  HAVING l > 5 ORDER BY l DESC, ste");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(ste,int16
l,int128
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(6,10,50000
1,8,150000
)");
}

TEST_F(BenchTest, _29) {
    constexpr ui64 its = 31;

    JfEngine::TExecutor exec;
    prolog(exec);
    std::string q = "SELECT SUM(what), ";
    for (i64 i = 1; i < its; i++) {
        q += "SUM(+(what, " + std::to_string(i) + ")), ";
    }
    q.pop_back();
    q.pop_back();
    q += " FROM josh";

    {
        auto err = exec.ExecQuery(q);
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    std::string ans_scheme = "SUM(what),int128\n";
    for (ui64 i = 1; i < its; i++) {
        ans_scheme += "SUM(+(what " + std::to_string(i) + ")),int128\n";
    }

    std::string ans_data;
    for (i64 i = 0; i < its; i++) {
        ans_data += std::to_string(1600000 + static_cast<i64>(i * iter * 8)) + ",";
    }
    ans_data.back() = '\n';

    EXPECT_EQ(out_scheme->str(), ans_scheme);
    EXPECT_EQ(out_data->str(), ans_data);
}

TEST_F(BenchTest, _30) {
    constexpr ui64 its = 31;

    JfEngine::TExecutor exec;
    prolog(exec);
    std::string q = "SELECT SUM(what), ";
    for (i64 i = 1; i < its; i++) {
        q += "SUM(-(what, " + std::to_string(i) + ")), ";
    }
    q.pop_back();
    q.pop_back();
    q += " FROM josh";

    {
        auto err = exec.ExecQuery(q);
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    std::string ans_scheme = "SUM(what),int128\n";
    for (ui64 i = 1; i < its; i++) {
        ans_scheme += "SUM(-(what " + std::to_string(i) + ")),int128\n";
    }

    std::string ans_data;

    for (i64 i = 0; i < its; i++) {
        // std::cout << 1600000 - static_cast<i64>(i * iter * 8) << std::endl;
        ans_data += std::to_string(1600000 - static_cast<i64>(i * iter * 8)) + ",";
    }
    ans_data.back() = '\n';

    EXPECT_EQ(out_scheme->str(), ans_scheme);
    EXPECT_EQ(out_data->str(), ans_data);
}

TEST_F(BenchTest, _31) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh WHERE was <> '' GROUP BY was, getaway ORDER BY 'AVG(once)' DESC, was, getaway LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,2,50000,350000,8
john,2,50000,250000,8
klinghoffer,4,50000,50000,4
josh,1,100000,200000,4
john,1,50000,150000,4
frusciante,2,100000,600000,4
)");
}

TEST_F(BenchTest, _32) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh WHERE empty <> '' GROUP BY was, getaway ORDER BY 'AVG(once)' DESC, was, getaway LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->str(), R"(josh,1,50000,150000,7
klinghoffer,4,50000,50000,4
john,1,50000,150000,4
frusciante,2,100000,600000,4
)");
}

TEST_F(BenchTest, _33) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh GROUP BY was, getaway ORDER BY 'AVG(once)' DESC, was, getaway LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,2,50000,350000,8
john,2,50000,250000,8
klinghoffer,4,50000,50000,4
josh,1,100000,200000,4
john,1,50000,150000,4
frusciante,2,100000,600000,4
)");
}

TEST_F(BenchTest, _34) {
    // CONST_INT в SELECT и GROUP BY.

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT CONST_INT(1), was, COUNT(*) AS c "
            "FROM josh "
            "GROUP BY CONST_INT(1), was "
            "ORDER BY c, was DESC LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // CONST_INT(1) даёт одинаковую константу во всех строках; группировка сводится к GROUP BY was.
    // 4 группы, каждая c = 2*iter = 100000. ORDER BY c, was DESC → все DESC.
    EXPECT_EQ(out_scheme->str(), R"(CONST_INT(1),int64
was,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(1,klinghoffer,100000
1,josh,100000
1,john,100000
1,frusciante,100000
)");
}

TEST_F(BenchTest, _35) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT what, -(what, 1), -(what, 2), COUNT(*) AS c "
            "FROM josh "
            "GROUP BY what, -(what, 1), -(what, 2) "
            "ORDER BY c DESC, what LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(what,int64
-(what 1),int64
-(what 2),int64
c,int64
)");
    // 4 groups (what ∈ {1,3,5,7}), each with count = 2 * iter = 100000.
    // Engine uses single direction: ORDER BY c DESC, what ⇒ both DESC.
    // So expect what descending: 7, 5, 3, 1.
    EXPECT_EQ(out_data->str(), R"(7,6,5,100000
5,4,3,100000
3,2,1,100000
1,0,-1,100000
)");
}

TEST_F(BenchTest, _36) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT was, COUNT(*) AS c "
            "FROM josh "
            "WHERE low >= '2022-01-01' AND low <= '2022-12-31' "
            "AND getaway <> 0 AND empty <> '' "
            "GROUP BY was ORDER BY c, was DESC LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Rows matching filters per block:
    //   row 2 (john, dolores,  2022-11-08)
    //   row 3 (frusciante, dolores, 2022-03-27)
    //   row 6 (klinghoffer, dolores2, 2022-07-19)
    //   row 7 (frusciante, dolores1, ...): beam is 2023-12-03 but low is 2022-12-03 — YES
    //   row 8 (josh, dolores1, 2022-06-21)
    // Row 7: low=2022-12-03 matches; empty=dolores1 matches; getaway=2 matches. YES.
    // Groups: frusciante×2=100000, john×1=50000, josh×1=50000, klinghoffer×1=50000
    EXPECT_EQ(out_scheme->str(), R"(was,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(frusciante,100000
klinghoffer,50000
josh,50000
john,50000
)");
}

TEST_F(BenchTest, _37) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS cnt FROM josh WHERE ste = 1 AND low <= '2022-12-31' AND low >= '2022-01-01' AND empty <> '' GROUP BY was ORDER BY cnt DESC, was LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
cnt,int64
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,50000
josh,50000
frusciante,50000
)");
}

TEST_F(BenchTest, _38) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        // CLI 38 — аналог с OFFSET
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS cnt FROM josh WHERE ste = 1 AND low <= '2022-12-31' AND low >= '2022-01-01' AND empty <> '' GROUP BY was ORDER BY cnt, was DESC LIMIT 10 OFFSET 1");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Те же 3 группы, что в _37, но после OFFSET 1 отбрасываем klinghoffer.
    EXPECT_EQ(out_scheme->str(), R"(was,string
cnt,int64
)");
    EXPECT_EQ(out_data->str(), R"(josh,50000
frusciante,50000
)");
}

TEST_F(BenchTest, _39) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT IF (was = 'josh' THEN hers ELSE 'other') AS label FROM josh"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(label,string
)");
    // Per 8-row block: "rip\n" (was=josh, hers=rip) + 6 × "other\n"
    //                  + "forever\n" (was=josh, hers=forever)
    // Sizes: 4 + 6*6 + 8 = 48 bytes per block.
    EXPECT_EQ(out_data->str().size(), iter * 48);
    EXPECT_EQ(out_data->str().substr(0, 4), "rip\n");
}

TEST_F(BenchTest, _40) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT what, low, COUNT(*) AS c "
            "FROM josh "
            "WHERE what IN (1, 3, 5) AND low >= '2022-01-01' AND low <= '2023-12-31' "
            "AND empty <> '' "
            "GROUP BY what, low "
            "ORDER BY c DESC, what, low LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Rows matching per block: row 2 (3, 2022-11-08, dolores),
    //   row 3 (5, 2022-03-27, dolores),
    //   row 6 (1, 2022-07-19, dolores2),
    //   row 8 (3, 2022-06-21, dolores1)
    // 4 groups, each with count = iter = 50000.
    // Sort by c DESC then (what, low) DESC (single direction):
    //   5, 2022-03-27; 3, 2022-11-08; 3, 2022-06-21; 1, 2022-07-19
    EXPECT_EQ(out_scheme->str(), R"(what,int64
low,date
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(5,2022-03-27,50000
3,2022-11-08,50000
3,2022-06-21,50000
1,2022-07-19,50000
)");
}

TEST_F(BenchTest, _41) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT once, what, COUNT(*) AS c "
            "FROM josh "
            "WHERE ste >= 1 AND ste <= 10 AND empty <> '' "
            "GROUP BY once, what "
            "ORDER BY c, once, what DESC LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // Rows matching per block (ste in [1,10] AND empty<>''):
    //   row 2 (once=4, what=3, ste=5, empty=dolores)
    //   row 3 (once=6, what=5, ste=6, empty=dolores)
    //   row 6 (once=4, what=1, ste=1, empty=dolores2)
    //   row 7 (once=2, what=7, ste=1, empty=dolores1)
    //   row 8 (once=7, what=3, ste=1, empty=dolores1)
    // 5 groups, each c = iter = 50000.
    // ORDER BY c, once, what DESC (все DESC):
    //   once=7 → (7, 3); once=6 → (6, 5); once=4 ties → what DESC: (4, 3), (4, 1); once=2 → (2, 7)
    EXPECT_EQ(out_scheme->str(), R"(once,int32
what,int64
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(7,3,50000
6,5,50000
4,3,50000
4,1,50000
2,7,50000
)");
}

TEST_F(BenchTest, _42) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT TRUNC_MINUTE(beam) AS M, COUNT(*) AS c "
            "FROM josh "
            "GROUP BY M "
            "ORDER BY M LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // All 8 beam values distinct; TRUNC_MINUTE zeroes the seconds.
    // 8 groups, each c = iter = 50000.
    // ORDER BY M (ASC — no DESC keyword): chronological.
    EXPECT_EQ(out_scheme->str(), R"(M,timestamp
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(2019-07-19 11:33:00,50000
2020-09-12 07:52:00,50000
2021-11-08 09:45:00,50000
2022-03-27 23:18:00,50000
2022-06-21 05:08:00,50000
2023-05-15 14:30:00,50000
2023-12-03 20:15:00,50000
2024-01-30 16:40:00,50000
)");
}

TEST_F(BenchTest, _28) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT REGEXP_REPLACE(was, '^j', 'J') AS k, COUNT(*) AS c "
            "FROM josh "
            "GROUP BY k ORDER BY k LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // was values: josh(×2), john(×2), frusciante(×2), klinghoffer(×2) per block.
    // REGEXP_REPLACE('^j','J'): josh→Josh, john→John, others unchanged.
    // 4 groups × 2 rows × iter = 100000 each.
    // ORDER BY k ASC: 'J'(74) < 'f'(102) < 'k'(107) → John, Josh, frusciante, klinghoffer.
    EXPECT_EQ(out_scheme->str(), R"(k,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(John,100000
Josh,100000
frusciante,100000
klinghoffer,100000
)");
}

} // namespace JfEngine::Testing
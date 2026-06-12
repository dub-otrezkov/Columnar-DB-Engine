#include "executor/executor.h"
#include "ios_factory/ios_factory.h"
#include "utils/mmap_input/mmap_input.h"
#include "workers/base.h"

#include <gtest/gtest.h>

#include <memory>

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

    std::shared_ptr<TStringStreamFileOutput> out_scheme;
    std::shared_ptr<TStringStreamFileOutput> out_data;

    void SetUp() override {
        auto scheme_in = std::make_shared<TStringStreamFileInput>(scheme);
        auto data_in   = std::make_shared<TStringStreamFileInput>();
        for (ui64 i = 0; i < iter; i++) {
            data_in->Stream() << data;
        }

        TIoFactory::RegisterCustomInput("scheme", scheme_in);
        TIoFactory::RegisterCustomInput("data",   data_in);
        TIoFactory::RegisterSStreamIo("josh", ETypeFile::kJfFile);
        TIoFactory::RegisterSStreamIo("tmp1", ETypeFile::kJfFile);
        TIoFactory::RegisterSStreamIo("tmp2", ETypeFile::kJfFile);

        out_scheme = std::make_shared<TStringStreamFileOutput>();
        out_data   = std::make_shared<TStringStreamFileOutput>();
        TIoFactory::RegisterCustomOutput(kResultScheme, out_scheme);
        TIoFactory::RegisterCustomOutput(kResultData,   out_data);
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

    EXPECT_EQ(out_scheme->Str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), "400000\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), "300000\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(SUM(what),int128
COUNT(*),int64
AVG(once),int128
)");
    EXPECT_EQ(out_data->Str(), "1600000,400000,5\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(AVG(once),int128
)");
    EXPECT_EQ(out_data->Str(), "5\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(COUNT_DISTINCT(once),int64
)");
    EXPECT_EQ(out_data->Str(), "5\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(COUNT_DISTINCT(was),int64
)");
    EXPECT_EQ(out_data->Str(), "4\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(MIN(low),date
MAX(low),date
)");
    EXPECT_EQ(out_data->Str(), R"(2020-09-12,2024-01-30
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

    EXPECT_EQ(out_scheme->Str(), R"(once,int32
COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), R"(8,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(hers,string
u,int64
)");
    EXPECT_EQ(out_data->Str(), R"(forever,4
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

    EXPECT_EQ(out_scheme->Str(), R"(hers,string
SUM(what),int128
c,int64
AVG(once),int128
COUNT_DISTINCT(was),int64
)");
    EXPECT_EQ(out_data->Str(), R"(forever,800000,200000,4,3
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

    EXPECT_EQ(out_scheme->Str(), R"(hers,string
u,int64
)");
    EXPECT_EQ(out_data->Str(), R"(forever,3
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
hers,string
u,int64
)");
    EXPECT_EQ(out_data->Str(), R"(frusciante,forever,2
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,100000
josh,100000
john,100000
frusciante,100000
)");
}

TEST_F(BenchTest, _13) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT_DISTINCT(hers) AS u FROM josh WHERE was <> 'josh' GROUP BY was ORDER BY u, was DESC LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->Str(), R"(was,string
u,int64
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,2
john,2
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

    EXPECT_EQ(out_scheme->Str(), R"(getaway,int8
was,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(2,frusciante,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(getaway,int8
COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), R"(2,200000
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
getaway,int8
COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), R"(josh,1,100000
frusciante,2,100000
klinghoffer,4,50000
klinghoffer,2,50000
john,2,50000
john,1,50000
)");
}

TEST_F(BenchTest, _17) {

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

    EXPECT_EQ(out_scheme->Str(), R"(what,int64
m,int64
was,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(7,52,klinghoffer,50000
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

    EXPECT_EQ(out_scheme->Str(), R"(what,int64
)");
    EXPECT_EQ(out_data->Str().size(), iter * 2 * 2);
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

    EXPECT_EQ(out_scheme->Str(), R"(getaway,int8
)");
    EXPECT_EQ(out_data->Str().size(), 400000);
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

    EXPECT_EQ(out_scheme->Str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), "200000\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
MIN(hers),string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,alive,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
MIN(what),int64
MIN(once),int32
c,int64
COUNT_DISTINCT(what),int64
)");
    EXPECT_EQ(out_data->Str(), R"(josh,1,2,100000,2
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

    EXPECT_EQ(out_scheme->Str(), R"(what,int64
was,string
low,date
)");
    EXPECT_EQ(out_data->Str(), R"(7,klinghoffer,2020-09-12
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

    EXPECT_EQ(out_scheme->Str(), R"(empty,string
)");
    EXPECT_EQ(out_data->Str(), R"(dolores2
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

    EXPECT_EQ(out_scheme->Str(), R"(empty,string
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

    EXPECT_EQ(out_scheme->Str(), R"(empty,string
)");
    EXPECT_EQ(out_data->Str(), R"(dolores
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

    EXPECT_EQ(out_scheme->Str(), R"(ste,int16
l,int128
COUNT(*),int64
)");
    EXPECT_EQ(out_data->Str(), R"(6,10,50000
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

    EXPECT_EQ(out_scheme->Str(), ans_scheme);
    EXPECT_EQ(out_data->Str(), ans_data);
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
        ans_data += std::to_string(1600000 - static_cast<i64>(i * iter * 8)) + ",";
    }
    ans_data.back() = '\n';

    EXPECT_EQ(out_scheme->Str(), ans_scheme);
    EXPECT_EQ(out_data->Str(), ans_data);
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,2,50000,350000,8
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->Str(), R"(josh,1,50000,150000,7
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
getaway,int8
c,int64
SUM(what),int128
AVG(once),int128
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,2,50000,350000,8
john,2,50000,250000,8
klinghoffer,4,50000,50000,4
josh,1,100000,200000,4
john,1,50000,150000,4
frusciante,2,100000,600000,4
)");
}

TEST_F(BenchTest, _34) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery(
            "SELECT CONST_INT(1), was, COUNT(*) AS c "
            "FROM josh "
            "GROUP BY was "
            "ORDER BY c, was DESC LIMIT 10"
        );
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->Str(), R"(CONST_INT(1),int64
was,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(1,klinghoffer,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(what,int64
-(what 1),int64
-(what 2),int64
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(7,6,5,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(frusciante,100000
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

    EXPECT_EQ(out_scheme->Str(), R"(was,string
cnt,int64
)");
    EXPECT_EQ(out_data->Str(), R"(klinghoffer,50000
josh,50000
frusciante,50000
)");
}

TEST_F(BenchTest, _38) {

    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS cnt FROM josh WHERE ste = 1 AND low <= '2022-12-31' AND low >= '2022-01-01' AND empty <> '' GROUP BY was ORDER BY cnt, was DESC LIMIT 10 OFFSET 1");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->Str(), R"(was,string
cnt,int64
)");
    EXPECT_EQ(out_data->Str(), R"(josh,50000
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

    EXPECT_EQ(out_scheme->Str(), R"(label,string
)");
    EXPECT_EQ(out_data->Str().size(), iter * 48);
    EXPECT_EQ(out_data->Str().substr(0, 4), "rip\n");
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

    EXPECT_EQ(out_scheme->Str(), R"(what,int64
low,date
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(5,2022-03-27,50000
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

    EXPECT_EQ(out_scheme->Str(), R"(once,int32
what,int64
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(7,3,50000
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

    EXPECT_EQ(out_scheme->Str(), R"(M,timestamp
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(2019-07-19 11:33:00,50000
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

    EXPECT_EQ(out_scheme->Str(), R"(k,string
c,int64
)");
    EXPECT_EQ(out_data->Str(), R"(John,100000
Josh,100000
frusciante,100000
klinghoffer,100000
)");
}

} // namespace JfEngine::Testing

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
        TIoFactory::UnregisterIo("scheme");
        TIoFactory::UnregisterIo("data");
        TIoFactory::UnregisterIo("josh");
        TIoFactory::UnregisterIo("tmp1");
        TIoFactory::UnregisterIo("tmp2");

        TIoFactory::UnregisterIo(kResultData);
        TIoFactory::UnregisterIo(kResultScheme);
    }
};

void prolog(JfEngine::TExecutor& exec) {
    auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
    if (err.HasError()) {
        std::cout << err.GetError() << std::endl;
    }
    ASSERT_FALSE(err.HasError());
}

TEST_F(BenchTest, _1) {
    
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

TEST_F(BenchTest, _2) {
    
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

TEST_F(BenchTest, _3) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT SUM(what), COUNT(*), AVG(once) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int64
COUNT(*),int64
AVG(once),double
)");
    EXPECT_EQ(out_data->str(), "1600000,400000,5.125\n");
}

TEST_F(BenchTest, _4) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT AVG(once) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(AVG(once),double
)");
    EXPECT_EQ(out_data->str(), "5.125\n");
}

TEST_F(BenchTest, _5) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT(DISTINCT(once)) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(DISTINCT(once)),int64
)");
    EXPECT_EQ(out_data->str(), "5\n");
}

TEST_F(BenchTest, _6) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT COUNT(DISTINCT(was)) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(DISTINCT(was)),int64
)");
    EXPECT_EQ(out_data->str(), "4\n");
}

TEST_F(BenchTest, _7) {

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

TEST_F(BenchTest, _8) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT once, COUNT(*) FROM josh WHERE once <> 2 GROUP BY once ORDER BY 'COUNT(*)' DESC");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(once,int32
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(4,100000
8,100000
6,50000
7,50000
)");
}

TEST_F(BenchTest, _9) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, COUNT(DISTINCT(what)) AS u FROM josh GROUP BY hers ORDER BY u DESC LIMIT 2");
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
alive,2
)");
}

TEST_F(BenchTest, _10) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, SUM(what), COUNT(*) AS c, AVG(once), COUNT(DISTINCT(was)) FROM josh GROUP BY hers ORDER BY c DESC LIMIT 2");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(hers,string
SUM(what),int64
c,int64
AVG(once),double
COUNT(DISTINCT(was)),int64
)");
    EXPECT_EQ(out_data->str(), R"(forever,800000,200000,4.75,3
alive,600000,100000,8,2
)");
}

TEST_F(BenchTest, _11) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT hers, COUNT(DISTINCT(was)) AS u FROM josh WHERE hers <> 'rip' GROUP BY hers ORDER BY u DESC LIMIT 1");
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

TEST_F(BenchTest, _12) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, hers, COUNT(DISTINCT(what)) AS u FROM josh WHERE what <> 3 GROUP BY was, hers ORDER BY u DESC LIMIT 4");
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
john,alive,1
josh,rip,1
klinghoffer,alive,1
)");
}

TEST_F(BenchTest, _13) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS c FROM josh WHERE was <> '' GROUP BY was ORDER BY c DESC LIMIT 5");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,100000
john,100000
josh,100000
klinghoffer,100000
)");
}

TEST_F(BenchTest, _14) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(DISTINCT(was)) AS u FROM josh WHERE was <> 'josh' GROUP BY was ORDER BY u DESC LIMIT 10");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,1
john,1
klinghoffer,1
)");
}

TEST_F(BenchTest, _15) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT getaway, was, COUNT(*) AS c FROM josh WHERE was <> 'josh' GROUP BY getaway, was ORDER BY c DESC LIMIT 3");
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
1,john,50000
2,john,50000
)");
}

TEST_F(BenchTest, _16) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT getaway, COUNT(*) FROM josh GROUP BY getaway ORDER BY 'COUNT(*)' DESC LIMIT 10");
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

TEST_F(BenchTest, _17) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) FROM josh GROUP BY was, getaway ORDER BY 'COUNT(*)' DESC LIMIT 10");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,2,100000
josh,1,100000
john,1,50000
john,2,50000
klinghoffer,2,50000
klinghoffer,4,50000
)");
}

TEST_F(BenchTest, _18) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) FROM josh GROUP BY was, getaway LIMIT 3");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,2,100000
john,1,50000
josh,1,100000
)");
}

TEST_F(BenchTest, _20) {
    
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

TEST_F(BenchTest, _21) {
    
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

TEST_F(BenchTest, _22) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, MIN(hers), COUNT(*) AS c FROM josh WHERE was LIKE '%f%' AND was <> '' GROUP BY was ORDER BY c DESC LIMIT 10");
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

TEST_F(BenchTest, _23) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, MIN(what), MIN(once), COUNT(*) AS c, COUNT(DISTINCT(what)) FROM josh WHERE was LIKE '%o%' AND was NOT LIKE 'k%' AND was <> '' GROUP BY was ORDER BY c DESC LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(was,string
MIN(what),int64
MIN(once),int64
c,int64
COUNT(DISTINCT(what)),int64
)");
    EXPECT_EQ(out_data->str(), R"(john,3,4,5000,2
josh,1,2,5000,2
)");
}

TEST_F(BenchTest, _24) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was FROM josh WHERE was LIKE '%o%' ORDER BY low LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
klinghoffer
)");
}

TEST_F(BenchTest, _25) {
    
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

TEST_F(BenchTest, _26) {
    
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
    EXPECT_EQ(out_data->str(), R"(dolores
dolores1
dolores
dolores1
dolores
dolores1
dolores
dolores1
dolores
dolores1
)");
}

TEST_F(BenchTest, _27) {
    
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

TEST_F(BenchTest, _28) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT ste, AVG(LENGTH(was)) AS l, COUNT(*) FROM josh WHERE empty <> '' GROUP BY ste  HAVING l > 5 ORDER BY l DESC");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(ste,int16
l,double
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(6,10,50000
1,8.333333,150000
)");
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

    std::string ans_scheme = "SUM(what),int64\n";
    for (ui64 i = 1; i < its; i++) {
        ans_scheme += "SUM(-(what " + std::to_string(i) + ")),int64\n";
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
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh WHERE was <> '' GROUP BY was, getaway ORDER BY 'AVG(once)' DESC LIMIT 10");
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
SUM(what),int64
AVG(once),double
)");
    EXPECT_EQ(out_data->str(), R"(john,2,50000,250000,8
klinghoffer,2,50000,350000,8
josh,1,100000,200000,4.5
frusciante,2,100000,600000,4
john,1,50000,150000,4
klinghoffer,4,50000,50000,4
)");
}

TEST_F(BenchTest, _32) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh WHERE empty <> '' GROUP BY was, getaway ORDER BY 'AVG(once)' DESC LIMIT 10");
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
SUM(what),int64
AVG(once),double
)");
    EXPECT_EQ(out_data->str(), R"(josh,1,50000,150000,7
frusciante,2,100000,600000,4
john,1,50000,150000,4
klinghoffer,4,50000,50000,4
)");
}

TEST_F(BenchTest, _33) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, getaway, COUNT(*) AS c, SUM(what), AVG(once) FROM josh GROUP BY was, getaway ORDER BY 'AVG(once)' DESC LIMIT 10");
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
SUM(what),int64
AVG(once),double
)");
    EXPECT_EQ(out_data->str(), R"(john,2,50000,250000,8
klinghoffer,2,50000,350000,8
josh,1,100000,200000,4.5
frusciante,2,100000,600000,4
john,1,50000,150000,4
klinghoffer,4,50000,50000,4
)");
}

TEST_F(BenchTest, _34) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT empty, COUNT(*) AS c FROM josh GROUP BY was ORDER BY c DESC LIMIT 10");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    // std::cout << out_scheme->str() << std::endl;
    // std::cout << out_data->str() << std::endl;

    EXPECT_EQ(out_scheme->str(), R"(empty,string
c,int64
)");
    EXPECT_EQ(out_data->str(), R"(dolores,100000
dolores,100000
,100000
,100000
)");
}

TEST_F(BenchTest, _37) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS cnt FROM josh WHERE ste = 1 AND low <= '2022-12-31' AND low >= '2022-01-01' AND empty <> '' GROUP BY was ORDER BY cnt DESC LIMIT 10");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,50000
josh,50000
klinghoffer,50000
)");
}

TEST_F(BenchTest, _38) {
    
    JfEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*) AS cnt FROM josh WHERE ste = 1 AND low <= '2022-12-31' AND low >= '2022-01-01' AND empty <> '' GROUP BY was ORDER BY cnt DESC LIMIT 10");
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
    EXPECT_EQ(out_data->str(), R"(frusciante,50000
josh,50000
klinghoffer,50000
)");
}

} // namespace JfEngine::Testing
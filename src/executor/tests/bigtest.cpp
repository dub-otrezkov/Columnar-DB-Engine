#include "tests.h"
#include "workers/base.h"

namespace JfEngine::Testing {

struct BigTest : testing::Test {

    std::string scheme = R"(what,int64
once,int32
was,string
hers,string
low,date
beam,timestamp
)";
    std::string data = R"(1,2,josh,rip,"2022-02-24","2022-02-24 00:00:00"
3,4,john,rip,"2022-02-24",2022-02-24 00:00:03
5,6,frusciante,rip,2024-02-24,"2022-02-24 00:00:04"
7,8,klinghoffer,rip,2025-02-24,"2022-02-24 00:00:00"
)";

    static constexpr ui64 iter = kRowGroupLen * 10;

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

TEST_F(BigTest, SimpleColumnGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT what FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(what,int64
)");
    EXPECT_EQ(out_data->str().size(), iter * 8);
}

TEST_F(BigTest, DatesFilter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) AS d FROM josh WHERE low < '2023-02-28'");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(d,int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(iter * 2) + "\n");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, SumGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT SUM(what) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int128
)");
    EXPECT_EQ(out_data->str(), std::to_string(16 * iter) + "\n");
}

TEST_F(BigTest, MinMaxGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT MIN(what), MAX(what), MAX(was), MIN(was) FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(MIN(what),int64
MAX(what),int64
MAX(was),string
MIN(was),string
)");
    EXPECT_EQ(out_data->str(), "1,7,klinghoffer,frusciante\n");
}

TEST_F(BigTest, DistinctGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT DISTINCT(was) AS c FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(c,string
)");
    EXPECT_EQ(out_data->str(), R"(josh
john
frusciante
klinghoffer
)");
}

TEST_F(BigTest, CountDistinctGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT COUNT(DISTINCT(was)) AS c FROM josh");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(c,int64
)");
    EXPECT_EQ(out_data->str(), R"(4
)");
}

TEST_F(BigTest, LikeGetter) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT SUM(what) FROM josh WHERE was LIKE 'j%'");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int128
)");
    EXPECT_EQ(out_data->str(), std::to_string(4 * iter) + "\n");
}

TEST_F(BigTest, In) {
    
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT COUNT(*) FROM josh WHERE was NOT IN ('john') ");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(3 * iter) + "\n");
}

TEST_F(BigTest, GroupBySimple) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT hers, COUNT(*), SUM(what) FROM josh GROUP BY hers");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(hers,string
COUNT(*),int64
SUM(what),int128
)");
    EXPECT_EQ(out_data->str(), R"(rip,400000,1600000
)");
}

TEST_F(BigTest, GroupByWithWhere) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*), SUM(what) FROM josh WHERE was IN ('josh','john') GROUP BY was");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
COUNT(*),int64
SUM(what),int128
)");
    EXPECT_EQ(out_data->str(), R"(josh,100000,100000
john,100000,300000
)");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, GroupBySeveral) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT was, hers, COUNT(*) AS cnt FROM josh GROUP BY hers, was");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
hers,string
cnt,int64
)");
    EXPECT_EQ(out_data->str(), R"(john,rip,100000
frusciante,rip,100000
josh,rip,100000
klinghoffer,rip,100000
)");
}

TEST_F(BigTest, OrderBy) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT what, once, was FROM josh ORDER BY was LIMIT 3");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(what,int64
once,int32
was,string
)");
    EXPECT_EQ(out_data->str(), R"(5,6,frusciante
5,6,frusciante
5,6,frusciante
)");
}

TEST_F(BigTest, OrderByStable) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT what, once, was FROM josh ORDER BY hers LIMIT 5");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(what,int64
once,int32
was,string
)");
    EXPECT_EQ(out_data->str(), R"(1,2,josh
3,4,john
5,6,frusciante
7,8,klinghoffer
1,2,josh
)");
}

TEST_F(BigTest, WhereGroupOrder) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*), SUM(what) AS sum FROM josh WHERE was IN ('josh','john') GROUP BY was ORDER BY sum");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
COUNT(*),int64
sum,int128
)");
    EXPECT_EQ(out_data->str(), R"(josh,100000,100000
john,100000,300000
)");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, GroupByTimestamp) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT beam, COUNT(*) FROM josh GROUP BY beam ORDER BY beam DESC");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(beam,timestamp
COUNT(*),int64
)");
    EXPECT_EQ(out_data->str(), R"(2022-02-24 00:00:04,100000
2022-02-24 00:00:03,100000
2022-02-24 00:00:00,200000
)");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, GroupOrderWhere) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*), SUM(what) AS sum FROM josh GROUP BY was HAVING sum <> 300000 ORDER BY sum");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
COUNT(*),int64
sum,int128
)");
    EXPECT_EQ(out_data->str(), R"(josh,100000,100000
frusciante,100000,500000
klinghoffer,100000,700000
)");
}

TEST_F(BigTest, GroupOrderLimit) {
    JfEngine::TExecutor exec;
    {
        auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }
    {
        auto err = exec.ExecQuery("SELECT was, COUNT(*), SUM(what) AS sum FROM josh GROUP BY was ORDER BY sum DESC LIMIT 2");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    EXPECT_EQ(out_scheme->str(), R"(was,string
COUNT(*),int64
sum,int128
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,100000,700000
frusciante,100000,500000
)");
}

} // namespace JfEngine::Testing
#include "tests.h"

#include "workers/base.h"

namespace JFEngine::Testing {

struct BigTest : testing::Test {

    std::string scheme = R"(what,int64
once,int32
was,string
hers,string
)";
    std::string data = R"(1,2,josh,rip
3,4,john,rip
5,6,frusciante,rip
7,8,klinghoffer,rip
)";

    static constexpr ui64 iter = kRowGroupLen * 10;

    std::shared_ptr<std::stringstream> out_scheme;
    std::shared_ptr<std::stringstream> out_data;

    void SetUp() override {
        TIOFactory::RegisterSStreamIO("scheme", ETypeFile::kCSVFile);
        TIOFactory::RegisterSStreamIO("data", ETypeFile::kCSVFile);
        TIOFactory::RegisterSStreamIO("josh", ETypeFile::kJFFile);
        TIOFactory::RegisterSStreamIO("tmp1", ETypeFile::kJFFile);
        TIOFactory::RegisterSStreamIO("tmp2", ETypeFile::kJFFile);

        TIOFactory::GetIO("scheme").GetRes() << scheme;
        for (ui64 i = 0; i < iter; i++) {
            TIOFactory::GetIO("data").GetRes() << data;
        }

        TIOFactory::RegisterSStreamIO(kResultScheme, ETypeFile::kCSVFile);
        TIOFactory::RegisterSStreamIO(kResultData, ETypeFile::kCSVFile);

        out_scheme = std::dynamic_pointer_cast<std::stringstream>(
            TIOFactory::GetIO(kResultScheme).GetShared()
        );
        out_data = std::dynamic_pointer_cast<std::stringstream>(
            TIOFactory::GetIO(kResultData).GetShared()
        );
    }

    void TearDown() override {
        TIOFactory::UnregisterIO("scheme");
        TIOFactory::UnregisterIO("data");
        TIOFactory::UnregisterIO("josh");
        TIOFactory::UnregisterIO("tmp1");
        TIOFactory::UnregisterIO("tmp2");

        TIOFactory::UnregisterIO(kResultData);
        TIOFactory::UnregisterIO(kResultScheme);
    }
};

TEST_F(BigTest, SimpleColumnGetter) {
    
    JFEngine::TExecutor exec;
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

TEST_F(BigTest, SumGetter) {
    
    JFEngine::TExecutor exec;
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

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(16 * iter) + "\n");
}

TEST_F(BigTest, MinMaxGetter) {
    
    JFEngine::TExecutor exec;
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

TEST_F(BigTest, LikeGetter) {
    
    JFEngine::TExecutor exec;
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

    EXPECT_EQ(out_scheme->str(), R"(SUM(what),int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(4 * iter) + "\n");
}

TEST_F(BigTest, In) {
    
    JFEngine::TExecutor exec;
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
    JFEngine::TExecutor exec;
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
SUM(what),int64
)");
    EXPECT_EQ(out_data->str(), R"(rip,400000,1600000
)");
}

TEST_F(BigTest, GroupByWithWhere) {
    JFEngine::TExecutor exec;
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
SUM(what),int64
)");
    EXPECT_EQ(out_data->str(), R"(john,100000,300000
josh,100000,100000
)");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, GroupBySeveral) {
    JFEngine::TExecutor exec;
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
    EXPECT_EQ(out_data->str(), R"(frusciante,rip,100000
john,rip,100000
josh,rip,100000
klinghoffer,rip,100000
)");
}

TEST_F(BigTest, OrderBy) {
    JFEngine::TExecutor exec;
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
    JFEngine::TExecutor exec;
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
    JFEngine::TExecutor exec;
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
sum,int64
)");
    EXPECT_EQ(out_data->str(), R"(josh,100000,100000
john,100000,300000
)");
    // std::cout << out_data->str() << std::endl;
}

TEST_F(BigTest, GroupOrderWhere) {
    JFEngine::TExecutor exec;
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
sum,int64
)");
    EXPECT_EQ(out_data->str(), R"(josh,100000,100000
frusciante,100000,500000
klinghoffer,100000,700000
)");
}

TEST_F(BigTest, GroupOrderLimit) {
    JFEngine::TExecutor exec;
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
sum,int64
)");
    EXPECT_EQ(out_data->str(), R"(klinghoffer,100000,700000
frusciante,100000,500000
)");
}

} // namespace JFEngine::Testing
#include "tests.h"

#include "workers/base.h"

namespace JFEngine::Testing {

struct BigTest : testing::Test {

    std::string scheme = R"(what,int64
once,int32
was,string
)";
    std::string data = R"(1,2,josh
3,4,john
5,6,frusciante
7,8,klinghoffer
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(16 * iter) + "\n");
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
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

    EXPECT_EQ(out_scheme->str(), R"(column 0,int64
)");
    EXPECT_EQ(out_data->str(), std::to_string(3 * iter) + "\n");
}Ц

} // namespace JFEngine::Testing
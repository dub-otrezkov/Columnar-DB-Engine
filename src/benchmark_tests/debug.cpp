#include "workers/base.h"
#include "executor/executor.h"

#include "ios_factory/ios_factory.h"

#include <gtest/gtest.h>

#include <string_view>
#include <memory>

namespace JFEngine::Testing {

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
dorothy,int64
)"; // TODO add dates
    std::string data = R"(1,2,josh,rip,4,laading,1,"",2023-05-15,2023-05-15 14:30:22,1
3,4,john,rip,5,she needs him,1,dolores,2022-11-08,2021-11-08 09:45:17,2
5,6,frusciante,forever,6,harvey,2,dolores,2022-03-27,2022-03-27 23:18:43,3
7,8,klinghoffer,alive,6,"low,beam",2,"",2020-09-12,2020-09-12 07:52:09,4
5,8,john,alive,8,dorothy,2,"",2024-01-30,2024-01-30 16:40:55,5
1,4,klinghoffer,forever,1,if you know what's right,4,dolores2,2022-07-19,2019-07-19 11:33:41,6
7,2,frusciante,forever,1,speed racer,2,dolores1,2022-12-03,2023-12-03 20:15:28,7
3,7,josh,forever,1,cool with u,1,dolores1,2022-06-21,2022-06-21 05:08:14,8
)";

    static constexpr ui64 iter = 1;

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


void prolog(JFEngine::TExecutor& exec) {
    auto err = exec.ExecQuery("CREATE josh FROM scheme, data");
    if (err.HasError()) {
        std::cout << err.GetError() << std::endl;
    }
    ASSERT_FALSE(err.HasError());
}

TEST_F(BenchTest, _35) {
    
    JFEngine::TExecutor exec;
    prolog(exec);
    {
        auto err = exec.ExecQuery("SELECT what, +(what, 1), +(what, 2), -(what, 3), COUNT(*) AS c FROM josh GROUP BY what ORDER BY c DESC LIMIT 10;");
        if (err.HasError()) {
            std::cout << err.GetError() << std::endl;
        }
        ASSERT_FALSE(err.HasError());
    }

    std::cout << out_scheme->str() << std::endl;
    std::cout << out_data->str() << std::endl;

//     EXPECT_EQ(out_scheme->str(), R"(empty,string
// c,int64
// )");
//     EXPECT_EQ(out_data->str(), R"(dolores,100000
// dolores,100000
// ,100000
// ,100000
// )");
}

} // namespace JFEngine::Testing

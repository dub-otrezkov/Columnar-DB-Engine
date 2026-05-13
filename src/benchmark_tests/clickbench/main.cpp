#include "executor/executor.h"
#include "ios_factory/ios_factory.h"
#include "utils/perf_stats/perf_stats.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// (cd ../../build/benchmark_tests/clickbench; make clickbench)
// >hits.jf; >tmp1.jf; >tmp2.jf; >RESULT_DATA.csv; >RESULT_SCHEME.csv; ../../build/benchmark_tests/clickbench/clickbench
// >hits.jf; >RESULT_DATA.csv; >RESULT_SCHEME.csv; ../../build/benchmark_tests/clickbench/clickbench
// >RESULT_DATA.csv; >RESULT_SCHEME.csv; ../../build/executor/debug/debug_exec

// >RESULT_DATA.csv; >RESULT_SCHEME.csv; ../../build/benchmark_tests/clickbench/clickbench

/*
sync    
echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
*/

// AI GENERATED CODE WARNING

int main() {
    JfEngine::TQueryStats stats;
    JfEngine::TQueryStats::instance = &stats;

    JfEngine::TExecutor exec;

    // Список всех запросов (включая CREATE и помеченные как "ok")
    std::vector<std::string> queries = {
        // "CREATE hits FROM scheme, dorothy",
        "SELECT COUNT(*) FROM hits",
        "SELECT COUNT(*) FROM hits WHERE AdvEngineID <> 0",
        "SELECT SUM(AdvEngineID), COUNT(AdvEngineID), AVG(ResolutionWidth) FROM hits",
        "SELECT AVG(UserID) FROM hits",
        "SELECT COUNT(DISTINCT(UserID)) FROM hits",
        "SELECT COUNT(DISTINCT(SearchPhrase)) FROM hits",
        "SELECT MIN(EventDate), MAX(EventDate) FROM hits",
        "SELECT AdvEngineID, COUNT(*) FROM hits WHERE AdvEngineID <> 0 GROUP BY AdvEngineID ORDER BY 'COUNT(*)' DESC",
        "SELECT RegionID, COUNT(DISTINCT(UserID)) AS u FROM hits GROUP BY RegionID ORDER BY u DESC LIMIT 10",
        "SELECT RegionID, SUM(AdvEngineID), COUNT(AdvEngineID) AS c, AVG(ResolutionWidth), COUNT(DISTINCT(UserID)) FROM hits GROUP BY RegionID ORDER BY c DESC LIMIT 10",
        "SELECT MobilePhoneModel, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhoneModel ORDER BY u DESC LIMIT 10",
        "SELECT MobilePhone, MobilePhoneModel, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhone, MobilePhoneModel ORDER BY u DESC LIMIT 10",
        "SELECT SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
        "SELECT SearchPhrase, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY u DESC LIMIT 10",
        "SELECT SearchEngineID, SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, SearchPhrase ORDER BY c DESC LIMIT 10",
        "SELECT UserID, COUNT(*) FROM hits GROUP BY UserID ORDER BY 'COUNT(*)' DESC LIMIT 10",
        "SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase ORDER BY 'COUNT(*)' DESC LIMIT 10",
        "SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase LIMIT 10",
        "SELECT UserID, EXTRACT_MINUTE(EventTime) AS m, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, m, SearchPhrase ORDER BY 'COUNT(*)' DESC LIMIT 10",
        "SELECT UserID FROM hits WHERE UserID = 435090932899640449",
        "SELECT WatchID, JavaEnable, Title, GoodEvent, EventTime, EventDate, CounterID, ClientIP, RegionID, UserID, CounterClass, OS, UserAgent, URL, Referer, IsRefresh, RefererCategoryID, RefererRegionID, URLCategoryID, URLRegionID, ResolutionWidth, ResolutionHeight, ResolutionDepth, FlashMajor, FlashMinor, FlashMinor2, NetMajor, NetMinor, UserAgentMajor, UserAgentMinor, CookieEnable, JavascriptEnable, IsMobile, MobilePhone, MobilePhoneModel, Params, IPNetworkID, TraficSourceID, SearchEngineID, SearchPhrase, AdvEngineID, IsArtifical, WindowClientWidth, WindowClientHeight, ClientTimeZone, ClientEventTime, SilverlightVersion1, SilverlightVersion2, SilverlightVersion3, SilverlightVersion4, PageCharset, CodeVersion, IsLink, IsDownload, IsNotBounce, FUniqID, OriginalURL, HID, IsOldCounter, IsEvent, IsParameter, DontCountHits, WithHash, HitColor, LocalEventTime, Age, Sex, Income, Interests, Robotness, RemoteIP, WindowName, OpenerName, HistoryLength, BrowserLanguage, BrowserCountry, SocialNetwork, SocialAction, HTTPError, SendTiming, DNSTiming, ConnectTiming, ResponseStartTiming, ResponseEndTiming, FetchTiming, SocialSourceNetworkID, SocialSourcePage, ParamPrice, ParamOrderID, ParamCurrency, ParamCurrencyID, OpenstatServiceName, OpenstatCampaignID, OpenstatAdID, OpenstatSourceID, UTMSource, UTMMedium, UTMCampaign, UTMContent, UTMTerm, FromTag, HasGCLID, RefererHash, URLHash, CLID FROM hits ORDER BY EventTime LIMIT 10",
        "SELECT COUNT(URL) FROM hits WHERE URL LIKE '%google%'",
        "SELECT SearchPhrase, MIN(URL), COUNT(URL) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
        "SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(URL) AS c, COUNT(DISTINCT(UserID)) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
        "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime LIMIT 10",
        "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY SearchPhrase LIMIT 10",
        "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime, SearchPhrase LIMIT 10",
        "SELECT CounterID, AVG(LENGTH(URL)) AS l, COUNT(URL) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING c > 100000 ORDER BY l DESC LIMIT 25",
        R"(SELECT REGEXP_REPLACE(Referer, '^https?://(?:www\.)?([^/]+)/.*$', '\1') AS k, AVG(LENGTH(Referer)) AS l, COUNT(Referer) AS c, MIN(Referer) FROM hits WHERE Referer <> '' GROUP BY k HAVING c > 100000 ORDER BY l DESC LIMIT 25)",
        "SELECT SUM(ResolutionWidth), SUM(+(ResolutionWidth, 1)), SUM(+(ResolutionWidth, 2)), SUM(+(ResolutionWidth, 3)), SUM(+(ResolutionWidth, 4)), SUM(+(ResolutionWidth, 5)), SUM(+(ResolutionWidth, 6)), SUM(+(ResolutionWidth, 7)), SUM(+(ResolutionWidth, 8)), SUM(+(ResolutionWidth, 9)), SUM(+(ResolutionWidth, 10)), SUM(+(ResolutionWidth, 11)), SUM(+(ResolutionWidth, 12)), SUM(+(ResolutionWidth, 13)), SUM(+(ResolutionWidth, 14)), SUM(+(ResolutionWidth, 15)), SUM(+(ResolutionWidth, 16)), SUM(+(ResolutionWidth, 17)), SUM(+(ResolutionWidth, 18)), SUM(+(ResolutionWidth, 19)), SUM(+(ResolutionWidth, 20)), SUM(+(ResolutionWidth, 21)), SUM(+(ResolutionWidth, 22)), SUM(+(ResolutionWidth, 23)), SUM(+(ResolutionWidth, 24)), SUM(+(ResolutionWidth, 25)), SUM(+(ResolutionWidth, 26)), SUM(+(ResolutionWidth, 27)), SUM(+(ResolutionWidth, 28)), SUM(+(ResolutionWidth, 29)), SUM(+(ResolutionWidth, 30)), SUM(+(ResolutionWidth, 31)), SUM(+(ResolutionWidth, 32)), SUM(+(ResolutionWidth, 33)), SUM(+(ResolutionWidth, 34)), SUM(+(ResolutionWidth, 35)), SUM(+(ResolutionWidth, 36)), SUM(+(ResolutionWidth, 37)), SUM(+(ResolutionWidth, 38)), SUM(+(ResolutionWidth, 39)), SUM(+(ResolutionWidth, 40)), SUM(+(ResolutionWidth, 41)), SUM(+(ResolutionWidth, 42)), SUM(+(ResolutionWidth, 43)), SUM(+(ResolutionWidth, 44)), SUM(+(ResolutionWidth, 45)), SUM(+(ResolutionWidth, 46)), SUM(+(ResolutionWidth, 47)), SUM(+(ResolutionWidth, 48)), SUM(+(ResolutionWidth, 49)), SUM(+(ResolutionWidth, 50)), SUM(+(ResolutionWidth, 51)), SUM(+(ResolutionWidth, 52)), SUM(+(ResolutionWidth, 53)), SUM(+(ResolutionWidth, 54)), SUM(+(ResolutionWidth, 55)), SUM(+(ResolutionWidth, 56)), SUM(+(ResolutionWidth, 57)), SUM(+(ResolutionWidth, 58)), SUM(+(ResolutionWidth, 59)), SUM(+(ResolutionWidth, 60)), SUM(+(ResolutionWidth, 61)), SUM(+(ResolutionWidth, 62)), SUM(+(ResolutionWidth, 63)), SUM(+(ResolutionWidth, 64)), SUM(+(ResolutionWidth, 65)), SUM(+(ResolutionWidth, 66)), SUM(+(ResolutionWidth, 67)), SUM(+(ResolutionWidth, 68)), SUM(+(ResolutionWidth, 69)), SUM(+(ResolutionWidth, 70)), SUM(+(ResolutionWidth, 71)), SUM(+(ResolutionWidth, 72)), SUM(+(ResolutionWidth, 73)), SUM(+(ResolutionWidth, 74)), SUM(+(ResolutionWidth, 75)), SUM(+(ResolutionWidth, 76)), SUM(+(ResolutionWidth, 77)), SUM(+(ResolutionWidth, 78)), SUM(+(ResolutionWidth, 79)), SUM(+(ResolutionWidth, 80)), SUM(+(ResolutionWidth, 81)), SUM(+(ResolutionWidth, 82)), SUM(+(ResolutionWidth, 83)), SUM(+(ResolutionWidth, 84)), SUM(+(ResolutionWidth, 85)), SUM(+(ResolutionWidth, 86)), SUM(+(ResolutionWidth, 87)), SUM(+(ResolutionWidth, 88)), SUM(+(ResolutionWidth, 89)) FROM hits",
        "SELECT SearchEngineID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, ClientIP ORDER BY c DESC LIMIT 10",
        "SELECT WatchID, ClientIP, COUNT(IsRefresh) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
        "SELECT WatchID, ClientIP, COUNT(IsRefresh) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
        "SELECT URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10",
        "SELECT CONST_INT(1), URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10",
        "SELECT ClientIP, -(ClientIP, 1), -(ClientIP, 2), -(ClientIP, 3), COUNT(*) AS c FROM hits GROUP BY ClientIP, -(ClientIP, 1), -(ClientIP, 2), -(ClientIP, 3) ORDER BY c DESC LIMIT 10",
        "SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT 10",
        "SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC LIMIT 10",
        "SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND IsLink <> 0 AND IsDownload = 0 GROUP BY URL ORDER BY PageViews DESC LIMIT 10 OFFSET 1000",
        "SELECT TraficSourceID, SearchEngineID, AdvEngineID, IF (SearchEngineID = 0 AND AdvEngineID = 0 THEN Referer ELSE '') AS Src, URL AS Dst, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 GROUP BY TraficSourceID, SearchEngineID, AdvEngineID, Src, Dst ORDER BY PageViews DESC LIMIT 10 OFFSET 1000",
        "SELECT URLHash, EventDate, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND TraficSourceID IN (-1, 6) AND RefererHash = 3594120000172545465 GROUP BY URLHash, EventDate ORDER BY PageViews DESC LIMIT 10 OFFSET 100",
        "SELECT WindowClientWidth, WindowClientHeight, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND DontCountHits = 0 AND URLHash = 2868770270353813622 GROUP BY WindowClientWidth, WindowClientHeight ORDER BY PageViews DESC LIMIT 10 OFFSET 10000",
        "SELECT TRUNC_MINUTE(EventTime) AS M, COUNT(EventTime) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-14' AND EventDate <= '2013-07-15' AND IsRefresh = 0 AND DontCountHits = 0 GROUP BY M ORDER BY M LIMIT 10 OFFSET 1000"
    };

    double total_time = 0;
    double total_time_no_create = 0;

    std::cout << std::left << std::setw(15) << "Time (ms)" << "| Query" << std::endl;
    std::cout << std::string(100, '-') << std::endl;

    for (const auto& q : queries) {
        stats.Reset();

        // Замер времени начала
        auto start = std::chrono::high_resolution_clock::now();

        auto err = exec.ExecQuery(q);

        // Замер времени окончания
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        double current_ms = elapsed.count();
        total_time += current_ms;

        // Проверяем, не является ли запрос операцией CREATE
        bool is_create = (q.find("CREATE") != std::string::npos);
        if (!is_create) {
            total_time_no_create += current_ms;
        }

        // Вывод результата
        std::cout << std::left << std::setw(15) << std::fixed << std::setprecision(3) << current_ms << "| " << q << std::endl;
        stats.Print(std::cout);

        if (err.HasError()) {
            std::cerr << "   ^-- ERROR: " << err.GetError() << std::endl;
            // Если CREATE упал, дальнейшие SELECT к таблице могут не иметь смысла
            if (is_create) {
                std::cerr << "Stopping execution due to CREATE failure." << std::endl;
                break;
            }
        }
    }

    // Итоговое саммари
    std::cout << std::string(100, '=') << std::endl;
    std::cout << "SUMMARY:" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Total execution time:          " << total_time << " ms" << std::endl;
    std::cout << "Total time (excluding CREATE): " << total_time_no_create << " ms" << std::endl;

    return 0;
}
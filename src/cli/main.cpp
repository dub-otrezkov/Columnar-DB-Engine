#include "executor/executor.h"
#include "ios_factory/ios_factory.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std::chrono;

using namespace JfEngine;

// NEURO SLOP

// Queries 0-42 matching done.txt / ClickBench numbering.
// Empty string = not supported by this engine.
static const std::vector<std::string> kQueries = {
    // 0: ok
    "SELECT COUNT(*) FROM hits",
    // 1: ok
    "SELECT COUNT(*) FROM hits WHERE AdvEngineID <> 0",
    // 2: ok
    "SELECT SUM(AdvEngineID), COUNT(AdvEngineID), AVG(ResolutionWidth) FROM hits",
    // 3: ok
    "SELECT AVG(UserID) FROM hits",
    // 4: ok
    "SELECT COUNT(DISTINCT(UserID)) FROM hits",
    // 5: ok
    "SELECT COUNT(DISTINCT(SearchPhrase)) FROM hits",
    // 6: ok
    "SELECT MIN(EventDate), MAX(EventDate) FROM hits",
    // 7: ok
    "SELECT AdvEngineID, COUNT(*) FROM hits WHERE AdvEngineID <> 0 GROUP BY AdvEngineID ORDER BY 'COUNT(*)' DESC",
    // 8: ok
    "SELECT RegionID, COUNT(DISTINCT(UserID)) AS u FROM hits GROUP BY RegionID ORDER BY u DESC LIMIT 10",
    // 9: ok
    "SELECT RegionID, SUM(AdvEngineID), COUNT(AdvEngineID) AS c, AVG(ResolutionWidth), COUNT(DISTINCT(UserID)) FROM hits GROUP BY RegionID ORDER BY c DESC LIMIT 10",
    // 10: ok
    "SELECT MobilePhoneModel, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhoneModel ORDER BY u DESC LIMIT 10",
    // 11: ok
    "SELECT MobilePhone, MobilePhoneModel, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhone, MobilePhoneModel ORDER BY u DESC LIMIT 10",
    // 12: ok
    "SELECT SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
    // 13: ok
    "SELECT SearchPhrase, COUNT(DISTINCT(UserID)) AS u FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY u DESC LIMIT 10",
    // 14: ok
    "SELECT SearchEngineID, SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, SearchPhrase ORDER BY c DESC LIMIT 10",
    // 15: ok
    "SELECT UserID, COUNT(*) FROM hits GROUP BY UserID ORDER BY 'COUNT(*)' DESC LIMIT 10",
    // 16: ok
    "SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase ORDER BY 'COUNT(*)' DESC LIMIT 10",
    // 17: ok
    "SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase LIMIT 10",
    // 18: ok
    "SELECT UserID, EXTRACT_MINUTE(EventTime) AS m, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, m, SearchPhrase ORDER BY 'COUNT(*)' DESC LIMIT 10",
    // 19: ok
    "SELECT UserID FROM hits WHERE UserID = 435090932899640449",
    // 20: ok
    "SELECT COUNT(URL) FROM hits WHERE URL LIKE '%google%'",
    // 21: ok
    "SELECT SearchPhrase, MIN(URL), COUNT(URL) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
    // 22: ok
    "SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(URL) AS c, COUNT(DISTINCT(UserID)) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
    // 23: ok (SELECT * replaced with explicit column list)
    "SELECT WatchID, JavaEnable, Title, GoodEvent, EventTime, EventDate, CounterID, ClientIP, RegionID, UserID, CounterClass, OS, UserAgent, URL, Referer, IsRefresh, RefererCategoryID, RefererRegionID, URLCategoryID, URLRegionID, ResolutionWidth, ResolutionHeight, ResolutionDepth, FlashMajor, FlashMinor, FlashMinor2, NetMajor, NetMinor, UserAgentMajor, UserAgentMinor, CookieEnable, JavascriptEnable, IsMobile, MobilePhone, MobilePhoneModel, Params, IPNetworkID, TraficSourceID, SearchEngineID, SearchPhrase, AdvEngineID, IsArtifical, WindowClientWidth, WindowClientHeight, ClientTimeZone, ClientEventTime, SilverlightVersion1, SilverlightVersion2, SilverlightVersion3, SilverlightVersion4, PageCharset, CodeVersion, IsLink, IsDownload, IsNotBounce, FUniqID, OriginalURL, HID, IsOldCounter, IsEvent, IsParameter, DontCountHits, WithHash, HitColor, LocalEventTime, Age, Sex, Income, Interests, Robotness, RemoteIP, WindowName, OpenerName, HistoryLength, BrowserLanguage, BrowserCountry, SocialNetwork, SocialAction, HTTPError, SendTiming, DNSTiming, ConnectTiming, ResponseStartTiming, ResponseEndTiming, FetchTiming, SocialSourceNetworkID, SocialSourcePage, ParamPrice, ParamOrderID, ParamCurrency, ParamCurrencyID, OpenstatServiceName, OpenstatCampaignID, OpenstatAdID, OpenstatSourceID, UTMSource, UTMMedium, UTMCampaign, UTMContent, UTMTerm, FromTag, HasGCLID, RefererHash, URLHash, CLID FROM hits ORDER BY EventTime LIMIT 10",
    // 24: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime LIMIT 10",
    // 25: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY SearchPhrase LIMIT 10",
    // 26: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime, SearchPhrase LIMIT 10",
    // 27: ok
    "SELECT CounterID, AVG(LENGTH(URL)) AS l, COUNT(URL) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING c > 100000 ORDER BY l DESC LIMIT 25",
    // 28: ok
    "SELECT REGEXP_REPLACE(Referer, '^https?://(?:www\\.)?([^/]+)/.*', '$1') AS k, AVG(LENGTH(Referer)) AS l, COUNT(*) AS c, MIN(Referer) FROM hits WHERE Referer <> '' GROUP BY k HAVING c > 100000 ORDER BY l DESC LIMIT 25",
    // 29: ok
    "SELECT SUM(ResolutionWidth), SUM(+(ResolutionWidth, 1)), SUM(+(ResolutionWidth, 2)), SUM(+(ResolutionWidth, 3)), SUM(+(ResolutionWidth, 4)), SUM(+(ResolutionWidth, 5)), SUM(+(ResolutionWidth, 6)), SUM(+(ResolutionWidth, 7)), SUM(+(ResolutionWidth, 8)), SUM(+(ResolutionWidth, 9)), SUM(+(ResolutionWidth, 10)), SUM(+(ResolutionWidth, 11)), SUM(+(ResolutionWidth, 12)), SUM(+(ResolutionWidth, 13)), SUM(+(ResolutionWidth, 14)), SUM(+(ResolutionWidth, 15)), SUM(+(ResolutionWidth, 16)), SUM(+(ResolutionWidth, 17)), SUM(+(ResolutionWidth, 18)), SUM(+(ResolutionWidth, 19)), SUM(+(ResolutionWidth, 20)), SUM(+(ResolutionWidth, 21)), SUM(+(ResolutionWidth, 22)), SUM(+(ResolutionWidth, 23)), SUM(+(ResolutionWidth, 24)), SUM(+(ResolutionWidth, 25)), SUM(+(ResolutionWidth, 26)), SUM(+(ResolutionWidth, 27)), SUM(+(ResolutionWidth, 28)), SUM(+(ResolutionWidth, 29)), SUM(+(ResolutionWidth, 30)), SUM(+(ResolutionWidth, 31)), SUM(+(ResolutionWidth, 32)), SUM(+(ResolutionWidth, 33)), SUM(+(ResolutionWidth, 34)), SUM(+(ResolutionWidth, 35)), SUM(+(ResolutionWidth, 36)), SUM(+(ResolutionWidth, 37)), SUM(+(ResolutionWidth, 38)), SUM(+(ResolutionWidth, 39)), SUM(+(ResolutionWidth, 40)), SUM(+(ResolutionWidth, 41)), SUM(+(ResolutionWidth, 42)), SUM(+(ResolutionWidth, 43)), SUM(+(ResolutionWidth, 44)), SUM(+(ResolutionWidth, 45)), SUM(+(ResolutionWidth, 46)), SUM(+(ResolutionWidth, 47)), SUM(+(ResolutionWidth, 48)), SUM(+(ResolutionWidth, 49)), SUM(+(ResolutionWidth, 50)), SUM(+(ResolutionWidth, 51)), SUM(+(ResolutionWidth, 52)), SUM(+(ResolutionWidth, 53)), SUM(+(ResolutionWidth, 54)), SUM(+(ResolutionWidth, 55)), SUM(+(ResolutionWidth, 56)), SUM(+(ResolutionWidth, 57)), SUM(+(ResolutionWidth, 58)), SUM(+(ResolutionWidth, 59)), SUM(+(ResolutionWidth, 60)), SUM(+(ResolutionWidth, 61)), SUM(+(ResolutionWidth, 62)), SUM(+(ResolutionWidth, 63)), SUM(+(ResolutionWidth, 64)), SUM(+(ResolutionWidth, 65)), SUM(+(ResolutionWidth, 66)), SUM(+(ResolutionWidth, 67)), SUM(+(ResolutionWidth, 68)), SUM(+(ResolutionWidth, 69)), SUM(+(ResolutionWidth, 70)), SUM(+(ResolutionWidth, 71)), SUM(+(ResolutionWidth, 72)), SUM(+(ResolutionWidth, 73)), SUM(+(ResolutionWidth, 74)), SUM(+(ResolutionWidth, 75)), SUM(+(ResolutionWidth, 76)), SUM(+(ResolutionWidth, 77)), SUM(+(ResolutionWidth, 78)), SUM(+(ResolutionWidth, 79)), SUM(+(ResolutionWidth, 80)), SUM(+(ResolutionWidth, 81)), SUM(+(ResolutionWidth, 82)), SUM(+(ResolutionWidth, 83)), SUM(+(ResolutionWidth, 84)), SUM(+(ResolutionWidth, 85)), SUM(+(ResolutionWidth, 86)), SUM(+(ResolutionWidth, 87)), SUM(+(ResolutionWidth, 88)), SUM(+(ResolutionWidth, 89)) FROM hits",
    // 30: ok
    "SELECT SearchEngineID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, ClientIP ORDER BY c DESC LIMIT 10",
    // 31: ok
    "SELECT WatchID, ClientIP, COUNT(IsRefresh) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
    // 32: ok
    "SELECT WatchID, ClientIP, COUNT(IsRefresh) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
    // 33: ok
    "SELECT URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10",
    // 34: ok
    "SELECT CONST_INT(1), URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10",
    // 35: ok
    "SELECT ClientIP, -(ClientIP, 1), -(ClientIP, 2), -(ClientIP, 3), COUNT(*) AS c FROM hits GROUP BY ClientIP, -(ClientIP, 1), -(ClientIP, 2), -(ClientIP, 3) ORDER BY c DESC LIMIT 10",
    // 36: ok
    "SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT 10",
    // 37: ok
    "SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC LIMIT 10",
    // 38: ok
    "SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND IsLink <> 0 AND IsDownload = 0 GROUP BY URL ORDER BY PageViews DESC LIMIT 10 OFFSET 1000",
    // 39: ok (IF instead of CASE WHEN)
    "SELECT TraficSourceID, SearchEngineID, AdvEngineID, IF (SearchEngineID = 0 AND AdvEngineID = 0 THEN Referer ELSE '') AS Src, URL AS Dst, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 GROUP BY TraficSourceID, SearchEngineID, AdvEngineID, Src, Dst ORDER BY PageViews DESC LIMIT 10 OFFSET 1000",
    // 40: ok
    "SELECT URLHash, EventDate, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND TraficSourceID IN (-1, 6) AND RefererHash = 3594120000172545465 GROUP BY URLHash, EventDate ORDER BY PageViews DESC LIMIT 10 OFFSET 100",
    // 41: ok
    "SELECT WindowClientWidth, WindowClientHeight, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND DontCountHits = 0 AND URLHash = 2868770270353813622 GROUP BY WindowClientWidth, WindowClientHeight ORDER BY PageViews DESC LIMIT 10 OFFSET 10000",
    // 42: ok
    "SELECT TRUNC_MINUTE(EventTime) AS M, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-14' AND EventDate <= '2013-07-15' AND IsRefresh = 0 AND DontCountHits = 0 GROUP BY M ORDER BY M LIMIT 10 OFFSET 1000",
};

// Open a file as a shared iostream. mode should include in|out.
static std::shared_ptr<std::fstream> open_file(const std::string& path, std::ios::openmode mode) {
    return std::make_shared<std::fstream>(path, mode);
}

static int do_convert(const std::string& input_csv,
                      const std::string& input_schema,
                      const std::string& output_jf) {
    // Input files opened directly - no temp dir needed.
    TIoFactory::RegisterCustomIo("scheme",  open_file(input_schema, std::ios::in | std::ios::out));
    TIoFactory::RegisterCustomIo("dorothy", open_file(input_csv,    std::ios::in | std::ios::out));
    TIoFactory::RegisterCustomIo("hits",    open_file(output_jf,    std::ios::in | std::ios::out | std::ios::trunc));
    // Intermediates as in-memory streams (convert only needs the CREATE step).
    TIoFactory::RegisterCustomIo("tmp1", std::make_shared<std::stringstream>());
    TIoFactory::RegisterCustomIo("tmp2", std::make_shared<std::stringstream>());

    JfEngine::TExecutor exec;
    auto t1  = high_resolution_clock::now();
    auto err = exec.ExecQuery("CREATE hits FROM scheme, dorothy");
    auto t2  = high_resolution_clock::now();

    std::cerr << "convert: "
              << duration<double, std::milli>(t2 - t1).count() << " ms\n";

    if (err.HasError()) {
        std::cerr << "error: " << err.GetError() << "\n";
        return 1;
    }
    return 0;
}

static int do_query(int query_num,
                    const std::string& input_jf,
                    const std::string& output_csv) {
    if (query_num < 0 || query_num >= static_cast<int>(kQueries.size())) {
        std::cerr << "query " << query_num << " out of range [0, 42]\n";
        return 1;
    }

    const std::string& query = kQueries[query_num];
    if (query.empty()) {
        std::cerr << "query " << query_num << " not supported\n";
        // Write empty output so the caller gets a file either way.
        std::ofstream(output_csv);
        return 0;
    }

    // Pre-register all aliases before the engine can touch them.
    TIoFactory::RegisterCustomIo("hits", open_file(input_jf, std::ios::in | std::ios::out));
    TIoFactory::RegisterCustomIo("tmp1", std::make_shared<std::stringstream>());
    TIoFactory::RegisterCustomIo("tmp2", std::make_shared<std::stringstream>());
    // RESULT_DATA goes straight to the output file.
    TIoFactory::RegisterCustomIo("RESULT_DATA",
        open_file(output_csv, std::ios::in | std::ios::out | std::ios::trunc));
    // RESULT_SCHEME is discarded (not needed in the benchmark output).
    TIoFactory::RegisterCustomIo("RESULT_SCHEME", std::make_shared<std::stringstream>());

    JfEngine::TExecutor exec;
    auto t1  = high_resolution_clock::now();
    auto err = exec.ExecQuery(query);
    auto t2  = high_resolution_clock::now();

    std::cerr << "query " << query_num << ": "
              << duration<double, std::milli>(t2 - t1).count() << " ms\n";

    if (err.HasError()) {
        std::cerr << "error: " << err.GetError() << "\n";
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " convert <input_csv> <input_schema> <output_jf>\n";
        std::cerr << "       " << argv[0] << " query <query_num> <input_jf> <output_csv>\n";
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "convert") {
        if (argc < 5) {
            std::cerr << "usage: " << argv[0] << " convert <input_csv> <input_schema> <output_jf>\n";
            return 1;
        }
        return do_convert(argv[2], argv[3], argv[4]);

    } else if (mode == "query") {
        if (argc < 5) {
            std::cerr << "usage: " << argv[0] << " query <query_num> <input_jf> <output_csv>\n";
            return 1;
        }
        return do_query(std::stoi(argv[2]), argv[3], argv[4]);

    } else {
        std::cerr << "unknown mode: " << mode << "\n";
        return 1;
    }
}

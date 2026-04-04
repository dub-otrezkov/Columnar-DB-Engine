#include "executor/executor.h"
#include "ios_factory/ios_factory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std::chrono;

using namespace JFEngine;

// NEURO SLOP

// Queries 0-42 matching done.txt / ClickBench numbering.
// Empty string = not supported by this engine.
static const std::vector<std::string> kQueries = {
    // 0: ok
    "SELECT COUNT(*) FROM hits",
    // 1: ok
    "SELECT COUNT(*) FROM hits WHERE AdvEngineID <> 0",
    // 2: ok
    "SELECT SUM(AdvEngineID), COUNT(*), AVG(ResolutionWidth) FROM hits",
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
    "SELECT RegionID, SUM(AdvEngineID), COUNT(*) AS c, AVG(ResolutionWidth), COUNT(DISTINCT(UserID)) FROM hits GROUP BY RegionID ORDER BY c DESC LIMIT 10",
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
    // 18: no dates (extract(minute FROM EventTime))
    "",
    // 19: ok
    "SELECT UserID FROM hits WHERE UserID = 435090932899640449",
    // 20: ok
    "SELECT COUNT(*) FROM hits WHERE URL LIKE '%google%'",
    // 21: ok
    "SELECT SearchPhrase, MIN(URL), COUNT(*) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
    // 22: ok
    "SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(*) AS c, COUNT(DISTINCT(UserID)) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10",
    // 23: ok (no *!) - SELECT * is not supported
    "",
    // 24: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime LIMIT 10",
    // 25: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY SearchPhrase LIMIT 10",
    // 26: ok
    "SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime, SearchPhrase LIMIT 10",
    // 27: ok
    "SELECT CounterID, AVG(LENGTH(URL)) AS l, COUNT(*) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING c > 100000 ORDER BY l DESC LIMIT 25",
    // 28: no regexp
    "",
    // 29: no columnar +
    "",
    // 30: ok
    "SELECT SearchEngineID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, ClientIP ORDER BY c DESC LIMIT 10",
    // 31: ok
    "SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
    // 32: ok
    "SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10",
    // 33: ok
    "SELECT URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10",
    // 34: no const (SELECT 1, URL ...)
    "",
    // 35: no columnar - (ClientIP - 1 ...)
    "",
    // 36: ok
    "SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT 10",
    // 37: ok
    "SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC LIMIT 10",
    // 38-42: no offset
    "", "", "", "", "",
};

// Open a file as a shared iostream. mode should include in|out.
static std::shared_ptr<std::fstream> open_file(const std::string& path, std::ios::openmode mode) {
    return std::make_shared<std::fstream>(path, mode);
}

static int do_convert(const std::string& input_csv,
                      const std::string& input_schema,
                      const std::string& output_jf) {
    // Input files opened directly - no temp dir needed.
    TIOFactory::RegisterCustomIO("scheme",  open_file(input_schema, std::ios::in | std::ios::out));
    TIOFactory::RegisterCustomIO("dorothy", open_file(input_csv,    std::ios::in | std::ios::out));
    TIOFactory::RegisterCustomIO("hits",    open_file(output_jf,    std::ios::in | std::ios::out | std::ios::trunc));
    // Intermediates as in-memory streams (convert only needs the CREATE step).
    TIOFactory::RegisterCustomIO("tmp1", std::make_shared<std::stringstream>());
    TIOFactory::RegisterCustomIO("tmp2", std::make_shared<std::stringstream>());

    JFEngine::TExecutor exec;
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
    TIOFactory::RegisterCustomIO("hits", open_file(input_jf, std::ios::in | std::ios::out));
    TIOFactory::RegisterCustomIO("tmp1", std::make_shared<std::stringstream>());
    TIOFactory::RegisterCustomIO("tmp2", std::make_shared<std::stringstream>());
    // RESULT_DATA goes straight to the output file.
    TIOFactory::RegisterCustomIO("RESULT_DATA",
        open_file(output_csv, std::ios::in | std::ios::out | std::ios::trunc));
    // RESULT_SCHEME is discarded (not needed in the benchmark output).
    TIOFactory::RegisterCustomIO("RESULT_SCHEME", std::make_shared<std::stringstream>());

    JFEngine::TExecutor exec;
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

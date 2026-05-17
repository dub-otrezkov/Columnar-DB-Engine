# Отчет
## Про ClickBench
Все запросы прогоняются на плоской таблице из `99997497` записей. Записи хранятся в формате .parquet, запросы к табличке выполняются через создание виртуальной таблицы-копии. Запросы выполняются 
однопоточно (и по тексту на гитхабе без кэширования, но у меня есть некоторые сомнения в этом, потому что кое-где на запросах с одинаковой сортировкой первый запрос долгий, а остальные очень быстрые). Данные в табличке - судя по запросам - это данные
пользователей какого-то сайта. Запросы - поиск пользователей по фильтрам или подсчет статистики 
(или стресс-тесты, например `SELECT SUM(ResolutionWidth), SUM(ResolutionWidth + 1), SUM(ResolutionWidth + 2), SUM(ResolutionWidth + 3), SUM(ResolutionWidth + 4), SUM(ResolutionWidth + 5), SUM(ResolutionWidth + 6), SUM(ResolutionWidth + 7), SUM(ResolutionWidth + 8)...`).
## Бенчмарки
### Запуск бенчмарков
Про систему, на которой запускались бенчмарки ниже. Установка [DuckDB](https://duckdb.org/install/?platform=linux&environment=cli), [ClickHouse](https://clickhouse.com/docs/getting-started/quick-start/oss). Для запуска бенчмарков в каждом варианте есть файл `benchmark.sh`. С запском возникли некоторые трудности, так как запускаю на достаточно слабой машине, но почти все запросы были завершены без ошибок (все ошибки были свзяаны с нехваткой памяти на устройстве).

### Результат
Результаты я сгруппировал по наличию ключевых слов в запросах.
Везде взял результат холодного запуска

| Query | ClickHouse (s) | DuckDB (s) | Result | Difference |
|-------|----------------|------------|--------|------------|
| `SELECT COUNT(*) FROM hits;` | 0.072 | 0.036 | ClickHouse better | **50.0%** |
| `SELECT SUM(AdvEngineID), COUNT(*), AVG(ResolutionWidth) FROM hits;` | 0.397 | 0.389 | ClickHouse better | **2.02%** |
| `SELECT AVG(UserID) FROM hits;` | 0.467 | 0.375 | ClickHouse better | **19.7%** |
| `SELECT COUNT(DISTINCT UserID) FROM hits;` | 3.394 | 3.683 | DuckDB better | **7.85%** |
| `SELECT COUNT(DISTINCT SearchPhrase) FROM hits;` | 3.578 | null | DuckDB better | **N/A** |
| `SELECT MIN(EventDate), MAX(EventDate) FROM hits;` | 0.293 | 0.157 | ClickHouse better | **46.42%** |
| `SELECT SUM(ResolutionWidth)... FROM hits;` | 0.369 | 0.311 | ClickHouse better | **15.72%** |
**WHERE**  
| `SELECT COUNT(*) FROM hits WHERE AdvEngineID <> 0;` | 0.206 | 0.146 | ClickHouse better | **29.13%** |
| `SELECT UserID FROM hits WHERE UserID = 435090932899640449;` | 0.216 | 0.368 | DuckDB better | **41.3%** |
| `SELECT COUNT(*) FROM hits WHERE URL LIKE '%google%';` | 8.020 | 4.897 | ClickHouse better | **38.94%** |
**GROUP WHERE ORDER**  
| `SELECT AdvEngineID, COUNT(*) FROM hits WHERE AdvEngineID <> 0 GROUP BY AdvEngineID ORDER BY COUNT(*) DESC;` | 0.185 | 0.155 | ClickHouse better | **16.22%** |
**GROUP ORDER LIMIT**  
| `SELECT RegionID, COUNT(DISTINCT UserID) AS u FROM hits GROUP BY RegionID ORDER BY u DESC LIMIT 10;` | 3.215 | 9.891 | DuckDB better | **67.5%** |
| `SELECT RegionID, SUM(AdvEngineID), COUNT(*) AS c, AVG(ResolutionWidth), COUNT(DISTINCT UserID) FROM hits GROUP BY RegionID ORDER BY c DESC LIMIT 10;` | 3.935 | 0.004 | ClickHouse better | **99.9%** |
| `SELECT UserID, COUNT(*) FROM hits GROUP BY UserID ORDER BY COUNT(*) DESC LIMIT 10;` | 3.088 | 0.005 | ClickHouse better | **99.84%** |
| `SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase ORDER BY COUNT(*) DESC LIMIT 10;` | null | null | Both failed | **N/A** |
| `SELECT UserID, extract(minute FROM EventTime) AS m, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, m, SearchPhrase ORDER BY COUNT(*) DESC LIMIT 10;` | null | null | Both failed | **N/A** |
| `SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10;` | null | 0.004 | ClickHouse better | **N/A** |
| `SELECT URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10;` | null | 0.005 | ClickHouse better | **N/A** |
| `SELECT 1, URL, COUNT(*) AS c FROM hits GROUP BY 1, URL ORDER BY c DESC LIMIT 10;` | null | 0.004 | ClickHouse better | **N/A** |
| `SELECT ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3, COUNT(*) AS c FROM hits GROUP BY ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3 ORDER BY c DESC LIMIT 10;` | 3.287 | 1.685 | ClickHouse better | **48.74%** |
**GROUP WHERE ORDER LIMIT**  
| `SELECT MobilePhoneModel, COUNT(DISTINCT UserID) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhoneModel ORDER BY u DESC LIMIT 10;` | 0.765 | 1.059 | DuckDB better | **27.76%** |
| `SELECT MobilePhone, MobilePhoneModel, COUNT(DISTINCT UserID) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhone, MobilePhoneModel ORDER BY u DESC LIMIT 10;` | 0.913 | 1.253 | DuckDB better | **27.13%** |
| `SELECT SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;` | 3.353 | null | DuckDB better | **N/A** |
| `SELECT SearchPhrase, COUNT(DISTINCT UserID) AS u FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY u DESC LIMIT 10;` | null | 10.750 | ClickHouse better | **N/A** |
| `SELECT SearchEngineID, SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, SearchPhrase ORDER BY c DESC LIMIT 10;` | null | 6.468 | ClickHouse better | **N/A** |
| `SELECT SearchPhrase, MIN(URL), COUNT(*) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;` | 7.297 | 10.884 | DuckDB better | **32.96%** |
| `SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(*) AS c, COUNT(DISTINCT UserID) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;` | 14.886 | 19.885 | DuckDB better | **25.14%** |
| `SELECT CounterID, AVG(length(URL)) AS l, COUNT(*) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING COUNT(*) > 100000 ORDER BY l DESC LIMIT 25;` | 6.925 | 9.162 | DuckDB better | **24.42%** |
| `SELECT REGEXP_REPLACE(Referer, '^https?://(?:www\.)?([^/]+)/.*$', '\1') AS k, AVG(length(Referer)) AS l, COUNT(*) AS c, MIN(Referer) FROM hits WHERE Referer <> '' GROUP BY k HAVING COUNT(*) > 100000 ORDER BY l DESC LIMIT 25;` | null | 0.004 | ClickHouse better | **N/A** |
| `SELECT SearchEngineID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, ClientIP ORDER BY c DESC LIMIT 10;` | 3.859 | 4.146 | DuckDB better | **6.92%** |
| `SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10;` | null | 11.540 | ClickHouse better | **N/A** |
| `SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT 10;` | 0.260 | 0.325 | DuckDB better | **20.0%** |
| `SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC LIMIT 10;` | 0.223 | 0.208 | ClickHouse better | **6.73%** |
| `SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND IsLink <> 0 AND IsDownload = 0 GROUP BY URL ORDER BY PageViews DESC LIMIT 10 OFFSET 1000;` | 0.174 | 0.129 | ClickHouse better | **25.86%** |
| `SELECT TraficSourceID, SearchEngineID, AdvEngineID, CASE WHEN (SearchEngineID = 0 AND AdvEngineID = 0) THEN Referer ELSE '' END AS Src, URL AS Dst, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 GROUP BY TraficSourceID, SearchEngineID, AdvEngineID, Src, Dst ORDER BY PageViews DESC LIMIT 10 OFFSET 1000;` | 0.395 | 0.392 | ClickHouse better | **0.76%** |
| `SELECT URLHash, EventDate, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND TraficSourceID IN (-1, 6) AND RefererHash = 3594120000172545465 GROUP BY URLHash, EventDate ORDER BY PageViews DESC LIMIT 10 OFFSET 100;` | 0.099 | 0.108 | DuckDB better | **8.33%** |
| `SELECT WindowClientWidth, WindowClientHeight, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND DontCountHits = 0 AND URLHash = 2868770270353813622 GROUP BY WindowClientWidth, WindowClientHeight ORDER BY PageViews DESC LIMIT 10 OFFSET 10000;` | 0.095 | 0.089 | ClickHouse better | **6.32%** |
| `SELECT DATE_TRUNC('minute', EventTime) AS M, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-14' AND EventDate <= '2013-07-15' AND IsRefresh = 0 AND DontCountHits = 0 GROUP BY DATE_TRUNC('minute', EventTime) ORDER BY DATE_TRUNC('minute', EventTime) LIMIT 10 OFFSET 1000;` | 0.095 | 0.073 | ClickHouse better | **23.16%** |
**GROUP LIMIT**  
| `SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase LIMIT 10;` | null | null | Both failed | **N/A** |
**WHERE ORDER LIMIT**  
| `SELECT * FROM hits WHERE URL LIKE '%google%' ORDER BY EventTime LIMIT 10;` | 2.563 | 20.774 | DuckDB better | **87.66%** |
| `SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime LIMIT 10;` | 0.680 | 2.415 | DuckDB better | **71.84%** |
| `SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY SearchPhrase LIMIT 10;` | 2.173 | 1.200 | ClickHouse better | **44.78%** |
| `SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime, SearchPhrase LIMIT 10;` | 0.719 | 2.423 | DuckDB better | **70.33%** |

Выводы по табличке: во-первых кликхаусу видимо нужно больше памяти, фейлился он чуть чаще (11 против 9 у дак дб, все фейлы были по причине нехватки памяти).

На простых аналитических запросах кликхаус показал себя лучше, преимущество до 50% (самая первая подтаблица). На сложных в целом тоже.
Также преимущество есть в запросах с `Group by`

Дак дб хорошо показал себя на запросах с поиском паттернов в тексте:
| Query | ClickHouse (s) | DuckDB (s) | Result | Difference |
|-------|----------------|------------|--------|------------|
| **LIKE паттерны** | | | | |
| `SELECT COUNT(*) FROM hits WHERE URL LIKE '%google%';` | 8.020 | 4.897 | ClickHouse better | **38.94%** |
| `SELECT SearchPhrase, MIN(URL), COUNT(*) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;` | 7.297 | 10.884 | DuckDB better | **32.96%** |
| `SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(*) AS c, COUNT(DISTINCT UserID) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;` | 14.886 | 19.885 | DuckDB better | **25.14%** |
| `SELECT * FROM hits WHERE URL LIKE '%google%' ORDER BY EventTime LIMIT 10;` | 2.563 | 20.774 | DuckDB better | **87.66%** |
| **REGEX и сложные текстовые операции** | | | | |
| `SELECT REGEXP_REPLACE(Referer, '^https?://(?:www\.)?([^/]+)/.*$', '\1') AS k, AVG(length(Referer)) AS l, COUNT(*) AS c, MIN(Referer) FROM hits WHERE Referer <> '' GROUP BY k HAVING COUNT(*) > 100000 ORDER BY l DESC LIMIT 25;` | null | 0.004 | DuckDB better | **N/A** |
| `SELECT CounterID, AVG(length(URL)) AS l, COUNT(*) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING COUNT(*) > 100000 ORDER BY l DESC LIMIT 25;` | 6.925 | 9.162 | DuckDB better | **24.42%** |
## Сравнение с прогонами умных ребят

| мой прогон | c6a.metal | c6a.4xlarge | c6a.xlarge |
|------------|-----------|-------------|------------|
| clickhouse better by 50.0% | duckdb better by 6.78% | clickhouse better by 12.31% | duckdb better by 50.42% |
| clickhouse better by 29.13% | duckdb better by 33.9% | duckdb better by 23.13% | duckdb better by 28.36% |
| clickhouse better by 2.02% | duckdb better by 49.33% | clickhouse better by 7.37% | duckdb better by 25.41% |
| clickhouse better by 19.7% | duckdb better by 24.15% | clickhouse better by 13.33% | duckdb better by 29.49% |
| duckdb better by 7.85% | duckdb better by 1.16% | clickhouse better by 4.23% | duckdb better by 49.24% |
| duckdb better | duckdb better by 6.23% | clickhouse better by 3.04% | duckdb better by 17.62% |
| clickhouse better by 46.42% | duckdb better by 43.43% | duckdb better by 4.03% | clickhouse better by 20.06% |
| clickhouse better by 16.22% | duckdb better by 42.33% | duckdb better by 19.87% | duckdb better by 19.08% |
| duckdb better by 67.5% | duckdb better by 51.4% | duckdb better by 13.01% | duckdb better by 12.67% |
| clickhouse better by 99.9% | duckdb better by 38.68% | duckdb better by 11.62% | duckdb better by 5.36% |
| duckdb better by 27.76% | duckdb better by 45.77% | duckdb better by 32.64% | duckdb better by 45.32% |
| duckdb better by 27.13% | duckdb better by 5.01% | duckdb better by 5.67% | duckdb better by 41.41% |
| duckdb better | clickhouse better by 14.24% | duckdb better by 14.14% | duckdb better by 19.63% |
| clickhouse better | clickhouse better by 4.87% | duckdb better by 3.16% | duckdb better by 18.53% |
| clickhouse better | duckdb better by 11.95% | duckdb better by 16.68% | duckdb better by 23.79% |
| clickhouse better by 99.84% | duckdb better by 23.03% | duckdb better by 19.8% | duckdb better by 10.69% |
| both failed | duckdb better by 15.26% | duckdb better by 35.14% | duckdb better by 46.06% |
| both failed | duckdb better by 11.01% | duckdb better by 18.1% | duckdb better by 34.94% |
| both failed | duckdb better by 31.35% | duckdb better by 51.31% | duckdb better by 62.09% |
| duckdb better by 41.3% | duckdb better by 48.58% | duckdb better by 32.57% | duckdb better by 61.0% |
| clickhouse better by 38.94% | duckdb better by 4.36% | duckdb better by 5.45% | duckdb better by 29.62% |
| duckdb better by 32.96% | duckdb better by 13.23% | duckdb better by 11.94% | duckdb better by 28.77% |
| duckdb better by 25.14% | duckdb better by 15.56% | duckdb better by 14.82% | duckdb better by 28.38% |
| duckdb better by 87.66% | duckdb better by 78.74% | clickhouse better by 0.18% | duckdb better by 97.04% |
| duckdb better by 71.84% | duckdb better by 55.0% | duckdb better by 78.85% | duckdb better by 75.05% |
| clickhouse better by 44.78% | clickhouse better by 18.13% | clickhouse better by 1.44% | duckdb better by 4.04% |
| duckdb better by 70.33% | duckdb better by 8.76% | duckdb better by 81.33% | duckdb better by 76.77% |
| duckdb better by 24.42% | duckdb better by 10.11% | duckdb better by 15.35% | duckdb better by 29.95% |
| clickhouse better | clickhouse better by 17.09% | clickhouse better by 0.31% | duckdb better by 28.98% |
| clickhouse better by 15.72% | duckdb better by 45.71% | clickhouse better by 2.96% | duckdb better by 3.23% |
| duckdb better by 6.92% | duckdb better by 13.69% | duckdb better by 4.89% | duckdb better by 6.39% |
| clickhouse better | duckdb better by 12.76% | duckdb better by 12.92% | duckdb better by 20.12% |
| clickhouse better | duckdb better by 32.57% | duckdb better by 45.67% | duckdb better by 62.11% |
| clickhouse better | duckdb better by 7.34% | duckdb better by 16.62% | duckdb better by 55.33% |
| clickhouse better | duckdb better by 7.84% | duckdb better by 16.09% | duckdb better by 51.13% |
| clickhouse better by 48.74% | clickhouse better by 12.01% | clickhouse better by 30.14% | clickhouse better by 34.4% |
| duckdb better by 20.0% | duckdb better by 10.26% | clickhouse better by 1.79% | duckdb better by 44.48% |
| clickhouse better by 6.73% | duckdb better by 11.55% | clickhouse better by 10.65% | duckdb better by 18.34% |
| clickhouse better by 25.86% | duckdb better by 22.75% | duckdb better by 4.44% | duckdb better by 52.46% |
| clickhouse better by 0.76% | clickhouse better by 39.78% | clickhouse better by 43.52% | duckdb better by 19.16% |
| duckdb better by 8.33% | duckdb better by 12.43% | duckdb better by 6.29% | duckdb better by 37.67% |
| clickhouse better by 6.32% | duckdb better by 7.64% | duckdb better by 1.53% | duckdb better by 41.71% |
| clickhouse better by 23.16% | clickhouse better by 5.1% | clickhouse better by 15.29% | duckdb better by 33.48% |

Это выписанные результаты по каждому запросу

Дальше сравнение разниц - я сравниваю не время, а то, насколько один вариант был лучше (или мнения моей машины и их не совпали)
| c6a.metal | c6a.4xlarge | c6a.xlarge |
|-----------|-------------|------------|
| opposite | same, their / mine = 0.2462 | opposite |
| opposite | opposite | opposite |
| opposite | same, their / mine = 3.6485 | opposite |
| opposite | same, their / mine = 0.6766 | opposite |
| same, their / mine = 0.1478 | opposite | same, their / mine = 6.2726 |
| opposite | opposite | same, their / mine = 0.4321 |
| opposite | opposite | opposite |
| same, their / mine = 0.7615 | same, their / mine = 0.1927 | same, their / mine = 0.1877 |
| opposite | opposite | opposite |
| same, their / mine = 1.6488 | same, their / mine = 1.1758 | same, their / mine = 1.6326 |
| same, their / mine = 0.1847 | same, their / mine = 0.2090 | same, their / mine = 1.5264 |
| opposite | opposite | opposite |
| same, their / mine = 1.1763 | same, their / mine = 0.7886 | same, their / mine = 1.4770 |
| opposite | opposite | opposite |
| same, their / mine = 0.4014 | same, their / mine = 0.3623 | same, their / mine = 0.8729 |
| same, their / mine = 0.6189 | same, their / mine = 0.5895 | same, their / mine = 1.1289 |
| same, their / mine = 0.8982 | opposite | same, their / mine = 1.1070 |
| same, their / mine = 0.7656 | same, their / mine = 1.0976 | same, their / mine = 1.0447 |
| same, their / mine = 0.4049 | same, their / mine = 0.0322 | opposite |
| same, their / mine = 0.1246 | same, their / mine = 1.1564 | same, their / mine = 1.0916 |
| same, their / mine = 0.4140 | same, their / mine = 0.6286 | same, their / mine = 1.2265 |
| opposite | same, their / mine = 0.1883 | opposite |
| same, their / mine = 1.9783 | same, their / mine = 0.7066 | same, their / mine = 0.9234 |
| same, their / mine = 0.2464 | same, their / mine = 0.6184 | same, their / mine = 0.7058 |
| same, their / mine = 0.5130 | opposite | same, their / mine = 2.2240 |
| opposite | same, their / mine = 1.5825 | opposite |
| opposite | opposite | opposite |
| same, their / mine = 52.3421 | same, their / mine = 57.2632 | opposite |
| same, their / mine = 1.4922 | same, their / mine = 0.7551 | same, their / mine = 4.5222 |
| opposite | opposite | opposite |
| same, their / mine = 0.2202 | same, their / mine = 0.6602 | opposite |
Количество несовпадений
| 13 | 11 | 15 |
Медианная относительная разница
| 0.6189 | 0.6602 | 1.2265 |

Как видно, результаты довольно сильно отличаются от моих, ближе всего был
c6a.xlarge, но у меня видимо получше система (вообще то нет, но результаты у меня лучше)

## Сравнение оборудования

| Параметр | c6a.metal | c6a.4xlarge | c6a.xlarge | мой сервер |
|----------|-----------|-------------|------------|------------|
| **vCPU** | 192 | 16 | 4 | 2 |
| **Ядер на сокет** | Много | 8 | 2 | 1 |
| **RAM** | 384 GB | 128 GB | 8 GB | 2 GB |
| **Частота CPU** | ~3.6 GHz | ~3.6 GHz | ~3.6 GHz | 2.1 GHz |
| **Кэш L3** | ~256 MB | ~64 MB | ~32 MB | 32 MB |

Мой запуск бенчмарка - тест на слабом оборудовании
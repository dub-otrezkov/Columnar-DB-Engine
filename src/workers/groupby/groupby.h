#pragma once

#include "narrow_io.h"

#include "workers/ao_engine/engine.h"

#include "csvio/csv_reader.h"
#include "workers/base.h"

#include "columns/operators/filter.h"
#include "utils/faster_hashmap/hashmap.h"

#include <map>

namespace JfEngine {

constexpr ui64 kUnlimited = -1;

struct TGroupByQuery {
    std::vector<std::shared_ptr<IOa>> cols;
    ui64 limit = kUnlimited;
    bool is_id = false;
};

class TGroupBy : public ITableInput {
public:
    TGroupBy(std::shared_ptr<ITableInput> jf_in, TGroupByQuery query, TAoQuery selects);

    Expected<void> SetupColumnsScheme() override;
    Expected<std::vector<TColumnPtr>> LoadRowGroup() override;

private:
    TGroupByQuery group_q_;
    TAoQuery agr_q_;
    
    std::shared_ptr<ITableInput> jf_in_;

    std::shared_ptr<IAoEngine> gc_eng;

    struct TGroup {
        std::shared_ptr<IAoEngine> eng;
        TNarrowTableInput io;

        TGroup(std::vector<TRowScheme>& scheme, TAoQuery agr_q) :
            eng(MakeAoEngine(std::move(agr_q))),
            io(scheme)
        {}
    };

    struct VectorStringHashed {
        std::vector<std::string> vals;
        ui64 hash;

        VectorStringHashed(std::vector<std::string> vals_ = {}) : vals(std::move(vals_)), hash(0) {
            static WyHash hasher;
            for (const auto& s : vals) {
                hash ^= hasher(s);
            }
        }

        bool operator==(const VectorStringHashed& other) const {
            if (hash != other.hash) {
                return false;
            }
            return vals == other.vals;
        }
    };

    struct WyHash {
        static inline ui64 wymix(ui64 a, ui64 b) {
            __uint128_t r = (__uint128_t)a * b;
            return (ui64)(r ^ (r >> 64));
        }

        static inline ui64 wyread4(const char* p) {
            ui32 v;
            __builtin_memcpy(&v, p, 4);
            return v;
        }

        static inline ui64 wyread8(const char* p) {
            ui64 v;
            __builtin_memcpy(&v, p, 8);
            return v;
        }

        ui64 operator()(const std::string& s) const {
            const char* p = s.data();
            ui64 len = s.size();
            ui64 seed = 0xa0761d6478bd642fULL;
            ui64 a, b;

            if (__builtin_expect(len <= 16, 1)) {
                if (__builtin_expect(len >= 4, 1)) {
                    a = (wyread4(p) << 32) | wyread4(p + ((len >> 3) << 2));
                    b = (wyread4(p + len - 4) << 32) | wyread4(p + len - 4 - ((len >> 3) << 2));
                } else if (__builtin_expect(len > 0, 1)) {
                    a = ((ui64)p[0] << 16) | ((ui64)p[len >> 1] << 8) | (ui64)p[len - 1];
                    b = 0;
                } else {
                    a = b = 0;
                }
            } else {
                ui64 i = len;
                if (__builtin_expect(i > 48, 0)) {
                    ui64 s1 = seed, s2 = seed;
                    do {
                        seed = wymix(wyread8(p) ^ 0xa0761d6478bd642fULL, wyread8(p + 8) ^ seed);
                        s1   = wymix(wyread8(p + 16) ^ 0xe7037ed1a0b428dbULL, wyread8(p + 24) ^ s1);
                        s2   = wymix(wyread8(p + 32) ^ 0x8a5cd789635d2dffULL, wyread8(p + 40) ^ s2);
                        p += 48;
                        i -= 48;
                    } while (__builtin_expect(i > 48, 0));
                    seed ^= s1 ^ s2;
                }
                while (__builtin_expect(i > 16, 0)) {
                    seed = wymix(wyread8(p) ^ 0xa0761d6478bd642fULL, wyread8(p + 8) ^ seed);
                    p += 16;
                    i -= 16;
                }
                a = wyread8(p + i - 16);
                b = wyread8(p + i - 8);
            }
            return wymix(0xa0761d6478bd642fULL ^ len, wymix(a ^ 0xa0761d6478bd642fULL, b ^ seed));
        }
    };

    struct VectorStringHash {
        WyHash str_hash;

        ui64 operator()(const std::vector<std::string>& v) const {
            // Полиномиальный хэш с большим нечётным множителем
            // h = h * M + hash(s_i), порядок элементов учитывается
            constexpr ui64 M = 0x517cc1b727220a95ULL; // большое нечётное
            ui64 h = 0xcbf29ce484222325ULL;            // offset basis
            for (const auto& s : v) {
                h = h * M + str_hash(s);
            }
            // Финальный mix
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;
            h ^= h >> 33;
            return h;
        }
    };


    struct VectorStringHasher {
        inline ui64 operator()(const VectorStringHashed& v) const {
            return v.hash;
        }
    };

    std::unordered_map<VectorStringHashed, TGroup, VectorStringHasher> groups_;
};

} // namespace JfEngine

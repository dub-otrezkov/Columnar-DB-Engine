#include "csv_reader.h"

#include <algorithm>
#include <iostream>

namespace JFEngine {

TCSVReader::TCSVReader(std::istream& in, i64 buf_size, char sep) :
    in_(in),
    sep_(sep),
    init_pos_(in_.tellg()),
    buf_size_(buf_size)
{
}

Expected<std::vector<std::string>> TCSVReader::ReadRow() {
    if (in_.eof() || buf_size_ == 0) {
        return {std::vector<std::string>(), MakeError<EofErr>()};
    }

    bool read_smth = false;

    bool in_quotes = false;

    // static std::vector<ui64> szs;
    // szs.resize(0);

    i64 total = 0;

    // szs.push_back(0);

    // auto start = in_.tellg();
    
    std::vector<std::string> ans;
    ans.push_back("");

    while (!in_.eof() && (buf_size_ == kUnlimitedBuffer || buf_size_ > 0)) {
        auto c = in_.get();
        total++;

        // std::cout << (char)c << std::endl;

        if (buf_size_ > 0) {
            buf_size_--;
        }

        if (in_quotes && c == EOF) {
            // std::cout << "Dkdkdkdk " << std::endl;
            return {std::vector<std::string>(), MakeError<EofErr>()};
        }
        if ((!in_quotes && c == '\n') || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            // szs.push_back(0);
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (in_.peek() == '\"') {
                    ans.back() += in_.get();
                    // szs.back()++;
                    // in_.get();
                    total++;
                    if (buf_size_ > 0) {
                        buf_size_--;
                    }
                } else if (in_.peek() != sep_ && in_.peek() != '\n') {
                    // std::cout << "dkdkdkdkdkdkkd " << std::endl;
                    return {std::vector<std::string>(), MakeError<EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
                // szs.back()++;
            }
        } else {
            ans.back() += c;
            // szs.back()++;
        }
    }

    // std::cout << "!! " << szs.size() << std::endl;
    // for (auto el : szs) {
    //     std::cout << el << " ";
    // }
    // std::cout << ":::" << " " << total << " " << in_.eof() << std::endl;
    
    // std::cout << ":::" << " " << total << " " << in_.eof() << " " << in_.tellg() << std::endl;

    // in_.clear();
    // in_.seekg(start);

    // for (ui64 i = 0; i < szs.size(); i++) {
    //     ans[i].resize(szs[i]);
    //     ui64 cur_i = 0;
    //     in_quotes = false;
    //     while (cur_i < szs[i]) {
    //         // std::cout << i << " " << ans[i] << std::endl;

    //         auto c = in_.get();
    //         total++;

    //         // std::cout << ":: " << c << std::endl;

    //         if (in_quotes && c == EOF) {
    //             return MakeError<EofErr>();
    //         }
    //         if ((!in_quotes && c == '\n') || c == EOF) {
    //             break;
    //         }

    //         read_smth = true;

    //         if (!in_quotes && c == sep_) {
    //         } else if (c == '"') {
    //             if (in_quotes) {
    //                 auto nxt = in_.peek();
    //                 if (nxt == '"') {
    //                     // ans[i] += in_.get();
    //                     ans[i][cur_i++] = in_.get();
    //                     // szs.back()++;
    //                 } else if (nxt != sep_ && nxt != '\n') {
    //                     return {std::vector<std::string>(), MakeError<EofErr>()};
    //                 } else {
    //                     in_quotes = false;
    //                 }
    //             } else if (cur_i == 0) {
    //                 in_quotes = true;
    //             } else {
    //                 // ans.back() += c;
    //                 // ans[i] += c;
    //                 ans[i][cur_i++] = c;
    //             }
    //         } else {
    //             // ans.back() += c;
    //             ans[i][cur_i++] = c;
    //             // ans[i] += c;
    //         }
    //     }
    //     // std::cout << ans[i] << "---" << (char) in_.peek() << std::endl;
    //     in_.get();
    //     if (in_quotes) {
    //         in_.get();
    //     }

    //     // std::cout << (char)in_.peek() << std::endl;
    // }

    if (!read_smth) {
        return {std::move(ans), MakeError<EofErr>()};
    }

    return {std::move(ans), nullptr};
}


Expected<std::vector<std::string>> TCSVReader::ReadRowBufI() {
    
    static char buf[kIBufSize];
    static i64 cpos = 0;
    static ui64 av = 0;
    static std::streampos st;

    auto read = [this]() -> char {
        if (cpos < av) {
            return buf[cpos++];
        }
        // std::cout << "::: " << av << " " << in_.tellg() << std::endl;


        // std::cout << "sdvg!" << " " << st << std::endl;
        st = in_.tellg();
        
        // std::cout << "sdvg!2" << " " << st << " " << in_.gcount() << " " << in_.tellg() << " | " << av << " " << cpos << " | " << (char) buf[0] << std::endl;

        av = in_.read(buf, kIBufSize).gcount();

        cpos = 0;
        return buf[cpos++];
    };
    auto peek = [this]() -> char {
        if (cpos < kIBufSize) {
            return buf[cpos];
        }
        return in_.peek();
    };
    auto shift_left = [this](i64 delta) -> void {
        if (cpos >= delta) {
            cpos -= delta;
        } else {
            // std::cout << "!!!!!!" << std::endl;
            in_.clear();
            in_.seekg(st + cpos - delta);
            st = st + cpos - delta;
            // std::cout <<"LL " << st + cpos - delta << std::endl;
            cpos = 0;
            av = in_.read(buf, kIBufSize).gcount();
        }
    };
    auto eof_c = [this]() -> bool {
        return cpos == av && in_.eof();
    };

    if (eof_c() || buf_size_ == 0) {
        return {std::vector<std::string>(), MakeError<EofErr>()};
    }

    read();
    shift_left(1);

    bool read_smth = false;

    bool in_quotes = false;

    std::vector<ui64> szs;

    i64 total = 0;

    // szs.push_back(0);

    Expected<std::vector<std::string>> ans_e(std::vector<std::string>(szs.size()));

    auto& ans = *ans_e.GetShared();
    ans.push_back("");

    while (!eof_c() && (buf_size_ == kUnlimitedBuffer || buf_size_ > 0)) {
        // auto c = in_.get();
        auto c = read();
        total++;
        // std::cout << cpos << " " << av << std::endl;

        if (buf_size_ > 0) {
            buf_size_--;
        }

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EofErr>()};
        }
        if ((!in_quotes && c == '\n') || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            // szs.push_back(0);
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (peek() == '\"') {
                    ans.back() += read();
                    // szs.back()++;
                    // in_.get();
                    // read();
                    total++;
                    if (buf_size_ > 0) {
                        buf_size_--;
                    }
                } else if (peek() != sep_ && peek() != '\n') {
                    // std::cout << peek() << std::endl;
                    // std::cout << "dkdkdkdkdkdkkd " << std::endl;
                    return {std::vector<std::string>(), MakeError<EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
                // szs.back()++;
            }
        } else {
            ans.back() += c;
            // szs.back()++;
        }
    }

    // std::cout << "!! " << szs.size() << std::endl;
    // for (auto el : szs) {
    //     std::cout << el << " ";
    // }
    // std::cout << ":::" << " " << total << " " << eof_c() << std::endl;
    
    // std::cout << ":::" << " " << total << " " << in_.eof() << " " << in_.tellg() << std::endl;

    // in_.clear();
    // in_.seekg(start);
    // shift_left(total);

    // // std::vector<std::string> ans(szs.size());
    // for (ui64 i = 0; i < szs.size(); i++) {
    //     ans[i] = std::string(szs[i], ' ');
    //     ui64 cur_i = 0;
    //     in_quotes = false;
    //     while (cur_i < szs[i]) {
    //         // std::cout << i << " " << ans[i] << std::endl;

    //         auto c = read();
    //         total++;

    //         // std::cout << ":: " << c << std::endl;

    //         if (in_quotes && c == EOF) {
    //             return MakeError<EofErr>();
    //         }
    //         if ((!in_quotes && c == '\n') || c == EOF) {
    //             break;
    //         }

    //         read_smth = true;

    //         if (!in_quotes && c == sep_) {
    //         } else if (c == '"') {
    //             if (in_quotes) {
    //                 auto nxt = peek();
    //                 if (nxt == '"') {
    //                     // ans[i] += in_.get();
    //                     ans[i][cur_i++] = read();
    //                     // szs.back()++;
    //                 } else if (nxt != sep_ && nxt != '\n') {
    //                     return {std::vector<std::string>(), MakeError<EofErr>()};
    //                 } else {
    //                     in_quotes = false;
    //                 }
    //             } else if (cur_i == 0) {
    //                 in_quotes = true;
    //             } else {
    //                 // ans.back() += c;
    //                 // ans[i] += c;
    //                 ans[i][cur_i++] = c;
    //             }
    //         } else {
    //             // ans.back() += c;
    //             ans[i][cur_i++] = c;
    //             // ans[i] += c;
    //         }
    //     }
    //     read();
    //     if (in_quotes) {
    //         read();
    //     }

    //     // std::cout << (char)in_.peek() << std::endl;
    // }

    if (!read_smth) {
        // return {std::move(ans), MakeError<EofErr>()};
        return MakeError<EofErr>();
    }

    return ans_e;
}

void TCSVReader::RestartRead() {
    in_.seekg(init_pos_);
}

} // namespace JFEngine
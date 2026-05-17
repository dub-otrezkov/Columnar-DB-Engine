#include "csv_reader.h"

#include <algorithm>
#include <iostream>

namespace JfEngine {

const ui64 kStrReserveSize = (1 << 5);

TCsvReader::TCsvReader(std::istream& in, char sep) :
    in_(in),
    sep_(sep),
    init_pos_(in_.tellg())
{
}

Expected<std::vector<std::string>> TCsvReader::ReadRow() {
    if (in_.eof()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;

    bool in_quotes = false;

    i64 total = 0;

    std::vector<std::string> ans;
    ans.push_back("");

    while (!in_.eof()) {
        auto c = in_.get();

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (in_.peek() == '\"') {
                    ans.back() += in_.get();
                } else if (in_.peek() != sep_ && in_.peek() != '\n' && in_.peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
            }
        } else {
            ans.back() += c;
        }
    }

    if (!read_smth) {
        return {std::move(ans), MakeError<EError::EofErr>()};
    }

    return {std::move(ans)};
}

TCsvOptimizedReader::TCsvOptimizedReader(std::istream& in, char sep) :
    in_(in),
    sep_(sep)
{
}

char TCsvOptimizedReader::ReadSym() {
    if (cpos_ < av_) {
        return buf_[cpos_++];
    }

    av_ = in_.read(buf_, kIBufSize).gcount();

    cpos_ = 0;
    return buf_[cpos_++];
}


bool TCsvOptimizedReader::EofC() {
    return cpos_ >= av_ && in_.eof();
}

char TCsvOptimizedReader::Peek() {
    if (cpos_ < kIBufSize) {
        return buf_[cpos_];
    }
    return in_.peek();
}

Expected<std::vector<std::string>> TCsvOptimizedReader::ReadRow() {
    if (EofC()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;

    bool in_quotes = false;

    Expected<std::vector<std::string>> ans_e{std::vector<std::string>()};

    auto& ans = ans_e.GetRes();
    ans.push_back("");

    while (!EofC()) {
        auto c = ReadSym();
        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (Peek() == '\"') {
                    ans.back() += ReadSym();
                } else if (Peek() != sep_ && Peek() != '\n' && Peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += c;
            }
        } else {
            ans.back() += c;
        }
    }

    if (!read_smth) {
        return MakeError<EError::EofErr>();
    }

    return ans_e;
}

Expected<void> TCsvOptimizedReader::ReadRow(TVectorString2d& out) {
    if (EofC()) {
        return MakeError<EError::EofErr>();
    }

    bool read_smth = false;

    bool in_quotes = false;

    out.NewRow();

    while (!EofC()) {
        auto c = ReadSym();
        if (in_quotes && c == EOF) {
            return MakeError<EError::EofErr>();
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            // ans.emplace_back();
            out.NewCol();
        } else if (c == '"') {
            if (in_quotes) {
                if (Peek() == '\"') {
                    // ans.back() += ReadSym();
                    out.WriteSymToLastCR(ReadSym());
                } else if (Peek() != sep_ && Peek() != '\n' && Peek() != '\r') {
                    return MakeError<EError::EofErr>();
                } else {
                    in_quotes = false;
                }
            } else if (out.LastEmpty()) {
                in_quotes = true;
            } else {
                // ans.back() += c;
                out.WriteSymToLastCR(c);
            }
        } else {
            // ans.back() += c;
            out.WriteSymToLastCR(c);
        }
    }

    if (!read_smth) {
        return EError::EofErr;
    }

    // return ans_e;
    return EError::NoError;
}

} // namespace JfEngine
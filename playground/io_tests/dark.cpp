#include <iostream>
#include <fstream>

#include <vector>
#include <memory>

// g++ -o dark dark.cpp -std=c++23 -O0;

std::vector<std::string> f1() {
    std::vector<std::string> ans = {
        "skkks",
        "slsllslskskks",
        "slkslskskls",
        "kskksksk"
    };

    return ans;
}

std::shared_ptr<std::vector<std::string>> f2() {
    std::vector<std::string> ans = {
        "skkks",
        "slsllslskskks",
        "slkslskskls",
        "kskksksk"
    };

    return std::make_shared<std::vector<std::string>>(std::move(ans));
}

std::vector<std::string> f3() {
    std::vector<std::string> ans = {
        "skkks",
        "slsllslskskks",
        "slkslskskls",
        "kskksksk"
    };

    return std::move(ans);
}

int main() {
    int cnt = 10000000;

    int ans = 0;

    for (int i = 0; i < cnt; i++) {
        ans += f2()->size() / i;
    }
}
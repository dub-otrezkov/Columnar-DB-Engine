#include "vector_1d.h"

std::string operator+(std::string_view a, std::string_view b) {
    std::string res(a.length() + b.length(), '0');
    std::memcpy(res.data(), a.data(), a.length());
    std::memcpy(res.data() + a.length(), b.data(), b.length());
    return res;
}

std::vector<char> Serialize(const StringVector& a) {
    std::vector<char> res(a.data_.size() + a.offsets_.size() * sizeof(ui64) + sizeof(ui64));
    std::memcpy(res.data(), a.data_.data(), a.data_.size());
    std::memcpy(
        res.data() + a.data_.size(),
        reinterpret_cast<const char*>(a.offsets_.data()),
        a.offsets_.size() * sizeof(ui64)
    );
    ui64 sz = a.data_.size();
    std::memcpy(
        res.data() + a.data_.size() + a.offsets_.size() * sizeof(ui64),
        &sz,
        sizeof(ui64)
    );

    return res;
}


StringVector UnserializeString(const std::vector<char>& a) {
    StringVector res;

    ui64 data_sz;
    std::memcpy(&data_sz, a.data() + a.size() - sizeof(ui64), sizeof(ui64));
    res.data_.resize(data_sz);
    res.offsets_.resize((a.size() - data_sz - sizeof(ui64)) / sizeof(ui64));

    std::memcpy(res.data_.data(), a.data(), data_sz);
    std::memcpy(reinterpret_cast<char*>(res.offsets_.data()), a.data() + data_sz, res.offsets_.size() * sizeof(ui64));

    return res;
}

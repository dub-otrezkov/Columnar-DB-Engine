#pragma once

template <typename Ostream>
class TEngine {

private:
    Ostream out;
    TCSVReader scheme_reader_;
    TCSVReader table_reader_;
};

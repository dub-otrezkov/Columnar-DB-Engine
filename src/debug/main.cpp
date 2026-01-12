#include <iostream>


#include "utils/csvio/csv_reader.h"
#include "table_node/nodes_factory.h"

template<typename T>
std::ostream& operator << (std::ostream& out, std::vector<T>& a) {
    for (int i = 0; i < a.size(); i++) out << a[i] << "\t";
    return out;
}

int main() {
    std::cout << "------ debug ------" << std::endl;

    TCSVReader r("input.txt", ",");

    {

        while (1) {
            auto [line, err] = r.ReadRow();

            if (err) {
                std::cout << "! " << err->Print() << std::endl;
                delete err;
                break;
            } else {
                std::cout << line << std::endl;
            }
        }
    }
}
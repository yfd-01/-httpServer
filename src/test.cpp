#include <iostream>
#include <fstream>

int main() {
    std::ofstream ofs;
    ofs.open("./2023_02_22.log", std::ios::app);
    ofs<< "hello\n";
    std::cout<< ofs.good();

    return 0;
}

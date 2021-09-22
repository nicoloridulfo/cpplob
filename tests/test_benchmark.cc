#include <math.h>

#include <chrono>
#include <iostream>
#include <random>

#include "../src/orderbook.h"

int main() {
    int orders = std::pow(10, 7);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    // std::default_random_engine generator(seed);
    // std::normal_distribution<int> distribution(10, 2);
    Book OB{};
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < orders; i++) {
        bool buy = rand() % 2 == 0;
        int size = rand() % 30;
        int price;
        if (buy)
            price = OB.getAsk() - (rand() % 4) - 2;
        else
            price = OB.getAsk() - (rand() % 4) - 2;
        // std::cout<<buy<<" "<<size<<" "<<price<<std::endl;
        OB.sendLimitOrder(buy, size, price);
        //std::cout << OB.ladder(10) << std::endl;
        //std::cin.get();
        // system("clear");
        /*if (i % (orders / 100) == 0){
            std::cout << i << ":" << OB.size() << std::endl;
            std::cout<<OB.ladder(10)<<std::endl;
        }*/
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << "Benchmark:" << std::endl;
    std::cout << orders << " orders" << std::endl;
    std::cout << microseconds / static_cast<double>(orders) << "us per order"
              << std::endl;
    std::cout << 1000 * static_cast<double>(orders) / microseconds
              << "k orders per second." << std::endl;
    std::cout << "Current state of the Book:" << std::endl;
    // std::cout << OB << std::endl;
    std::cout << "Count: " << OB.count() << std::endl;
    std::cout << "Size: " << OB.size() << std::endl;
}

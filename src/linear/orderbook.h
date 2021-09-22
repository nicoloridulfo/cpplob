#include <unordered_map>
#include <stack>
#include <ostream>
#include <iostream>
#include <limits>

struct Order{
    Order(){empty();};
    uint64_t id;
    uint64_t time;
    uint price, size, remaining;
    Order *next;
    void empty(){id=time=size=remaining=price=0;next=nullptr;};
};


class Orderbook{
    public:
        Orderbook();
        //~Orderbook();
        uint64_t sendLimitOrder(bool, uint32_t, double);
        uint64_t sendMarketOrder(bool, uint);
        uint64_t sendCancelOrder(uint64_t);

        double getBid() const;
        double getAsk() const;
        uint32_t getVolume(double);
        uint32_t size(){return numOrders;};
        uint32_t count();
        uint32_t countBids();
        uint32_t countAsks();

        std::stack<Order*> pointers;
        friend std::ostream& operator<<(std::ostream& os, const Orderbook& ob);
        std::string ladder(int n);

    private:
        Order* createOrder(uint size, uint remaining, int price, Order* next);
        void addBid(Order* order);
        void addAsk(Order* order);
        unsigned long nextid{1};
        std::unordered_map <uint64_t,Order*> orderids;
        Order *bid{nullptr}, *ask{nullptr};
        uint32_t numOrders{0};

};

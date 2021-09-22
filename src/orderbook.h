#include <limits>
#include <ostream>
#include <unordered_map>

using std::shared_ptr;

struct Limit;
struct Order {
    Order(uint64_t id, uint64_t time, uint size, uint remaining, uint price):
    id{id}, time{time}, size{size}, remaining{remaining}, price{price}{};
    uint64_t id;
    uint64_t time;
    uint size, remaining;
    uint price;
    shared_ptr<Order> nextOrder{nullptr};
    shared_ptr<Order> prevOrder{nullptr};
    shared_ptr<Limit> parentLimit{nullptr};
};

struct Limit{
    Limit(uint price):price{price}{};
    const uint price{0};
    uint totalVolume{0};
    uint size{0};
    //Limit* parent{nullptr};
    shared_ptr<Limit> next{nullptr};
    shared_ptr<Order> front{nullptr};
    shared_ptr<Order> tail{nullptr};
};

class Book {
    public:
    uint64_t sendLimitOrder(bool isBuy, uint size, double price);
    uint64_t sendMarketOrder(bool, uint size);
    uint64_t sendCancelOrder(uint64_t id);
    double getBid() const;
    double getAsk() const;
    uint size(){return numOrders;};
    uint count(){return numOrders;};
    std::string ladder(int depth);

    private:
    Order createOrder(uint size, double price);
    void addOrder(bool isBuy, uint size, uint remaining, uint price);
    shared_ptr<Order> execute(shared_ptr<Order> order);

    std::shared_ptr<Limit> createLimit(bool isbuy, uint price);
    std::unordered_map<uint64_t, std::shared_ptr<Order>> orderids{};
    std::unordered_map<uint, shared_ptr<Limit>> buylimitmap{};
    std::unordered_map<uint, shared_ptr<Limit>> selllimitmap{};
    uint64_t nextid{0};
    std::shared_ptr<Limit> lowestSell;
    std::shared_ptr<Limit> highestBuy;
    uint numOrders{0};
};
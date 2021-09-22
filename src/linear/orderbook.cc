#include "orderbook.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stack>
#include <sstream>

using std::cout;
using std::endl;

std::ostream& operator<<(std::ostream& os, const Order& o) {
    os << o.id;
    return os;
}

Orderbook::Orderbook() {
    for (int i = 0; i < 1000000; i++) {
        Order* order = new Order{};
        pointers.push(order);
    }
};

/*
Orderbook::~Orderbook(){
    while(!pointers.empty()){
        Order* order = pointers.top();
        pointers.pop();
        delete order;
    }
    Order* old;
    Order* head = bid;
    while(head!=nullptr){
        old = head;
        head = head->next;
        delete old;
    }
    head = ask;
    while(head!=nullptr){
        old = head;
        head = head->next;
        delete old;
    }
}
*/

double Orderbook::getAsk() const{
    if (ask != nullptr)
        return (*ask).price/100.0f;
    else
        return 0;
};
double Orderbook::getBid() const{
    if (bid != nullptr)
        return (*bid).price/100.0f;
    else
        return __DBL_MAX__;
}

std::ostream& operator<<(std::ostream& os, const Orderbook& ob) {
    os << "Limit Order Book:" << std::endl;
    os << "Bids:" << ob.getBid() << std::endl;
    os << "Asks:" << ob.getAsk() << std::endl;
    return os;
}

std::string Orderbook::ladder(int n = 5) {
    std::stringstream ss;
    ss << "Orderbook: (price | (remaining/size))" << endl;
    ss << "Asks:" << endl;
    std::stack<Order*> askstack{};
    Order* head = ask;
    for (int i = 0; i < n && head != nullptr; i++) {
        askstack.push(head);
        head = head->next;
    }
    while(!askstack.empty()){
        head = askstack.top(); askstack.pop();
        ss << head->price/100.0f << " | (" << head->remaining << "/" << head->size <<")"<<std::endl;
    }
    ss << "Bids:" << endl;
    head = bid;
    for (int i = 0; i < n && head != nullptr; i++) {
        ss << head->price/100.0f << " | (" << head->remaining << "/" << head->size <<")"<<std::endl;
        head = head->next;
    }
    return ss.str();
}

Order* Orderbook::createOrder(uint size, uint remaining, int price,
                              Order* next = nullptr) {
    if (pointers.empty()) {
        cout << "OUT OF POINTERS!" << endl;
        exit(1);
    }
    Order* order = pointers.top();
    pointers.pop();
    using namespace std::chrono;
    unsigned long time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now())
                .time_since_epoch())
            .count();
    order->id = nextid++;
    order->time = time;
    order->size = size;
    order->remaining = remaining;
    order->price = price;
    order->next = next;
    return order;
}

void execute(Order* existing) {
    // cout<<existing<<"just got matched"<<endl;
}

void Orderbook::addBid(Order* order) {
    // The bids are ordered with the highest price first
    if (bid == nullptr) {
        bid = order;
        return;
    }  // no bids

    // change head
    if (bid->price < order->price) {
        order->next = bid;
        bid = order;
        return;
    }
    // Move to where the next bid is either null or lower price
    Order* head = bid;
    while (head->next != nullptr && head->next->price >= order->price) {
        head = head->next;
    }
    order->next = head->next;
    head->next = order;
}

void Orderbook::addAsk(Order* order) {
    // The asks are ordered with lowest first
    if (ask == nullptr) {
        ask = order;
        return;
    }
    // if incoming is lower, make it the head
    if (ask->price > order->price) {
        order->next = ask;
        ask = order;
        return;
    }
    Order* head = ask;
    while (head->next != nullptr && head->next->price <= order->price) {
        head = head->next;
    }
    order->next = head->next;
    head->next = order;
}

uint64_t Orderbook::sendLimitOrder(bool isbuy, uint32_t size, double dprice) {
    uint price = dprice*100;
    uint remaining = size;
    if (isbuy) {
        if (ask == nullptr) {  // Ask is empty => add order to bids
            addBid(createOrder(size, remaining, price));
            numOrders++;
        } else {
            Order* currOrder = ask;
            while (currOrder != nullptr && currOrder->price <= price &&
                   remaining > 0) {
                // The incoming order exhaustes the present order
                if (currOrder->remaining < remaining) {
                    remaining -= currOrder->remaining;
                    currOrder->remaining = 0;
                } else if (currOrder->remaining > remaining) {
                    currOrder->remaining -= remaining;
                    remaining = 0;
                } else {
                    currOrder->remaining = 0;
                    remaining = 0;
                }
                if (currOrder->remaining == 0) {
                    execute(currOrder);
                    ask = currOrder->next;
                    currOrder->empty();
                    pointers.push(currOrder);
                    numOrders--;
                }
                currOrder = ask;
            }

            // Either because price too high or no more orders
            if (remaining > 0) {
                // An order is created and added at the head
                addBid(createOrder(size, remaining, price, bid));
                numOrders++;
            }
        }
    } else {
        if (bid == nullptr) {  // Ask is empty => add order to bids
            addAsk(createOrder(size, remaining, price));
            numOrders++;
        } else {
            Order* currOrder = bid;
            while (currOrder != nullptr && currOrder->price >= price &&
                   remaining > 0) {
                // The incoming order exhaustes the present order
                if (currOrder->remaining < remaining) {
                    remaining -= currOrder->remaining;
                    currOrder->remaining = 0;
                } else if (currOrder->remaining > remaining) {
                    currOrder->remaining -= remaining;
                    remaining = 0;
                } else {
                    currOrder->remaining = 0;
                    remaining = 0;
                }
                if (currOrder->remaining == 0) {
                    execute(currOrder);
                    bid = currOrder->next;
                    currOrder->empty();
                    pointers.push(currOrder);
                    numOrders--;
                }
                currOrder = bid;
            }
            if (remaining > 0) {
                // An order is created and added at the head
                addAsk(createOrder(size, remaining, price, ask));
                numOrders++;
            }
        }
    }
};
uint32_t Orderbook::count() { return countBids() + countAsks(); }
uint32_t Orderbook::countBids() {
    uint32_t n{0};
    Order* head = bid;
    while (head != nullptr) {
        n++;
        head = head->next;
    }
    return n;
}
uint32_t Orderbook::countAsks() {
    uint32_t n{0};
    Order* head = bid;
    head = ask;
    while (head != nullptr) {
        n++;
        head = head->next;
    }
    return n;
}

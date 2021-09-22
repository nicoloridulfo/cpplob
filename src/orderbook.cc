#include "orderbook.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <stack>

using std::cout;
using std::endl;
using std::shared_ptr;

shared_ptr<Limit> Book::createLimit(bool isbuy, uint price) {
    shared_ptr<Limit> limit = std::make_shared<Limit>(price);
    if (isbuy) {
        // If there are no limits
        if (highestBuy == nullptr) {
            highestBuy = limit;
        } else if (highestBuy->price < price) {
            // If the new limit is higher than the head
            limit->next = highestBuy;
            highestBuy = limit;
        } else {
            shared_ptr<Limit> head = highestBuy;
            while (head->next != nullptr && head->next->price > limit->price) {
                head = head->next;
            }
            limit->next = head->next;
            head->next = limit;
        }
        buylimitmap[price] = limit;
    } else {
        // If there are no limits
        if (lowestSell == nullptr) {
            lowestSell = limit;
        } else if (lowestSell->price > price) {
            // If the new limit is higher than the head
            limit->next = lowestSell;
            lowestSell = limit;
        } else {
            shared_ptr<Limit> head = lowestSell;
            while (head->next != nullptr && head->next->price < limit->price) {
                head = head->next;
            }
            limit->next = head->next;
            head->next = limit;
        }
        selllimitmap[price] = limit;
    }
    return limit;
}

void Book::addOrder(bool isbuy, uint size, uint remaining, uint price) {
    unsigned long time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now())
                .time_since_epoch())
            .count();
    shared_ptr<Order> order =
        std::make_shared<Order>(nextid++, time, size, remaining, price);
    shared_ptr<Limit> limit;
    if (isbuy) {
        // get the limit, if does not exist, create it
        if (buylimitmap.find(price) == buylimitmap.end()) {
            limit = createLimit(isbuy, price);
        } else {
            limit = buylimitmap.at(price);
        }
    } else {
        if (selllimitmap.find(price) == selllimitmap.end()) {
            limit = createLimit(isbuy, price);
        } else {
            limit = selllimitmap.at(price);
        }
    }
    order->parentLimit = limit;     // set the parent limit
    if (limit->front == nullptr) {  // if the limit is empty set the front
        limit->front = order;
    } else {  // otherwise sets the tails next to this order
        limit->tail->nextOrder = order;
    }
    limit->tail = order;  // set out order as the tail
    limit->size++;
    limit->totalVolume += remaining;
    numOrders++;
}

// Removes the given order from the limit and orderids map
// + Updates the volume in the limit.
// returns a pointer to the next order
shared_ptr<Order> Book::execute(shared_ptr<Order> order) {
    shared_ptr<Limit> limit = order->parentLimit;
    limit->totalVolume -= order->remaining;
    limit->size--;
    order->remaining = 0;
    if (limit->front == limit->tail) {
        limit->front = nullptr;
        limit->tail = nullptr;
    } else {
        limit->front = order->nextOrder;
    }
    orderids.erase(order->id);
    numOrders--;
    return limit->front;
}

uint64_t Book::sendLimitOrder(bool isbuy, uint size, double dprice) {
    uint price = dprice * 100;
    uint remaining = size;
    // The following two variables depend on if its a buy or sell
    shared_ptr<Limit> bestLimit;
    std::function<bool(uint, uint)> compare;
    if (isbuy) {
        bestLimit = lowestSell;
        compare = [](int first, int second) { return first <= second; };
    } else {
        bestLimit = highestBuy;
        compare = [](int first, int second) { return first >= second; };
    }
    if (bestLimit == nullptr) {
        addOrder(isbuy, size, size, price);
    } else {
        // While there are limits to exhaust
        shared_ptr<Limit> currLimit = bestLimit;
        while (currLimit != nullptr && compare(currLimit->price, price)) {
            // While there are orders in the limit to exhaust
            shared_ptr<Order> currOrder = currLimit->front;
            while (currOrder != nullptr && remaining > 0) {
                if (currOrder->remaining < remaining) {
                    remaining -= currOrder->remaining;
                    currOrder = execute(currOrder);
                } else if (currOrder->remaining > remaining) {
                    currOrder->remaining -= remaining;
                    remaining = 0;
                } else {
                    currOrder = execute(currOrder);
                    remaining = 0;
                }
            }
            // If the while loop was exited because of no more orders
            if(currLimit->size==0){
                if (isbuy) {
                    selllimitmap.erase(currLimit->price);
                    lowestSell = currLimit->next;
                    currLimit = lowestSell;
                } else {
                    buylimitmap.erase(currLimit->price);
                    highestBuy = currLimit->next;
                    currLimit = highestBuy;
                }
            }

            // If there if no more is needed exit limit-loop
            if (remaining == 0) {
                break;
            }
        }
        if (remaining > 0) {
            addOrder(isbuy, size, remaining, price);
        }
    }
};
double Book::getAsk() const {
    if (lowestSell != nullptr)
        return lowestSell->price / 100.0f;
    else
        return __DBL_MAX__;
};
double Book::getBid() const {
    if (highestBuy != nullptr)
        return highestBuy->price / 100.0f;
    else
        return 0;
}
std::string Book::ladder(int depth = 4) {
    std::stringstream ss;
    ss << "Orderbook: (price | volume)" << endl;
    ss << "Total orders in book: "<<numOrders<<endl;
    ss << "Best bid|ask: "<<getBid()<<"|"<<getAsk()<<endl;
    ss << "Asks:" << endl;
    std::stack<shared_ptr<Limit>> askstack{};
    shared_ptr<Limit> asks;
    shared_ptr<Limit> currLimit = lowestSell;
    while (currLimit != nullptr && askstack.size() <= depth) {
        askstack.push(currLimit);
        currLimit = currLimit->next;
    }
    while (!askstack.empty()) {
        currLimit = askstack.top();
        askstack.pop();
        ss << currLimit->price / 100.0f << " | " << currLimit->totalVolume
           << endl;
    }
    ss << "Bids:" << endl;
    currLimit = highestBuy;
    while (currLimit != nullptr && askstack.size() <= depth) {
        ss << currLimit->price / 100.0f << " | " << currLimit->totalVolume
           << endl;
        currLimit = currLimit->next;
    }
    return ss.str();
}

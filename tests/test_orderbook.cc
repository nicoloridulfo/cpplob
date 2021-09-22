#include <limits>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include "../src/orderbook.h"

using std::cout;
using std::endl;

void test_Book(){
    Book OB{};
    assert(OB.getBid()==0);
    assert(OB.getAsk()==__DBL_MAX__);
}

void test_Book_bidask(){
    /*
     * Testing the ordering in the Book
     */
    Book OB{};
    // Bids
    OB.sendLimitOrder(true, 1, 100);
    OB.sendLimitOrder(true, 1, 90);
    assert(OB.getBid()==100);
    assert(OB.size()==2);
    assert(OB.count()==2);

    OB = Book();
    OB.sendLimitOrder(true, 1, 90);
    OB.sendLimitOrder(true, 1, 100);
    assert(OB.getBid()==100);

    OB = Book();
    OB.sendLimitOrder(true, 1, 80);
    OB.sendLimitOrder(true, 1, 90);
    OB.sendLimitOrder(true, 1, 100);
    assert(OB.getBid()==100);

    //Asks
    OB = Book();
    OB.sendLimitOrder(false, 1, 100);
    OB.sendLimitOrder(false, 1, 90);
    assert(OB.getAsk()==90);

    OB = Book();
    OB.sendLimitOrder(false, 1, 90);
    OB.sendLimitOrder(false, 1, 100);
    assert(OB.getAsk()==90);

    OB = Book();
    OB.sendLimitOrder(false, 1, 80);
    OB.sendLimitOrder(false, 1, 90);
    OB.sendLimitOrder(false, 1, 100);
    assert(OB.getAsk()==80);
    assert(OB.size()==3);
    assert(OB.count()==3);
}

void test_execution(){
    Book OB{};
    OB.sendLimitOrder(true, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    assert(OB.getBid()==0);
    assert(OB.getAsk()==__DBL_MAX__);
    assert(OB.size()==0);
    assert(OB.count()==0);

    OB = Book();
    OB.sendLimitOrder(true, 20, 10);
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    assert(OB.getBid()==0);
    assert(OB.getAsk()==__DBL_MAX__);
    assert(OB.size()==0);
    assert(OB.count()==0);

    OB = Book();
    OB.sendLimitOrder(false, 20, 10);
    OB.sendLimitOrder(true, 10, 10);
    OB.sendLimitOrder(true, 10, 10);
    assert(OB.getBid()==0);
    assert(OB.getAsk()==__DBL_MAX__);
    assert(OB.size()==0);
    assert(OB.count()==0);

    OB = Book();
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(true, 20, 20);
    assert(OB.getBid()==0);
    assert(OB.getAsk()==__DBL_MAX__);
    assert(OB.size()==0);
    assert(OB.count()==0);

    OB = Book();
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(true, 20, 20);
    assert(OB.getBid()==0);
    assert(OB.getAsk()==10);
    assert(OB.size()==1);
    assert(OB.count()==1);

    OB = Book();
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 10, 10);
    OB.sendLimitOrder(false, 15, 10);
    OB.sendLimitOrder(true, 40, 20);
    assert(OB.getBid()==20);
    assert(OB.getAsk()==__DBL_MAX__);
    assert(OB.size()==1);
    assert(OB.count()==1);
}

int main(){
    test_Book();
    test_Book_bidask();
    test_execution();
    cout<<"All tests passed!"<<endl;
    return 0;
}

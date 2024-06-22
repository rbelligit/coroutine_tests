#include "range.hpp"

#include <iostream>


rbelli::RangeRet<int> range1()
{
    int val = 0;
    while(val < 10)
    {
        co_yield val;
        ++val;
    }
}

rbelli::RangeRet<int> xrange(int ini, int fim)
{
    int val = ini;
    while(val < fim)
    {
        co_yield val;
        ++val;
    }
    
}

rbelli::RangeRet<int> teste_f2()
{
    co_yield 4;
    co_yield 5;
}

rbelli::RangeRet<int> teste_f1()
{
    co_yield 2;
    co_yield 3;
    co_await teste_f2();
    co_yield 6;
}

int main()
{
    {
        // Sem 
        auto ret1 = range1();
        for(auto val = ret1.nextValue(); val.has_value(); val=ret1.nextValue())
        {
            std::cout << "val=" << val.value() << std::endl;
        }
    }

    for(auto &v : range1())
    {
        std::cout << "val2=" << v << std::endl;
    }

    

    for(auto &v : xrange(0,20))
    {
        std::cout << v << ",";
    }
    std::cout << std::endl;

    

    auto v1 = xrange(0,10);
    auto it = v1.begin();
    std::cout << *it << std::endl;


    for(int val : teste_f1())
    {
        std::cout << val << ",";
    }
    std::cout << std::endl;
    
    

    for(auto &v : teste_f1())
    {
        std::cout << v << ",";
    }
    std::cout << std::endl;

    return 0;
}
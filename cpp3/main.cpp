#include "function.h"
#include <iostream>

int main() {
    try {
        std::vector<TFunctionPtr> cont;

        auto f = FunctionFactory::Create("ident", {});
        auto g = FunctionFactory::Create("const", {2});
        auto h = FunctionFactory::Create("polynomial", {-3, 1});
        auto sum = *f + *g;
        auto prod = *sum * *h;
        std::cout << prod->ToString() << std::endl;
        std::cout << (*prod)(15) << std::endl;
        std::cout << prod->GetDeriv(-11) << std::endl;
        std::cout << "Градиентный спуск: " << std::endl;
        std::cout << GradientDescentRoot(prod, 10, 100) << std::endl;

        std::cout << GradientDescentRoot(prod, -20, 100) << std::endl;

        // std::vector<TFunctionPtr> cont;
        // auto f = FunctionFactory::Create("power", {2}); // PowerFunc x^2
        // cont.push_back(f);
        // auto g = FunctionFactory::Create("polynomial", {7, 0, 3, 15}); // TPolynomial 7 + 3*x^2 + 15*x^3
        // cont.push_back(g);
        // for (const auto &ptr : cont) {
        //     std::cout << ptr->ToString() << " for x = 10 is " << (*ptr)(10) << std::endl;
        // }
        // auto p = *f + *g;
        // std::cout << p->GetDeriv(1) << std::endl; // 53
        // std::cout << g->GetDeriv(3) << std::endl; // 423
        // auto h = *f + "abc"; // std::logic_error
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
#include <complex>
#include <expression.hpp>
#include <iostream>

int main() {
    Expression<Complexes_t> ex(Complexes_t(1));

    std::cout << (Expression(2) * Expression("x").pow(Expression(2)) -
                  Expression(1) / Expression("x"))
                     .diff("x");
    return 0;
}
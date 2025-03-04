#include <complex>
#include <expression.hpp>
#include <iostream>

int main() {
    symcpp::Expression x("x");

    std::cout << (2 * x.pow(2) - 1 / x).diff("x") << std::endl;
    auto df_dx = (x * symcpp::sin(x)).diff("x");
    std::cout << df_dx << " = " << df_dx.eval({{"x", 1}});
    return 0;
}
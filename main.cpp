#include <cxxopts.hpp>
#include <iostream>
#include <map>
#include <string>

#include "expression.hpp"

int main(int argc, char* argv[]) {
    cxxopts::Options options(
        "differentiator", "A symbolic differentiator and expression evaluator");

    options.add_options()("e,eval", "Evaluate expression with given variables",
                          cxxopts::value<std::string>())(
        "d,diff", "Differentiate expression with respect to a variable",
        cxxopts::value<std::string>())("b,by", "Variable to differentiate by",
                                       cxxopts::value<std::string>())(
        "h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("eval")) {
        std::string expression_str = result["eval"].as<std::string>();

        std::map<std::string, symcpp::Reals_t> variables;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            size_t eq_pos = arg.find('=');
            if (eq_pos != std::string::npos) {
                std::string var_name = arg.substr(0, eq_pos);
                symcpp::Reals_t var_value = std::stold(arg.substr(eq_pos + 1));
                variables[var_name] = var_value;
            }
        }

        auto expr = symcpp::parse_expression(expression_str);
        std::cout << expr.eval(variables) << std::endl;
    }

    if (result.count("diff")) {
        std::string expression_str = result["diff"].as<std::string>();
        std::string diff_var = result["by"].as<std::string>();

        auto expr = symcpp::parse_expression(expression_str);
        auto diff_expr = expr.diff(diff_var);

        std::cout << diff_expr << std::endl;
    }

    return 0;
}
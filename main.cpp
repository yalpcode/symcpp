#include <cxxopts.hpp>
#include <iostream>
#include <map>
#include <string>

#include "expression.hpp"

bool contains_imaginary_unit(const std::string& str) {
    return str.find('i') != std::string::npos;
}

symcpp::Complexes_t parse_complex(const std::string& str) {
    size_t i_pos = str.find('i');
    if (i_pos == std::string::npos) {
        return symcpp::Complexes_t(std::stod(str), 0.0);
    }

    std::string real_part, imag_part;
    size_t sign_pos = str.find_last_of("+-", i_pos - 1);
    if (sign_pos == std::string::npos) {
        imag_part = str.substr(0, i_pos);
    } else {
        real_part = str.substr(0, sign_pos);
        imag_part = str.substr(sign_pos, i_pos - sign_pos);
    }

    double real = real_part.empty() ? 0.0 : std::stod(real_part);
    double imag = imag_part.empty() || imag_part == "+" ? 1.0
                  : imag_part == "-"                    ? -1.0
                                                        : std::stod(imag_part);

    return symcpp::Complexes_t(real, imag);
}

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
        bool use_complex = contains_imaginary_unit(expression_str);

        for (int i = 1; i < argc && !use_complex; ++i) {
            if (contains_imaginary_unit(argv[i])) {
                use_complex = true;
            }
        }

        if (use_complex) {
            std::map<std::string, symcpp::Complexes_t> variables;
            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                size_t eq_pos = arg.find('=');
                if (eq_pos != std::string::npos) {
                    std::string var_name = arg.substr(0, eq_pos);
                    std::string var_value_str = arg.substr(eq_pos + 1);
                    symcpp::Complexes_t var_value =
                        parse_complex(var_value_str);
                    variables[var_name] = var_value;
                }
            }

            auto expr =
                symcpp::parse_expression<symcpp::Complexes_t>(expression_str);
            std::cout << expr.eval(variables) << std::endl;
        } else {
            std::map<std::string, symcpp::Reals_t> variables;
            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                size_t eq_pos = arg.find('=');
                if (eq_pos != std::string::npos) {
                    std::string var_name = arg.substr(0, eq_pos);
                    symcpp::Reals_t var_value =
                        std::stod(arg.substr(eq_pos + 1));
                    variables[var_name] = var_value;
                }
            }

            auto expr =
                symcpp::parse_expression<symcpp::Reals_t>(expression_str);
            std::cout << expr.eval(variables) << std::endl;
        }
    }

    if (result.count("diff")) {
        std::string expression_str = result["diff"].as<std::string>();
        std::string diff_var = result["by"].as<std::string>();
        bool use_complex = contains_imaginary_unit(expression_str);

        if (use_complex) {
            auto expr =
                symcpp::parse_expression<symcpp::Complexes_t>(expression_str);
            auto diff_expr = expr.diff(diff_var);
            std::cout << diff_expr << std::endl;
        } else {
            auto expr =
                symcpp::parse_expression<symcpp::Reals_t>(expression_str);
            auto diff_expr = expr.diff(diff_var);
            std::cout << diff_expr << std::endl;
        }
    }

    return 0;
}

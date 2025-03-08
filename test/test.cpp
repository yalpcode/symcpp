#include <gtest/gtest.h>

#include "expression.hpp"

TEST(ExpressionParsingTest, SimpleAddition) {
    auto expr = symcpp::parse_expression("2 + 3");
    EXPECT_EQ(expr.eval({}), 5);
}

TEST(ExpressionParsingTest, VariableEvaluation) {
    auto expr = symcpp::parse_expression("x + 3");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 2}};
    EXPECT_EQ(expr.eval(vars), 5);
}

TEST(ExpressionParsingTest, MultiplicationAndDivision) {
    auto expr = symcpp::parse_expression("2 * x / 4");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 8}};
    EXPECT_EQ(expr.eval(vars), 4);
}

TEST(ExpressionParsingTest, PowerFunction) {
    auto expr = symcpp::parse_expression("x ^ 2");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 3}};
    EXPECT_EQ(expr.eval(vars), 9);
}

TEST(ExpressionParsingTest, SinFunction) {
    auto expr = symcpp::parse_expression("sin(x)");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 0}};
    EXPECT_EQ(expr.eval(vars), 0);
}

TEST(SymbolicDifferentiationTest, PowerFunction) {
    auto expr = symcpp::parse_expression("x ^ 2");
    auto diff_expr = expr.diff("x");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 2}};
    EXPECT_EQ(diff_expr.eval(vars), 4);
}

TEST(SymbolicDifferentiationTest, SinFunction) {
    auto expr = symcpp::parse_expression("sin(x)");
    auto diff_expr = expr.diff("x");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 0}};
    EXPECT_EQ(diff_expr.eval(vars), 1);
}

TEST(SymbolicDifferentiationTest, LnFunction) {
    auto expr = symcpp::parse_expression("ln(x)");
    auto diff_expr = expr.diff("x");
    std::map<std::string, symcpp::Reals_t> vars = {{"x", 1}};
    EXPECT_EQ(diff_expr.eval(vars), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <cmath>
#include <complex>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

namespace symcpp {
using Reals_t = long double;
class Complexes_t : public std::complex<Reals_t> {
   public:
    using std::complex<Reals_t>::complex;
    explicit operator long double() const {
        return static_cast<Reals_t>(this->real());
    }

    Complexes_t(const std::complex<Reals_t>& other)
        : std::complex<Reals_t>(other) {}
};

std::string to_string(const Complexes_t& c) {
    return "(" + std::to_string(c.real()) + ", " + std::to_string(c.imag()) +
           ")";
}

template <typename T>
concept Numeric =
    std::is_arithmetic_v<T> || std::is_same_v<T, std::complex<long double>> ||
    std::is_same_v<T, Complexes_t>;

template <Numeric _Domain>
class Expression;

template <Numeric _Domain = Reals_t>
class ExpressionImpl {
   public:
    ExpressionImpl() = default;
    virtual ~ExpressionImpl() = default;

    virtual _Domain eval(const std::map<std::string, _Domain>&) const = 0;
    virtual Expression<_Domain> diff(const std::string&) const = 0;

    virtual std::string to_string() const = 0;
};

template <Numeric _Domain = Reals_t>
class Expression {
    Expression(std::shared_ptr<ExpressionImpl<_Domain>> impl) : impl(impl) {}

    std::shared_ptr<ExpressionImpl<_Domain>> impl;

   public:
    Expression() = default;

    template <Numeric T>
    Expression(T);
    Expression(const std::string&);
    Expression(const Expression& other) : impl(other.impl) {}
    Expression(Expression&& other) noexcept : impl(std::move(other.impl)) {}

    Expression& operator=(const Expression& other) {
        if (this != &other) {
            impl = other.impl;
        }
        return *this;
    }
    Expression& operator=(Expression&& other) noexcept {
        if (this != &other) {
            impl = std::move(other.impl);
        }
        return *this;
    }

    ~Expression() = default;

    Expression operator+(const Expression&) const;
    Expression operator-(const Expression&) const;
    Expression operator*(const Expression&) const;
    Expression operator/(const Expression&) const;
    Expression pow(const Expression&) const;

    template <Numeric T>
    friend Expression operator+(const T& lhs, const Expression& rhs) {
        return Expression(lhs) + rhs;
    }
    template <Numeric T>
    friend Expression operator-(const T& lhs, const Expression& rhs) {
        return Expression(lhs) - rhs;
    }
    template <Numeric T>
    friend Expression operator*(const T& lhs, const Expression& rhs) {
        return Expression(lhs) * rhs;
    }
    template <Numeric T>
    friend Expression operator/(const T& lhs, const Expression& rhs) {
        return Expression(lhs) / rhs;
    }
    template <Numeric T>
    friend Expression pow(const T& lhs, const Expression& rhs) {
        return Expression(lhs).pow(rhs);
    }

    Expression sin() const;
    Expression cos() const;
    Expression ln() const;
    Expression exp() const;

    std::string to_string() const { return impl ? impl->to_string() : "null"; }

    _Domain eval(const std::map<std::string, _Domain>& variables) const {
        return impl ? impl->eval(variables) : _Domain{};
    }
    Expression diff(const std::string& variable) const {
        return impl ? impl->diff(variable) : _Domain{};
    }

    friend std::ostream& operator<<(std::ostream& os, const Expression& ex) {
        os << ex.to_string();
        return os;
    }
};

template <Numeric _Domain>
class Value : public ExpressionImpl<_Domain> {
   public:
    Value(_Domain value) : value(value) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return value;
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return _Domain{};
    };

    virtual std::string to_string() const override {
        if constexpr (std::is_same_v<_Domain, Complexes_t>) {
            return symcpp::to_string(value);
        } else {
            return std::to_string(value);
        }
    }

    _Domain getValue() const { return value; }

   private:
    _Domain value;
};

template <Numeric _Domain>
class Variable : public ExpressionImpl<_Domain> {
   public:
    Variable(std::string variable) : variable(variable) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        auto it = variables.find(variable);
        if (it != variables.end()) {
            return it->second;
        }
        if (variable == "i") {
            return _Domain(Complexes_t(0, 1));
        }
        throw std::runtime_error("Variable not found: " + variable);
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        if (this->variable == variable) {
            return _Domain{1};
        }
        return _Domain{};
    };

    virtual std::string to_string() const override { return variable; }

   private:
    std::string variable;
};

template <Numeric _Domain>
class Add : public ExpressionImpl<_Domain> {
   public:
    Add(Expression<_Domain> lhs, Expression<_Domain> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return lhs.eval(variables) + rhs.eval(variables);
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return lhs.diff(variable) + rhs.diff(variable);
    };

    virtual std::string to_string() const override {
        return "(" + lhs.to_string() + " + " + rhs.to_string() + ")";
    }

   private:
    Expression<_Domain> lhs, rhs;
};

template <Numeric _Domain>
class Subtract : public ExpressionImpl<_Domain> {
   public:
    Subtract(Expression<_Domain> lhs, Expression<_Domain> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return lhs.eval(variables) - rhs.eval(variables);
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return lhs.diff(variable) - rhs.diff(variable);
    };

    virtual std::string to_string() const override {
        return "(" + lhs.to_string() + " - " + rhs.to_string() + ")";
    }

   private:
    Expression<_Domain> lhs, rhs;
};

template <Numeric _Domain>
class Multiply : public ExpressionImpl<_Domain> {
   public:
    Multiply(Expression<_Domain> lhs, Expression<_Domain> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return lhs.eval(variables) * rhs.eval(variables);
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return lhs.diff(variable) * rhs + lhs * rhs.diff(variable);
    };

    virtual std::string to_string() const override {
        return "(" + lhs.to_string() + " * " + rhs.to_string() + ")";
    }

   private:
    Expression<_Domain> lhs, rhs;
};

template <Numeric _Domain>
class Divide : public ExpressionImpl<_Domain> {
   public:
    Divide(Expression<_Domain> lhs, Expression<_Domain> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        _Domain divider = rhs.eval(variables);
        if (divider == _Domain(0.)) {
            throw std::runtime_error("Division by zero");
        }
        return lhs.eval(variables) / divider;
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return (lhs.diff(variable) * rhs - lhs * rhs.diff(variable)) /
               (rhs * rhs);
    };

    virtual std::string to_string() const override {
        return "(" + lhs.to_string() + " / " + rhs.to_string() + ")";
    }

   private:
    Expression<_Domain> lhs, rhs;
};

template <Numeric _Domain>
class Power : public ExpressionImpl<_Domain> {
   public:
    Power(Expression<_Domain> lhs, Expression<_Domain> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return std::pow(lhs.eval(variables), rhs.eval(variables));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return lhs.pow(rhs) *
               (rhs.diff(variable) * lhs.ln() + rhs * lhs.diff(variable) / lhs);
    };

    virtual std::string to_string() const override {
        return "(" + lhs.to_string() + " ^ " + rhs.to_string() + ")";
    }

   private:
    Expression<_Domain> lhs, rhs;
};

template <Numeric _Domain>
class Sin : public ExpressionImpl<_Domain> {
   public:
    Sin(Expression<_Domain> expr) : expr(std::move(expr)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return _Domain(std::sin(expr.eval(variables)));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return expr.cos() * expr.diff(variable);
    };

    virtual std::string to_string() const override {
        return "sin(" + expr.to_string() + ")";
    }

   private:
    Expression<_Domain> expr;
};

template <Numeric _Domain>
class Cos : public ExpressionImpl<_Domain> {
   public:
    Cos(Expression<_Domain> expr) : expr(std::move(expr)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return _Domain(std::cos(expr.eval(variables)));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return Expression<_Domain>(-1) * expr.sin() * expr.diff(variable);
    };

    virtual std::string to_string() const override {
        return "cos(" + expr.to_string() + ")";
    }

   private:
    Expression<_Domain> expr;
};

template <Numeric _Domain>
class Ln : public ExpressionImpl<_Domain> {
   public:
    Ln(Expression<_Domain> expr) : expr(std::move(expr)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        _Domain phlogarithmic = expr.eval(variables);
        if constexpr (std::is_same_v<_Domain, Complexes_t>) {
            if (phlogarithmic.real() <= 0) {
                throw std::runtime_error(
                    "Ln domain error: real part must be positive");
            }
        } else {
            if (phlogarithmic <= _Domain(0)) {
                throw std::runtime_error("Ln domain error");
            }
        }
        return _Domain(std::log(expr.eval(variables)));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return Expression<_Domain>(1) / expr * expr.diff(variable);
    };

    virtual std::string to_string() const override {
        return "ln(" + expr.to_string() + ")";
    }

   private:
    Expression<_Domain> expr;
};

template <Numeric _Domain>
class Exp : public ExpressionImpl<_Domain> {
   public:
    Exp(Expression<_Domain> expr) : expr(std::move(expr)) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        return _Domain(std::exp(expr.eval(variables)));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return expr * expr.diff(variable);
    };

    virtual std::string to_string() const override {
        return "exp(" + expr.to_string() + ")";
    }

   private:
    Expression<_Domain> expr;
};

template <Numeric _Domain>
template <Numeric T>
Expression<_Domain>::Expression(T value)
    : impl(std::make_shared<Value<_Domain>>(static_cast<_Domain>(value))) {}

template <Numeric _Domain>
Expression<_Domain>::Expression(const std::string& variable)
    : impl(std::make_shared<Variable<_Domain>>(variable)) {}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::operator+(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(valueLhsPtr->getValue() + valueRhsPtr->getValue());
    }
    if (valueLhsPtr && valueLhsPtr->getValue() == _Domain(0)) {
        return other;
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(0)) {
        return *this;
    }

    return Expression(std::make_shared<Add<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::operator-(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(valueLhsPtr->getValue() - valueRhsPtr->getValue());
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(0)) {
        return *this;
    }

    return Expression(std::make_shared<Subtract<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::operator*(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(valueLhsPtr->getValue() * valueRhsPtr->getValue());
    }
    if (valueLhsPtr && valueLhsPtr->getValue() == _Domain(1)) {
        return other;
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(1)) {
        return *this;
    }
    if ((valueLhsPtr && valueLhsPtr->getValue() == _Domain(0)) ||
        (valueRhsPtr && valueRhsPtr->getValue() == _Domain(0))) {
        return Expression<_Domain>(0);
    }

    return Expression(std::make_shared<Multiply<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::operator/(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(0)) {
        throw std::runtime_error("Division by zero");
    }
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(valueLhsPtr->getValue() / valueRhsPtr->getValue());
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(1)) {
        return *this;
    }
    if (valueLhsPtr && valueLhsPtr->getValue() == _Domain(0)) {
        return Expression<_Domain>(0);
    }

    return Expression(std::make_shared<Divide<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::pow(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(
            std::pow(valueLhsPtr->getValue(), valueRhsPtr->getValue()));
    }
    if (valueLhsPtr && valueLhsPtr->getValue() == _Domain(0)) {
        return Expression<_Domain>(1);
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == _Domain(1)) {
        return *this;
    }

    return Expression(std::make_shared<Power<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::sin() const {
    auto valuePtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    if (valuePtr) {
        return Expression(std::sin(valuePtr->getValue()));
    }
    return Expression(std::make_shared<Sin<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::cos() const {
    auto valuePtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    if (valuePtr) {
        return Expression(std::cos(valuePtr->getValue()));
    }
    return Expression(std::make_shared<Cos<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::ln() const {
    auto valuePtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    if (valuePtr) {
        return Expression(std::log(valuePtr->getValue()));
    }
    return Expression(std::make_shared<Ln<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::exp() const {
    auto valuePtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    if (valuePtr) {
        return Expression(std::exp(valuePtr->getValue()));
    }
    return Expression(std::make_shared<Exp<_Domain>>(*this));
}

template <typename T>
auto sin(const T& expr) {
    return Expression(expr).sin();
}

template <typename T>
auto cos(const T& expr) {
    return Expression(expr).cos();
}

template <typename T>
auto ln(const T& expr) {
    return Expression(expr).ln();
}

template <typename T>
auto exp(const T& expr) {
    return Expression(expr).exp();
}

template <Numeric _Domain = Reals_t>
Expression<_Domain> parse_expression(const std::string& expr) {
    std::stack<Expression<_Domain>> values;
    std::stack<char> ops;

    std::unordered_map<char, int> precedence = {
        {'+', 1}, {'-', 1}, {'*', 2}, {'/', 2}, {'^', 3}};

    std::unordered_map<std::string,
                       std::function<Expression<_Domain>(Expression<_Domain>)>>
        functions = {
            {"sin", [](Expression<_Domain> arg) { return arg.sin(); }},
            {"cos", [](Expression<_Domain> arg) { return arg.cos(); }},
            {"ln", [](Expression<_Domain> arg) { return arg.ln(); }},
            {"exp", [](Expression<_Domain> arg) { return arg.exp(); }}};

    bool expect_operand = true;

    for (size_t i = 0; i < expr.length(); ++i) {
        if (std::isspace(expr[i])) {
            continue;
        }

        if (expr[i] == '-' && expect_operand) {
            values.push(Expression<_Domain>(-1));
            ops.push('*');
            continue;
        }

        if (std::isdigit(expr[i]) || expr[i] == '.') {
            std::string num_str;
            while (i < expr.length() &&
                   (std::isdigit(expr[i]) || expr[i] == '.')) {
                num_str += expr[i++];
            }
            --i;
            values.push(Expression<_Domain>(std::stold(num_str)));

            expect_operand = false;
        } else if (std::isalpha(expr[i])) {
            std::string token;
            while (i < expr.length() && std::isalpha(expr[i])) {
                token += expr[i++];
            }
            --i;

            if (functions.find(token) != functions.end()) {
                if (i + 1 < expr.length() && expr[i + 1] == '(') {
                    i++;
                    std::string arg_expr;
                    int brace_count = 1;
                    while (i + 1 < expr.length() && brace_count > 0) {
                        i++;
                        if (expr[i] == '(') brace_count++;
                        if (expr[i] == ')') brace_count--;
                        if (brace_count > 0) arg_expr += expr[i];
                    }
                    auto arg = parse_expression<_Domain>(arg_expr);

                    if (!expect_operand) {
                        ops.push('*');
                    }

                    values.push(functions[token](arg));
                } else {
                    throw std::runtime_error(
                        "Expected '(' after function name");
                }
            } else {
                if (!expect_operand) {
                    ops.push('*');
                }
                values.push(Expression<_Domain>(token));
            }

            expect_operand = false;
        } else if (expr[i] == '(') {
            if (!expect_operand) {
                ops.push('*');
            }
            ops.push(expr[i]);
            expect_operand = true;
        } else if (expr[i] == ')') {
            while (!ops.empty() && ops.top() != '(') {
                char op = ops.top();
                ops.pop();

                Expression<_Domain> rhs = values.top();
                values.pop();
                Expression<_Domain> lhs = values.top();
                values.pop();

                if (op == '+')
                    values.push(lhs + rhs);
                else if (op == '-')
                    values.push(lhs - rhs);
                else if (op == '*')
                    values.push(lhs * rhs);
                else if (op == '/')
                    values.push(lhs / rhs);
                else if (op == '^')
                    values.push(lhs.pow(rhs));
            }
            ops.pop();

            expect_operand = false;
        } else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' ||
                   expr[i] == '/' || expr[i] == '^') {
            while (!ops.empty() && ops.top() != '(' &&
                   precedence[ops.top()] >= precedence[expr[i]]) {
                char op = ops.top();
                ops.pop();

                Expression<_Domain> rhs = values.top();
                values.pop();
                Expression<_Domain> lhs = values.top();
                values.pop();

                if (op == '+')
                    values.push(lhs + rhs);
                else if (op == '-')
                    values.push(lhs - rhs);
                else if (op == '*')
                    values.push(lhs * rhs);
                else if (op == '/')
                    values.push(lhs / rhs);
                else if (op == '^')
                    values.push(lhs.pow(rhs));
            }
            ops.push(expr[i]);
            expect_operand = true;
        }
    }

    while (!ops.empty()) {
        char op = ops.top();
        ops.pop();

        Expression<_Domain> rhs = values.top();
        values.pop();
        Expression<_Domain> lhs = values.top();
        values.pop();

        if (op == '+')
            values.push(lhs + rhs);
        else if (op == '-')
            values.push(lhs - rhs);
        else if (op == '*')
            values.push(lhs * rhs);
        else if (op == '/')
            values.push(lhs / rhs);
        else if (op == '^')
            values.push(lhs.pow(rhs));
    }

    return values.top();
}

};  // namespace symcpp

#endif  // EXPRESSION_HPP
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using Reals_t = long double;
class Complexes_t : public std::complex<Reals_t> {
   public:
    using std::complex<Reals_t>::complex;
    operator long double() const { return static_cast<Reals_t>(this->real()); }
};
template <typename T>
concept Numeric = std::is_arithmetic_v<T> || (std::is_same_v<T, Complexes_t>);

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

template <Numeric _Domain = Reals_t>
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
        return std::to_string(value);
    }

    _Domain getValue() const { return value; }

   private:
    _Domain value;
};

template <Numeric _Domain = Reals_t>
class Variable : public ExpressionImpl<_Domain> {
   public:
    Variable(std::string variable) : variable(variable) {}

    virtual _Domain eval(
        const std::map<std::string, _Domain>& variables) const override {
        auto it = variables.find(variable);
        if (it != variables.end()) {
            return it->second;
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
        return lhs.eval(variables) / rhs.eval(variables);
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
        return rhs * (lhs.pow(rhs - 1)) * lhs.diff(variable);
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
        return std::sin(expr.eval(variables));
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
        return std::cos(expr.eval(variables));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return Expression(-1) * expr.sin() * expr.diff(variable);
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
        return std::log(expr.eval(variables));
    }

    virtual Expression<_Domain> diff(
        const std::string& variable) const override {
        return Expression(1) / expr * expr.diff(variable);
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
        return std::exp(expr.eval(variables));
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
    if (valueLhsPtr && valueLhsPtr->getValue() == 0) {
        return other;
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == 0) {
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
    if (valueRhsPtr && valueRhsPtr->getValue() == 0) {
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
    if (valueLhsPtr && valueLhsPtr->getValue() == 1) {
        return other;
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == 1) {
        return *this;
    }
    if ((valueLhsPtr && valueLhsPtr->getValue() == 0) ||
        (valueRhsPtr && valueRhsPtr->getValue() == 0)) {
        return Expression(0);
    }

    return Expression(std::make_shared<Multiply<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::operator/(
    const Expression<_Domain>& other) const {
    auto valueLhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(this->impl);
    auto valueRhsPtr = std::dynamic_pointer_cast<Value<_Domain>>(other.impl);
    if (valueLhsPtr && valueRhsPtr) {
        return Expression(valueLhsPtr->getValue() / valueRhsPtr->getValue());
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == 1) {
        return *this;
    }
    if (valueLhsPtr && valueLhsPtr->getValue() == 0) {
        return Expression(0);
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
    if (valueLhsPtr && valueLhsPtr->getValue() == 0) {
        return Expression(1);
    }
    if (valueRhsPtr && valueRhsPtr->getValue() == 1) {
        return *this;
    }

    return Expression(std::make_shared<Power<_Domain>>(*this, other));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::sin() const {
    return Expression(std::make_shared<Sin<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::cos() const {
    return Expression(std::make_shared<Cos<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::ln() const {
    return Expression(std::make_shared<Ln<_Domain>>(*this));
}

template <Numeric _Domain>
Expression<_Domain> Expression<_Domain>::exp() const {
    return Expression(std::make_shared<Exp<_Domain>>(*this));
}

#endif  // EXPRESSION_HPP
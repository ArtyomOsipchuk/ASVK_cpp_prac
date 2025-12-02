#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>

class TFunction;
using TFunctionPtr = std::shared_ptr<TFunction>;

class TFunction {
public:
    virtual ~TFunction() = default;
    
    virtual double operator()(double x) const = 0;
    
    virtual double GetDeriv(double x) const = 0;
    
    virtual std::string ToString() const = 0;
    
    virtual TFunctionPtr Clone() const = 0;
};

class UnsupportedOperation : public std::logic_error {
public:
    explicit UnsupportedOperation(const std::string& what)
        : std::logic_error(what) {}
};

class FunctionFactory {
public:
    static TFunctionPtr Create(const std::string& type, const std::vector<double>& params = {});
};

class IdentityFunction : public TFunction {
public:
    double operator()(double x) const override { return x; }
    double GetDeriv(double) const override { return 1.0; }
    std::string ToString() const override { return "IdentityFunc x"; }
    TFunctionPtr Clone() const override { return std::make_shared<IdentityFunction>(); }
};

class ConstFunction : public TFunction {
    double value_;
public:
    explicit ConstFunction(double value) : value_(value) {}
    double operator()(double) const override { return value_; }
    double GetDeriv(double) const override { return 0.0; }
    std::string ToString() const override { return "Const " + std::to_string(value_); }
    TFunctionPtr Clone() const override { return std::make_shared<ConstFunction>(value_); }
};

class PowerFunction : public TFunction {
    int power_;
public:
    explicit PowerFunction(int power) : power_(power) {}
    double operator()(double x) const override { return std::pow(x, power_); }
    double GetDeriv(double x) const override { 
        if (power_ == 0) return 0.0;
        return power_ * std::pow(x, power_ - 1);
    }
    std::string ToString() const override { 
        return "PowerFunc x^" + std::to_string(power_); 
    }
    TFunctionPtr Clone() const override { return std::make_shared<PowerFunction>(power_); }
};

class ExpFunction : public TFunction {
public:
    double operator()(double x) const override { return std::exp(x); }
    double GetDeriv(double x) const override { return std::exp(x); }
    std::string ToString() const override { return "ExpFunc exp(x)"; }
    TFunctionPtr Clone() const override { return std::make_shared<ExpFunction>(); }
};

class PolynomialFunction : public TFunction {
    std::vector<double> coeffs_;
public:
    explicit PolynomialFunction(const std::vector<double>& coeffs) : coeffs_(coeffs) {}
    
    double operator()(double x) const override {
        double result = 0;
        double x_power = 1;
        for (double coeff : coeffs_) {
            result += coeff * x_power;
            x_power *= x;
        }
        return result;
    }
    
    double GetDeriv(double x) const override {
        double result = 0;
        double x_power = 1;
        for (size_t i = 1; i < coeffs_.size(); ++i) {
            result += coeffs_[i] * i * x_power;
            x_power *= x;
        }
        return result;
    }
    
    std::string ToString() const override {
        std::string result;
        result += "TPolynomial ";
        for (size_t i = 0; i < coeffs_.size(); ++i) {
            if (coeffs_[i]) {
                if (i > 0) {
                    result += " + ";
                    result += std::to_string(std::round(coeffs_[i])) + "*x";
                    if (i > 1) {
                        result += "^" + std::to_string(i);
                    }
                } else {
                    result += std::to_string(coeffs_[i]);
                }
            }
        }
        return result;
    }
    
    TFunctionPtr Clone() const override { 
        return std::make_shared<PolynomialFunction>(coeffs_); 
    }
};

class SumFunction : public TFunction {
    TFunctionPtr left_, right_;
public:
    SumFunction(TFunctionPtr left, TFunctionPtr right) : left_(left), right_(right) {}
    
    double operator()(double x) const override {
        return (*left_)(x) + (*right_)(x);
    }
    
    double GetDeriv(double x) const override {
        return left_->GetDeriv(x) + right_->GetDeriv(x);
    }
    
    std::string ToString() const override {
        return "(" + left_->ToString() + " + " + right_->ToString() + ")";
    }
    
    TFunctionPtr Clone() const override {
        return std::make_shared<SumFunction>(left_, right_);
    }
};

class DifferenceFunction : public TFunction {
    TFunctionPtr left_, right_;
public:
    DifferenceFunction(TFunctionPtr left, TFunctionPtr right) : left_(left), right_(right) {}
    
    double operator()(double x) const override {
        return (*left_)(x) - (*right_)(x);
    }
    
    double GetDeriv(double x) const override {
        return left_->GetDeriv(x) - right_->GetDeriv(x);
    }
    
    std::string ToString() const override {
        return "(" + left_->ToString() + " - " + right_->ToString() + ")";
    }
    
    TFunctionPtr Clone() const override {
        return std::make_shared<DifferenceFunction>(left_, right_);
    }
};

class ProductFunction : public TFunction {
    TFunctionPtr left_, right_;
public:
    ProductFunction(TFunctionPtr left, TFunctionPtr right) : left_(left), right_(right) {}
    
    double operator()(double x) const override {
        return (*left_)(x) * (*right_)(x);
    }
    
    double GetDeriv(double x) const override {
        return left_->GetDeriv(x) * (*right_)(x) + (*left_)(x) * right_->GetDeriv(x);
    }
    
    std::string ToString() const override {
        return "ProductFunc (" + left_->ToString() + " * " + right_->ToString() + ")";
    }
    
    TFunctionPtr Clone() const override {
        return std::make_shared<ProductFunction>(left_, right_);
    }
};

class QuotientFunction : public TFunction {
    TFunctionPtr left_, right_;
public:
    QuotientFunction(TFunctionPtr left, TFunctionPtr right) : left_(left), right_(right) {}
    
    double operator()(double x) const override {
        double denominator = (*right_)(x);
        if (std::abs(denominator) < 1e-12) {
            throw std::logic_error("Division by zero");
        }
        return (*left_)(x) / denominator;
    }
    
    double GetDeriv(double x) const override {
        double f = (*left_)(x);
        double g = (*right_)(x);
        double f_prime = left_->GetDeriv(x);
        double g_prime = right_->GetDeriv(x);
        
        if (std::abs(g) < 1e-12) {
            throw std::logic_error("Division by zero in derivative");
        }
        
        return (f_prime * g - f * g_prime) / (g * g);
    }
    
    std::string ToString() const override {
        return "(" + left_->ToString() + " / " + right_->ToString() + ")";
    }
    
    TFunctionPtr Clone() const override {
        return std::make_shared<QuotientFunction>(left_, right_);
    }
};

TFunctionPtr FunctionFactory::Create(const std::string& type, const std::vector<double>& params) {
    if (type == "ident") {
        return std::make_shared<IdentityFunction>();
    } else if (type == "const") {
        if (params.size() < 1) throw std::invalid_argument("Constant value required");
        return std::make_shared<ConstFunction>(params[0]);
    } else if (type == "power") {
        if (params.size() < 1) throw std::invalid_argument("Power exponent required");
        return std::make_shared<PowerFunction>(static_cast<int>(params[0]));
    } else if (type == "exp") {
        return std::make_shared<ExpFunction>();
    } else if (type == "polynomial") {
        if (params.empty()) throw std::invalid_argument("Polynomial coefficients required");
        return std::make_shared<PolynomialFunction>(params);
    }
    throw std::invalid_argument("Unknown function type: " + type);
}

TFunctionPtr operator+(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<SumFunction>(lhs.Clone(), rhs.Clone());
}

template <typename T>
TFunctionPtr operator+(const TFunction&, const T&) {
    throw std::logic_error("Unsupported type for operator+");
}

template <typename T>
TFunctionPtr operator+(const T&, const TFunction& rhs) {
        throw std::logic_error("Unsupported type for operator+");
}

TFunctionPtr operator-(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<DifferenceFunction>(lhs.Clone(), rhs.Clone());
}

template <typename T>
TFunctionPtr operator-(const TFunction&, const T&) {
    throw std::logic_error("Unsupported type for operator-");
}

template <typename T>
TFunctionPtr operator-(const T&, const TFunction&) {
        throw std::logic_error("Unsupported type for operator-");
}

TFunctionPtr operator*(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<ProductFunction>(lhs.Clone(), rhs.Clone());
}

template <typename T>
TFunctionPtr operator*(const TFunction&, const T&) {
    throw std::logic_error("Unsupported type for operator*");
}

template <typename T>
TFunctionPtr operator*(const T&, const TFunction&) {
        throw std::logic_error("Unsupported type for operator*");
}

TFunctionPtr operator/(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<QuotientFunction>(lhs.Clone(), rhs.Clone());
}

template <typename T>
TFunctionPtr operator/(const TFunction&, const T&) {
    throw std::logic_error("Unsupported type for operator/");
}

template <typename T>
TFunctionPtr operator/(const T&, const TFunction&) {
        throw std::logic_error("Unsupported type for operator/");
}

double GradientDescentRoot(TFunctionPtr func, double initial_guess, int iterations, double learning_rate = 0.1) {
    double x = initial_guess;
    for (int i = 0; i < iterations; ++i) {
        double dfx = func->GetDeriv(x);
        double f = (*func)(x);
        std::cout << x << std::endl;
        x = x - learning_rate * (f / dfx);
    }
    return x;
}

#endif
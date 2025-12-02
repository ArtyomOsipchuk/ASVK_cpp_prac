#include <gtest/gtest.h>
#include "function.h"

TEST(FunctionTest, IdentityFunction) {
    IdentityFunction f;
    EXPECT_DOUBLE_EQ(f(5.0), 5.0);
    EXPECT_DOUBLE_EQ(f.GetDeriv(5.0), 1.0);
    EXPECT_EQ(f.ToString(), "IdentityFunc x");
}

TEST(FunctionTest, ConstFunction) {
    ConstFunction f(3.14);
    EXPECT_DOUBLE_EQ(f(5.0), 3.14);
    EXPECT_DOUBLE_EQ(f.GetDeriv(5.0), 0.0);
}

TEST(FunctionTest, PowerFunction) {
    PowerFunction f(3); // x^3
    EXPECT_DOUBLE_EQ(f(2.0), 8.0);
    EXPECT_DOUBLE_EQ(f.GetDeriv(2.0), 12.0); // 3*x^2 = 3*4 = 12
}

TEST(FunctionTest, PolynomialFunction) {
    PolynomialFunction f({1, 2, 3}); // 1 + 2x + 3x^2
    EXPECT_DOUBLE_EQ(f(2.0), 1 + 4 + 12); // 17
    EXPECT_DOUBLE_EQ(f.GetDeriv(2.0), 2 + 12); // 2 + 6x = 2 + 12 = 14
}

TEST(FunctionTest, SumFunction) {
    auto f = std::make_shared<ConstFunction>(3.0);
    auto g = std::make_shared<IdentityFunction>();
    SumFunction h(f, g);
    
    EXPECT_DOUBLE_EQ(h(2.0), 5.0);
    EXPECT_DOUBLE_EQ(h.GetDeriv(2.0), 1.0);
}

TEST(FunctionTest, ProductFunction) {
    auto f = std::make_shared<ConstFunction>(3.0);
    auto g = std::make_shared<IdentityFunction>();
    ProductFunction h(f, g); // 3x
    
    EXPECT_DOUBLE_EQ(h(2.0), 6.0);
    EXPECT_DOUBLE_EQ(h.GetDeriv(2.0), 3.0);
}

TEST(FunctionTest, Factory) {
    auto f = FunctionFactory::Create("ident");
    EXPECT_DOUBLE_EQ((*f)(5.0), 5.0);
    
    auto g = FunctionFactory::Create("const", {3.14});
    EXPECT_DOUBLE_EQ((*g)(5.0), 3.14);
    
    auto h = FunctionFactory::Create("power", {2});
    EXPECT_DOUBLE_EQ((*h)(3.0), 9.0);
}

// TEST(FunctionTest, GradientDescent) { // по ТЗ можно не проверять :)
//     auto f = FunctionFactory::Create("polynomial", {-4, 0, 1}); // x^2 - 4
//     double root = GradientDescentRoot(f, 3.0, 10);
    
//     EXPECT_NEAR(root, 2.0, 0.001);
//     EXPECT_NEAR((*f)(root), 0.0, 0.001);
// }

TEST(FunctionTest, Operators) {
    auto f = FunctionFactory::Create("power", {2}); // x^2
    auto g = FunctionFactory::Create("const", {3}); // 3
    
    auto sum = *f + *g; // x^2 + 3
    EXPECT_DOUBLE_EQ((*sum)(2.0), 7.0);
    
    auto prod = *f * *g; // 3x^2
    EXPECT_DOUBLE_EQ((*prod)(2.0), 12.0);
}

TEST(FunctionTest, Error) {
    auto f = FunctionFactory::Create("power", {2}); // x^2
    EXPECT_THROW({ auto sum = *f + "abc"; }, std::logic_error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
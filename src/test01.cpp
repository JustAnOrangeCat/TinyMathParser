#include <iostream>

#include "TinyMathParser.h"

int main()
{
    std::string expression = "sin(x)";
    expression += ' '; // to add the last input.

    tmp::Compiler compiler;
    auto vecTokens = compiler.Parse(expression);
    for (const auto &tokens : vecTokens)
        std::cout << tokens.str() << '\n';

    return 0;
}

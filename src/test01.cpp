#include <iostream>
#include "TinyMathParser.h"

int main()
{
    std::string expression = "";
    expression += ' '; // to add the last input.

    tmp::Compiler compiler;
    auto vecTokens = compiler.Parse(expression);

    std::cout << expression << '\n';

    for (const auto &tokens : vecTokens)
        std::cout << tokens.str() << '\n';

    /// compiler.setVariableValue(vecTokens[2], 60);

    std::cout
        << '\n'
        << compiler.Evaluate(vecTokens);
    return 0;
}

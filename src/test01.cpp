#include <iostream>
#include "TinyMathParser.h"

int main()
{
    std::string expression = "x^2";
    expression += ' '; // to add the last input.

    try
    {
        tmp::Compiler compiler;
        auto vecTokens = compiler.Parse(expression);

        compiler.setVariableValue(vecTokens, "x", 10);

        std::cout
            << '\n'
            << compiler.Evaluate(vecTokens) << '\n';
    }
    catch (tmp::CompileError &e)
    {
        std::cout << e.what() << '\n';
    }

    return 0;
}

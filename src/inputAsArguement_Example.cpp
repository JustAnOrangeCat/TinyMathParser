#include <iostream>
#include "TinyMathParser.h"

int main(int argc, char *argv[])
{
    std::string expression = argv[1];
    expression += ' '; // to add the last input.

    tmp::Compiler compiler;                      // Creatign compiler
    auto vecTokens = compiler.Parse(expression); // Parsing expression

    std::cout << expression << '\n';
    for (auto tok : vecTokens)
    {
        std::cout << tok.str() << '\n'; // tok.str() -- Outputs the correspoinding token and character
    }

    std::cout << "Result = ";
    std::cout << compiler.Evaluate(vecTokens); // Evaluating result

    return 0;
}

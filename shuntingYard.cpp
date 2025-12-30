#include <iostream>

int main(int argv, char **argc)
{

    if (argv <= 1)
    {
        std::cout << "PLEASE ENTER EXPRESSION\n";
        return 0;
    }

    std::cout << "input: " << argc[1] << '\n';
    std::string expression = argc[1]; // simple expression

    struct Symbol // THINGS TO HOLD
    {
        std::string symbol = "";

        enum class Type : uint8_t
        {
            Unknown,
            Literal_Numeric,
            Operator,
            Parenthesis_Open,
            Parenthesis_Close,
        } type = Type::Unknown;

        Operator op;
    };

    // HOLDERS
    std::deque<Symbol> holding_stack;
    std::deque<Symbol> output_stack;

    Symbol previousSymbol = {"0", Symbol::Type::Literal_Numeric, 0, 0};

    // SHUNTING YARD ALGORITHM
    for (char c : expression)
    {
        // only single digits, only single digit operators
        if (std::isdigit(c))
        {
            // push literal straight to ouput, ther are already in order
            output_stack.push_back({std::string(1, c), Symbol::Type::Literal_Numeric});
            previousSymbol = output_stack.back();
        }

        else if (c == '(')
        {
            // push openPara to the front of the holding stack
            holding_stack.push_front({std::string(1, c), Symbol::Type::Parenthesis_Open});
            previousSymbol = holding_stack.front();
        }

        else if (c == ')')
        {
            // flushing the holding stack until we reach openPara
            while (!holding_stack.empty() && holding_stack.front().type != Symbol::Type::Parenthesis_Open)
            {
                output_stack.push_back(holding_stack.front());
                holding_stack.pop_front();
            }
            if (holding_stack.empty())
            {
                std::cout << "ERROR:Unexpected_Parenthesis " << std::string(1, c);
                return 0;
            }

            // remove the corresponding openPara from holding stack
            if (!holding_stack.empty() && holding_stack.front().type == Symbol::Type::Parenthesis_Open)
            {
                holding_stack.pop_front();
            }
            previousSymbol = {std::string(1, c), Symbol::Type::Parenthesis_Close};
        }

        else if (mapOperators.contains(c))
        {
            // symbol is operator
            Operator new_op = mapOperators[c];

            // Unary Operators
            if (c == '-' | c == '+')
            {
                if (previousSymbol.type != Symbol::Type::Literal_Numeric && previousSymbol.type != Symbol::Type::Parenthesis_Close)
                {
                    new_op.arguements = 1;
                    new_op.precedence = 100;
                }
            }

            // Checking precedence of operator already there...
            while (!holding_stack.empty() && holding_stack.front().type != Symbol::Type::Parenthesis_Open)
            {
                if (holding_stack.front().type == Symbol::Type::Operator) // Checking if the first element is indeed an operator
                {
                    const auto &holding_stack_op = holding_stack.front().op;

                    // Precedence check
                    if (holding_stack_op.precedence >= new_op.precedence)
                    {
                        output_stack.push_back(holding_stack.front());
                        holding_stack.pop_front();
                    }
                    else
                        break;
                }
            }

            // push new operator onto the holding stack
            holding_stack.push_front({std::string(1, c), Symbol::Type::Operator, new_op});

            previousSymbol = holding_stack.front();
        }
        else
        {
            std::cout << "BAD_SYMBOL: " << std::string(1, c) << '\n';
            return 0;
        }
    }

    // Draining the holding stack
    while (!holding_stack.empty())
    {
        output_stack.push_back(holding_stack.front());
        holding_stack.pop_front();
    }

    // quick TEST
    std::cout << "EXPRESSION: " << expression << '\n';
    std::cout << "RPN: ";
    for (const auto &s : output_stack)
    {
        std::cout << s.symbol << " ";
    }
    std::cout << '\n';

    // SOLVER
    std::deque<double> solving_stack;

    for (const auto &inst : output_stack)
    {
        switch (inst.type)
        {
        case Symbol::Type::Literal_Numeric:
            solving_stack.push_front(std::stod(inst.symbol));
            break;
        case Symbol::Type::Operator:
            std::vector<double> mem(inst.op.arguements);
            for (uint8_t a = 0; a < inst.op.arguements; a++)
            {
                if (solving_stack.empty())
                    std::cout << "ERROR::BAD_EXPRESSION\n";
                else
                {
                    mem[a] = solving_stack[0];
                    solving_stack.pop_front();
                }
            }
            double result = 0.0;
            // Binary Operators
            if (inst.op.arguements == 2)
            {
                if (inst.symbol[0] == '/')
                    result = mem[1] / mem[0];

                if (inst.symbol[0] == '*')
                    result = mem[1] * mem[0];

                if (inst.symbol[0] == '+')
                    result = mem[1] + mem[0];

                if (inst.symbol[0] == '-')
                    result = mem[1] - mem[0];
            }
            // Unary Operators
            if (inst.op.arguements == 1)
            {
                if (inst.symbol[0] == '+')
                    result = +mem[0];
                if (inst.symbol[0] == '-')
                    result = -mem[0];
            }

            solving_stack.push_front(result);
            break;
        }
    }

    std::cout << "RESULT    = " << std::to_string(solving_stack[0]) << '\n';

    return 0;
}
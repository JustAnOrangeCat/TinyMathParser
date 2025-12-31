#ifndef TINY_MATH_PARSER_H
#define TINY_MATH_PARSER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <variant>
#include <deque>

#include <cmath>

// LOOK UP TABLE
namespace lut
{
    static constexpr auto MakeLUT(const std::string_view AcceptedCharacters)
    {
        std::array<bool, 256> lut{0};
        for (const auto c : AcceptedCharacters)
            lut.at(uint8_t(c)) = true;
        return lut;
    };

    constexpr auto WhiteSpaceDigits = MakeLUT(" \t\r\n\v\f");
    constexpr auto NumericDigits = MakeLUT("0123456789");
    constexpr auto RealNumericDigits = MakeLUT(".0123456789");
    constexpr auto OperatorDigits = MakeLUT("!$%^&*+-=#@?|`/\\<>~");
    constexpr auto Alphabets = MakeLUT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
} // namespace lut

// OPERATOR STRUCT
struct Operator
{
    uint8_t precedence = 0;
    uint8_t arguement = 0;
};

// TOKEN STRUCT
struct Token
{
    enum class Type : uint8_t
    {
        Unknown,
        Literal_Numeric,
        Operator,
        Paranthesis_Open,
        Paranthesis_Close,
        Variable,
        Function,
    } type = Type::Unknown;

    std::string text = "";
    Operator op;
    double value = 0.0;

    std::string str() const
    {
        std::string o;
        switch (type)
        {
        case Token::Type::Unknown:
            o += "[UNKNOWN]";
            break;
        case Token::Type::Literal_Numeric:
            o += "[Literal, Numeric]";
            break;
        case Token::Type::Paranthesis_Open:
            o += "[Paranthesis, Open]";
            break;
        case Token::Type::Paranthesis_Close:
            o += "[Paranthesis, Close]";
            break;
        case Token::Type::Operator:
            o += "[Operator]";
            break;
        case Token::Type::Variable:
            o += "[Variable]";
            break;
        case Token::Type::Function:
            o += "[Function]";
            break;
        }

        o += " : " + text;
        return o;
    }
};

// ERROR CHECKER CLASS
class CompileError : public std::exception
{
public:
    CompileError(const std::string &msg)
    {
        p_message = msg;
    }

    const char *what()
    {
        return p_message.c_str();
    }

private:
    std::string p_message;
};

// COMPILER
namespace tmp
{
    class Compiler
    {
    protected:
        std::unordered_map<std::string, Operator> mapOperators;

        // HOLDERS
        std::deque<Token> holding_stack;
        std::deque<Token> output_stack;
        std::deque<double> solving_stack;

    public:
        Compiler()
        {
            mapOperators["*"] = {3, 2};
            mapOperators["/"] = {3, 2};
            mapOperators["+"] = {1, 2};
            mapOperators["-"] = {1, 2};
        }

        std::vector<Token> Parse(const std::string &input)
        {
            // ERROR HANDLING
            if (input.empty())
            {
                throw CompileError("ERROR::PARSER::NO_INPUT_PROVIDED");
            }

            std::vector<Token> vecOutputTokens;

            // FINITE STATE MACHINE
            enum class TokeniserState : uint8_t
            {
                NewToken,
                Numeric_Literal,
                Parenthesis_Open,
                Parenthesis_Close,
                Operator,
                String_Literal,
                CompleteToken,
            };

            TokeniserState stateNow = TokeniserState::NewToken;
            TokeniserState stateNext = TokeniserState::NewToken;
            std::string sCurrentToken = "";
            Token tokCurrent;

            size_t ParenthesisBalanceChecker = 0;
            auto charNow = input.begin();
            while (charNow != input.end())
            {
                switch (stateNow)
                {
                case TokeniserState::NewToken:
                    // Reset State
                    sCurrentToken.clear();
                    tokCurrent = {Token::Type::Unknown, ""};

                    // First character Analysis

                    // checking white space
                    if (lut::WhiteSpaceDigits.at(charNow[0]))
                    {
                        charNow++;
                        stateNext = TokeniserState::NewToken;
                    }

                    // checking numeric literals
                    else if (lut::NumericDigits.at(charNow[0]))
                    {
                        sCurrentToken = charNow[0];
                        stateNext = TokeniserState::Numeric_Literal; // switch to numeric literal state
                        charNow++;
                    }

                    // checking Operators
                    else if (lut::OperatorDigits.at(charNow[0]))
                    {
                        stateNext = TokeniserState::Operator;
                    }
                    // checking Parenthesis Open
                    else if (charNow[0] == '(')
                    {
                        stateNext = TokeniserState::Parenthesis_Open;
                    }
                    // checking Parenthesis Close
                    else if (charNow[0] == ')')
                    {
                        stateNext = TokeniserState::Parenthesis_Close;
                    }

                    else if (lut::Alphabets.at(charNow[0]))
                    {
                        sCurrentToken = charNow[0];
                        stateNext = TokeniserState::String_Literal;
                        charNow++;
                    }

                    break; // OUT NEW_STATE

                // Numeric_Literal
                case TokeniserState::Numeric_Literal:
                    if (lut::RealNumericDigits.at(charNow[0]))
                    {
                        sCurrentToken += charNow[0];
                        charNow++;
                        stateNext = TokeniserState::Numeric_Literal;
                    }
                    else
                    {
                        stateNext = TokeniserState::CompleteToken;
                        tokCurrent = {Token::Type::Literal_Numeric, sCurrentToken};
                        tokCurrent.value = std::stod(sCurrentToken);
                    }
                    break;
                // Operators
                case TokeniserState::Operator:
                    if (lut::OperatorDigits.at(charNow[0]))
                    {
                        // if this + next is an operator
                        if (mapOperators.contains(sCurrentToken + charNow[0]))
                        {
                            // YES - add the next to this
                            sCurrentToken += charNow[0];
                            charNow++;
                        }
                        else
                        {
                            if (mapOperators.contains(sCurrentToken))
                            {
                                tokCurrent = {Token::Type::Operator, sCurrentToken};
                                tokCurrent.op = mapOperators[sCurrentToken];
                                stateNext = TokeniserState::CompleteToken;
                            }
                            else
                            {
                                sCurrentToken += charNow[0];
                                charNow++;
                            }
                        }
                    }
                    else
                    {
                        if (mapOperators.contains(sCurrentToken))
                        {
                            tokCurrent = {Token::Type::Operator, sCurrentToken};
                            tokCurrent.op = mapOperators[sCurrentToken];
                            stateNext = TokeniserState::CompleteToken;
                        }
                        else
                        {
                            throw CompileError("ERROR::UNRECOGNIZED_OPERATOR");
                        }
                    }
                    break;

                // Parenthesis
                case TokeniserState::Parenthesis_Open:
                    sCurrentToken += charNow[0];
                    charNow++;
                    ParenthesisBalanceChecker++;
                    tokCurrent = {Token::Type::Paranthesis_Open, sCurrentToken};
                    stateNext = TokeniserState::CompleteToken;
                    break;

                case TokeniserState::Parenthesis_Close:
                    sCurrentToken += charNow[0];
                    charNow++;
                    ParenthesisBalanceChecker--;
                    tokCurrent = {Token::Type::Paranthesis_Close, sCurrentToken};
                    stateNext = TokeniserState::CompleteToken;
                    break;

                // String
                case TokeniserState::String_Literal:
                    if (lut::Alphabets.at(charNow[0]))
                    {
                        sCurrentToken += charNow[0];
                        charNow++;
                    }
                    else
                    {
                        if (sCurrentToken.size() == 1)
                        {
                            tokCurrent = {Token::Type::Variable, sCurrentToken};
                            stateNext = TokeniserState::CompleteToken;
                        }
                        else
                        {
                            tokCurrent = {Token::Type::Function, sCurrentToken};
                            stateNext = TokeniserState::CompleteToken;
                        }
                    }

                    break;
                // Completed
                case TokeniserState::CompleteToken:
                    vecOutputTokens.push_back(tokCurrent);
                    stateNext = TokeniserState::NewToken;
                    break;
                }

                stateNow = stateNext;
            }

            if (ParenthesisBalanceChecker != 0)
            {
                throw CompileError("ERROR::UNBALANCED_PAREN");
            }

            return vecOutputTokens;
        }

        std::string Evaluate(std::vector<Token> inputExpression)
        {
            for (auto tok : inputExpression)
            {
                // Literal_Numeric
                if (tok.type == Token::Type::Literal_Numeric)
                {
                    output_stack.push_back(tok);
                }
                // Operator
                else if (tok.type == Token::Type::Operator)
                {

                    // Unary Operators // TODO
                    // if (c == '-' | c == '+')
                    // {
                    //     if (previousSymbol.type != Symbol::Type::Literal_Numeric && previousSymbol.type != Symbol::Type::Parenthesis_Close)
                    //     {
                    //         new_op.arguements = 1;
                    //         new_op.precedence = 100;
                    //     }
                    // }

                    // Checking precedence of operator already there...
                    while (!holding_stack.empty() && holding_stack.front().type != Token::Type::Paranthesis_Open)
                    {
                        if (holding_stack.front().type == Token::Type::Operator) // Checking if the first element is indeed an operator
                        {
                            const auto &holding_stack_op = holding_stack.front().op;
                            // Precedence check
                            if (holding_stack_op.precedence >= tok.op.precedence)
                            {
                                output_stack.push_back(holding_stack.front());
                                holding_stack.pop_front();
                            }
                            else
                                break;
                        }
                        else
                            throw CompileError("ERROR::UNKNOWN_OPERATOR_FOUND");
                    }
                    holding_stack.push_front(tok);
                }
                // Open_Para
                else if (tok.type == Token::Type::Paranthesis_Open)
                {
                    holding_stack.push_front(tok);
                }
                // Close_Para
                else if (tok.type == Token::Type::Paranthesis_Close)
                {
                    // flushing the holding stack until we reach openPara
                    while (!holding_stack.empty() && holding_stack.front().type != Token::Type::Paranthesis_Open)
                    {
                        output_stack.push_back(holding_stack.front());
                        holding_stack.pop_front();
                    }
                    // holding stack becomes empty :: error
                    if (holding_stack.empty())
                    {
                        throw CompileError("ERROR::UNEXPECTED_PARANTHESIS");
                    }
                    // if holding stack has parenthesis
                    // remove the corresponding openPara from holding stack
                    if (!holding_stack.empty() && holding_stack.front().type == Token::Type::Paranthesis_Open)
                    {
                        holding_stack.pop_front();
                    }
                }
                // Variable
                else if (tok.type == Token::Type::Variable)
                {
                }
                // Function
                else if (tok.type == Token::Type::Function)
                {
                    holding_stack.push_front(tok);
                }
                // Unknown
                else
                {
                    throw CompileError("ERROR::BAD_SYMBOL");
                }
            }
            // Draining the holding stack
            while (!holding_stack.empty())
            {
                output_stack.push_back(holding_stack.front());
                holding_stack.pop_front();
            }

            // quick TEST -- printing RPN
            // std::cout << "\nRPN: ";
            // for (const auto &s : output_stack)
            // {
            //     std::cout << s.text << " ";
            // }
            // std::cout << '\n';

            for (const auto &inst : output_stack)
            {
                switch (inst.type)
                {
                case Token::Type::Literal_Numeric:
                    solving_stack.push_front(inst.value);
                    break;

                case Token::Type::Function:
                {
                    double result = 0.0;
                    if (inst.text == "sin")
                    {
                        result += std::sin(solving_stack[0]);
                        solving_stack.pop_front();
                    }
                    solving_stack.push_front(result);
                    break;
                }
                case Token::Type::Operator:
                    std::vector<double> mem(inst.op.arguement);
                    for (uint8_t a = 0; a < inst.op.arguement; a++)
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
                    if (inst.op.arguement == 2)
                    {
                        if (inst.text == "/")
                            result = mem[1] / mem[0];

                        if (inst.text == "*")
                            result = mem[1] * mem[0];

                        if (inst.text == "+")
                            result = mem[1] + mem[0];

                        if (inst.text == "-")
                            result = mem[1] - mem[0];
                    }
                    // Unary Operators
                    if (inst.op.arguement == 1)
                    {
                        if (inst.text == "+")
                            result = +mem[0];
                        if (inst.text == "-")
                            result = -mem[0];
                    }

                    solving_stack.push_front(result);
                    break;
                }
            }
            return std::to_string(solving_stack[0]);
        }

        void setVariableValue(Token &variable, double value)
        {
            if (variable.type == Token::Type::Variable)
            {
                variable.value = value;
                variable.type = Token::Type::Literal_Numeric;
            }
            else
                throw CompileError("ERROR::UNRECOGNIZED_TOKEN::NOT_VARIABLE");
        }
    };
} // namespace tmp

#endif
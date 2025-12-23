#ifndef TINY_MATH_PARSER_H
#define TINY_MATH_PARSER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <variant>
#include <deque>

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

// OPERATOR STRUCT
struct Operator
{
    uint8_t precedence = 0;
    uint8_t atgurement = 0;
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
    };
} // namespace tmp

#endif
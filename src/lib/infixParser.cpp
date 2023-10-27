#include <sstream>
#include <stdexcept>
#include "infixParser.h"

std::map<std::string, double> symbolTable;

Assignment::Assignment(const std::string& varName, ASTNode* expression)
    : variableName(varName), expression(expression) {}


double Assignment::evaluate(std::map<std::string, double>& symbolTable) const {
    double result = expression->evaluate(symbolTable);
    symbolTable[variableName] = result;
    return result;   
}

double Variable::evaluate(std::map<std::string, double>& symbolTable) const {
    if (symbolTable.find(variableName) != symbolTable.end()) {
            return symbolTable.at(variableName);
        } else {
            throw UnknownIdentifierException(symbolTable, variableName);
        }
}

std::string Assignment::toInfix() const {
    return "(" + variableName + " = " + expression->toInfix() + ")";
}

double BinaryOperation::evaluate(std::map<std::string, double>& symbolTable) const {
    double leftValue = left->evaluate(symbolTable);
    double rightValue = right->evaluate(symbolTable);

    switch (op) {
        case '+': return leftValue + rightValue;
        case '-': return leftValue - rightValue;
        case '*': return leftValue * rightValue;
        case '/':
            if (rightValue == 0) {
                throw DivisionByZeroException();
            }
            return leftValue / rightValue;
        default:
        throw InvalidOperatorException();
    }
}

std::string BinaryOperation::toInfix() const {
    std::string leftStr = left->toInfix();
    std::string rightStr = right->toInfix();
    return "(" + leftStr + " " + op + " " + rightStr + ")";
}

std::string Number::toInfix() const {
    std::ostringstream oss;
    oss << value;
    std::string num = oss.str();
    return num;
}

infixParser::infixParser(const std::vector<Token>& tokens, std::map<std::string, double>& symbolTable)
    : tokens(tokens), index(0), symbolTable(symbolTable) {
    if (!tokens.empty()) {
        currentToken = tokens[index];
    }
}

void infixParser::nextToken() {
    if (index < tokens.size() - 1) {
        index++;
        currentToken = tokens[index];
    } else {
        // when END is reached
        currentToken = Token(0, 0, "END", TokenType::OPERATOR);
    }
}

ASTNode* infixParser::infixparse() {
    return infixparseExpression();
}

ASTNode* infixParser::infixparseExpression() {
    ASTNode* left = infixparseTerm();

    while (currentToken.type == TokenType::OPERATOR && (currentToken.text == "+" || currentToken.text == "-")) {
        char op = currentToken.text[0];
        nextToken();  
        ASTNode* right = infixparseTerm();
        left = new BinaryOperation(op, left, right);
    }

    return left;
}

BinaryOperation::~BinaryOperation() {
        delete left;
        delete right;
    }

Assignment::~Assignment() {
        delete expression;
    }

ASTNode* infixParser::infixparseTerm() {
    ASTNode* left = infixparseFactor();

    while (currentToken.type == TokenType::OPERATOR && (currentToken.text == "*" || currentToken.text == "/")) {
        char op = currentToken.text[0];
        nextToken();  
        ASTNode* right = infixparseFactor();
        left = new BinaryOperation(op, left, right);
    }

    return left;
}

ASTNode* infixParser::infixparseFactor() {
    return infixparsePrimary();
}


ASTNode* infixParser::infixparsePrimary() {
    //int parenCount = 0;
    if (currentToken.type == TokenType::NUMBER) {
        double value = std::stod(currentToken.text);
        nextToken();
         return new Number(value);
    } else if (currentToken.type == TokenType::IDENTIFIER) {
        std::string varName = currentToken.text;
        nextToken();  
        if (currentToken.type == TokenType::ASSIGNMENT) {
            nextToken();  
            ASTNode* expr = infixparseExpression();
            return new Assignment(varName, expr);
        } else {
            return new Variable(varName);
        }
    } else if (currentToken.type == TokenType::LEFT_PAREN) {
        //parenCount++;
        nextToken();
        ASTNode* result = infixparseExpression();
        if (currentToken.type == TokenType::RIGHT_PAREN) {
            //parenCount--;
            nextToken();
            return result;
        } else {
            throw UnexpectedTokenException(currentToken.text, currentToken.line, currentToken.column);
        }
    }else if (currentToken.type == TokenType::RIGHT_PAREN) {
        throw UnexpectedTokenException(currentToken.text, currentToken.line, currentToken.column);
    } else {
        throw UnexpectedTokenException(currentToken.text, currentToken.line, currentToken.column);
    }
}

std::string infixParser::printInfix(ASTNode* node) {
    if (dynamic_cast<BinaryOperation*>(node) != nullptr) {
        BinaryOperation* binOp = dynamic_cast<BinaryOperation*>(node);
        std::string leftStr = printInfix(binOp->left);
        std::string rightStr = printInfix(binOp->right);
        return "(" + leftStr + " " + binOp->op + " " + rightStr + ")";
    } else if (dynamic_cast<Number*>(node) != nullptr) {
        std::ostringstream oss;
        oss << dynamic_cast<Number*>(node)->value;
        return oss.str();
    } else if (dynamic_cast<Assignment*>(node) != nullptr) {
        Assignment* assignment = dynamic_cast<Assignment*>(node);
        return "(" + assignment->variableName + " = " + printInfix(assignment->expression) + ")";
    } else if (dynamic_cast<Variable*>(node) != nullptr) {
        Variable* variable = dynamic_cast<Variable*>(node);
        return variable->variableName;
    } else {
        std::cout << "Invalid node type" << std::endl;
        exit(4);
    }
}
#include <iostream>
#include <regex>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

using namespace std;

enum class Type { //перел≥к тип≥в токен≥в
    Identifier, //≥дентиф≥катори
    Keyword, //зарезервован≥ слова
    String, //р€дков≥ 
    CharConstant,//та символьн≥ константи,
    Number, //числа (дес€тков≥, з плаваючою крапкою, ш≥стнадц€тков≥)
    Comment, //коментар≥
    Operator, //оператори
    //Delimiter, 
    PreprocessorDirective, //директиви препроцесора
    Punctuation, //розд≥лов≥ знаки
    Invalid
};

static const unordered_set<char> delimiters = { '(', ')', '{', '}', '[', ']', ';', ',' };
static const unordered_set<char> whitespace = { ' ', '\t', '\n' };
static const unordered_set<string> keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};
static const unordered_set<string> operators = {
    "+", "-", "*", "/", "=", "==", "+=", "-=", "*=", "/=", ">", "<", ">=", "<=", "!=", "&&", "||", "++", "--"
};

struct Token {
    string tokenValue;
    Type tokenType;
    Token(const string& tokenValue_ = "", Type tokenType_ = Type::Invalid)
        : tokenValue(tokenValue_), tokenType(tokenType_) {}
};

Token classifyToken(const string& word) {
    regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
    regex str("\".*?\"");
    regex charConst("'.'");
    regex number("^(0x[0-9A-Fa-f]+|[0-9]+(\\.[0-9]+)?)$");
    regex comment("//.*");
    regex multilineComment("/\\*(.*?)\\*/");
    regex directive("#.*");
    regex punctuation("[!\"#$%&'()*+,-./:;>?@[\\]^_`{|}~]");
    //regex delimiter("[(){}\\[\\];,]");

    if (keywords.find(word) != keywords.end())
        return Token(word, Type::Keyword);

    if (operators.find(word) != operators.end())
        return Token(word, Type::Operator);

    if (regex_match(word, identifier))
        return Token(word, Type::Identifier);

    if (regex_match(word, str))
        return Token(word, Type::String);

    if (regex_match(word, charConst))
        return Token(word, Type::CharConstant);

    if (regex_match(word, number))
        return Token(word, Type::Number);

    if (regex_match(word, comment) || regex_match(word, multilineComment))
        return Token(word, Type::Comment);

    //if (regex_match(word, delimiter))
    //    return Token(word, Type::Delimiter);

    if (regex_match(word, directive))
        return Token(word, Type::PreprocessorDirective);

    if (regex_match(word, punctuation))
        return Token(word, Type::Punctuation);

    return Token(word, Type::Invalid);
}

vector<Token> tokenize(const string& filename) {
    ifstream file(filename, ios::in);
    if (!file) {
        cerr << "Unable to open file!" << endl;
        exit(1);
    }

    vector<Token> tokens;
    string line;
    int lineNumber = 1;
    bool isMultilineComment = false; //part of the multiline comment
    string multilineComment;

    while (getline(file, line)) {
        string buffer;
        bool isString = false; //inside the quotes
        bool isComment = false; //part of the comment
        bool isDirective = false; //part of the directive

        for (char ch : line) {
            if (ch == '\"') {
                isString = !isString;
                buffer += ch;
            }
            else if (isString)
                buffer += ch;

            else if (ch == '/') {
                buffer += ch;
                if (buffer == "//")
                    isComment = true;
                else if (buffer == "*/" && isMultilineComment)
                    isMultilineComment = false;
            }
            else if (isComment) {
                buffer += ch;
                if (ch == '\n')
                    isComment = !isComment;
            }

            else if (ch == '*') {
                buffer += ch;
                if (buffer == "/*")
                    isMultilineComment = true;
            }

            else if (ch == '#') {
                isDirective = true;
                buffer += ch;
            }
            else if (isDirective) {
                buffer += ch;
                if (ch == '\n')
                    isDirective = !isDirective;
            }

            else if (whitespace.find(ch) != whitespace.end()) {
                if (!buffer.empty()) {
                    if (isMultilineComment) {
                        buffer += ch;
                        multilineComment += buffer;
                        buffer.clear();
                    }
                    else {
                        Token token = classifyToken(buffer);
                        tokens.push_back(token);
                        buffer.clear();
                    }
                }
            }
            else if (delimiters.find(ch) != delimiters.end()) {
                if (!buffer.empty()) {
                    if (isMultilineComment) {
                        buffer += ch;
                        multilineComment += buffer;
                        buffer.clear();
                    }
                    else {
                        Token token = classifyToken(buffer);
                        tokens.push_back(token);
                        buffer.clear();
                    }
                }
                tokens.push_back(Token(string(1, ch), Type::Punctuation));
            }
            else
                buffer += ch;
            
        }

        if (!buffer.empty() && !isMultilineComment) {
            Token token = classifyToken(buffer);
            tokens.push_back(token);
        }

        if (isMultilineComment)
            multilineComment += buffer;

        lineNumber++;
    }
    Token token = classifyToken(multilineComment);
    tokens.push_back(token);
    return tokens;
}

int main() {
    unordered_map<Type, string> tokenMap = {
        { Type::Keyword, "keyword" },
        { Type::Identifier, "identifier" },
        { Type::Operator, "operator" },
        { Type::Number, "number" },
        { Type::Comment, "comment" },
        //{ Type::Delimiter, "delimiter" },
        { Type::PreprocessorDirective, "preprocessor directive" },
        { Type::Punctuation, "punctuation" },
        { Type::String, "string" },
        { Type::CharConstant, "character constant" },
        { Type::Invalid, "invalid" }
    };

    vector<Token> tokens = tokenize("lab3_var2_text.txt");

    for (const auto& token : tokens)
        cout << "<" << token.tokenValue << ", " << tokenMap[token.tokenType] << ">" << endl;

    return 0;
}
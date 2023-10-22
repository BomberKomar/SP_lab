#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

enum TypeOfToken
{
  NUMBER_LITERAL,
  STRING_LITERAL,
  CHAR_LITERAL,
  COMMENT,
  KEYWORD,
  OPERATOR,
  WHITESPACE,
  IDENTIFIER
};

class Token
{
public:
  Token(std::string content, TypeOfToken type, bool isInvalid) : content(content), type(type), isInvalid(isInvalid) {}

  std::string getContent() { return content; }

  TypeOfToken getType() { return this->type; }

  bool getIsInvalid() {
    return this->isInvalid;
  }

  std::string getTypeName()
  {
    switch (type)
    {
    case NUMBER_LITERAL:
      return "Number";
    case STRING_LITERAL:
      return "String";
    case CHAR_LITERAL:
      return "Char";
    case COMMENT:
      return "Comment";
    case KEYWORD:
      return "Keyword";
    case OPERATOR:
      return "Operator";
    case WHITESPACE:
      return "Whitespace";
    case IDENTIFIER:
      return "Identifier";
    }

    return "";
  }

private:
  std::string content;
  TypeOfToken type;
  bool isInvalid;
};


class Parser
{
public:
  Parser(std::istream &input) : input(input) {}

  std::vector<Token> parse()
  {
    std::vector<Token> tokens;

    while (this->input.peek() != EOF)
    {
        char currentSymbol = this->input.peek();

        if (isWhitespace(currentSymbol))
        {
            tokens.push_back(this->readWhitespace());
        }
        else if (this->readSpecificWord("//"))
        {
            tokens.push_back(this->readOneLineComment());
        }
        else if (this->readSpecificWord("/*"))
        {
            tokens.push_back(this->readMultiLineComment());
        }
        else if (currentSymbol == '\'') 
        {
            tokens.push_back(this->readCharLiteral());
        }
        else if (currentSymbol == '"')
        {
            tokens.push_back(this->readStringLiteral());
        }
        else if (isDigit(currentSymbol))
        {
            tokens.push_back(this->readNumberLiteral());
        }
        else if (isWordSymbol(currentSymbol))
        {
            tokens.push_back(this->readWord());
        }
        else
        {
            tokens.push_back(this->readOperator());
        }
    }

    return tokens;
  }

private:
  std::istream &input;

  char peek()
  {
    return this->input.peek();
  }

  void moveForward()
  {
    this->input.seekg(1, this->input.cur);
  }

  bool readSpecificWord(const std::string &word)
  {
    for (int i = 0; i < word.length(); i++)
    {
      if (this->peek() != word[i])
      {
        this->input.seekg(-i, this->input.cur);
        return false;
      }

      this->moveForward();
    }

    return true;
  }

  Token readWhitespace()
  {
    std::string content = "";

    while (this->peek() != EOF && this->isWhitespace(this->peek()))
    {
      content += this->peek();
      this->moveForward();
    }

    return Token(content, TypeOfToken::WHITESPACE, false);
  }

  Token readMultiLineComment()
  {
    std::string content = "/*";

    bool isInvalid = true;

    while (this->peek() != EOF)
    {
      content += this->peek();
      this->moveForward();

      if (this->peek() == '*')
      {
        content += this->peek();
        this->moveForward();

        if (this->peek() == '/')
        {
          content += this->peek();
          this->moveForward();
          isInvalid = false;
          break;
        }
      }
    }

    return Token(content, TypeOfToken::COMMENT, isInvalid);
  }

  Token readOneLineComment()
  {
    std::string content = "//";

    while (this->peek() != EOF && !this->isEndOfLine(this->peek()))
    {
      content += this->peek();
      this->moveForward();
    }

    return Token(content, TypeOfToken::COMMENT, false);
  }

  Token readStringLiteral()
  {
    char quoteSymbol = this->peek();
    this->moveForward();
    std::string content;
    content += quoteSymbol;
    bool isInvalid = true;

    while (this->peek() != EOF && !this->isEndOfLine(this->peek()))
    {
      content += this->peek();

      if (this->peek() == quoteSymbol)
      {
        this->moveForward();
        isInvalid = false;
        break;
      }

      this->moveForward();
    }

    return Token(content, TypeOfToken::STRING_LITERAL, isInvalid);
  }

  Token readNumberLiteral()
  {
    std::string content = "";
    bool isInvalid = false;

    content += this->peek();
    this->moveForward();

    if (content == "0" && this->peek() == 'x')
    {
      content += this->peek();
      this->moveForward();

      while (this->peek() != EOF && (this->isWordSymbol(this->peek()) || this->peek() == '.'))
      {
        if (!this->isHexadecimalDigit(this->peek()))
        {
          isInvalid = true;
        }

        content += this->peek();
        this->moveForward();
      }

      return Token(content, TypeOfToken::NUMBER_LITERAL, isInvalid);
    }
    else
    {
      bool contentHasDot = false;
      bool contentHasBotAtTheEnd = false;

      while (this->peek() != EOF && (this->isDigit(this->peek()) || this->peek() == '.'))
      {
        if (this->peek() == '.')
        {
          if (contentHasDot)
          {
            isInvalid = true;
          }
          else
          {
            contentHasDot = true;
          }

          contentHasBotAtTheEnd = true;
        }
        else
        {
          contentHasBotAtTheEnd = false;
        }

        content += this->peek();
        this->moveForward();
      }

      return Token(content, TypeOfToken::NUMBER_LITERAL, isInvalid || contentHasBotAtTheEnd);
    }
  }

  Token readWord()
  {
    std::string content = "";

    while (this->peek() != EOF && this->isWordSymbol(this->peek()))
    {
      content += this->peek();
      this->moveForward();
    }

    if (this->isKeyword(content))
    {
      return Token(content, TypeOfToken::KEYWORD, false);
    }
    else
    {
      return Token(content, TypeOfToken::IDENTIFIER, false);
    }
  }

  Token readOperator()
  {
    auto operators = getOperators();

    for (auto operator_ : operators)
    {
      if (this->readSpecificWord(operator_))
      {
        return Token(operator_, TypeOfToken::OPERATOR, false);
      }
    }

    std::string content = "";

    while (this->peek() != EOF && !this->isWhitespace(this->peek()) && !this->isWordSymbol(this->peek()))
    {
      content += this->peek();
      this->moveForward();
    }

    return Token(content, TypeOfToken::OPERATOR, true);
  }


  Token readCharLiteral() {
    char quoteSymbol = this->input.get();
    std::string content;
    content += quoteSymbol;
    bool isInvalid = true;

    while (this->input.peek() != EOF && this->input.peek() != '\n') {
        content += this->input.peek();

        if (this->input.peek() == quoteSymbol) {
        this->input.get();
        isInvalid = false;
        break;
        }

        this->input.get();
    }

    return Token(content, TypeOfToken::STRING_LITERAL, isInvalid);
  }

  static bool isDigit(char symbol)
  {
    return symbol >= '0' && symbol <= '9';
  }

  static bool isHexadecimalDigit(char symbol)
  {
    return isDigit(symbol) || (symbol >= 'a' && symbol <= 'f') || (symbol >= 'A' && symbol <= 'F');
  }

  static bool isLetter(char symbol)
  {
    return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
  }

  static bool isWordSymbol(char symbol)
  {
    return isDigit(symbol) || isLetter(symbol) || symbol == '_' || symbol == '$';
  }

  static bool isEndOfLine(char symbol)
  {
    return symbol == '\n' || symbol == '\r';
  }

  static bool isWhitespace(char symbol)
  {
    return isEndOfLine(symbol) || symbol == '\t' || symbol == ' ';
  }

  static std::vector<std::string> getOperators()
  {
    std::vector<std::string> operators = {
    "=", "+", "-", "*", "/", "%", 
    "++", "--", 
    "==", "!=", "<", ">", "<=", ">=", 
    "&&", "||", "!", "&", "|", "^", "~", 
    "<<", ">>", 
    "+=", "-=", "*=", "/=", "%=", 
    "&=", "|=", "^=", "<<=", ">>=", 
    "->", ".", ",", ";", ":", "?", "[", "]", "(", ")", "{", "}"};

    std::sort(operators.begin(), operators.end(), [](const std::string &a, const std::string &b)
              { return a.length() > b.length(); });

    return operators;
  };

  static bool isKeyword(const std::string &word)
  {
    return word == "break" ||
           word == "case" ||
           word == "catch" ||
           word == "class" ||
           word == "const" ||
           word == "continue" ||
           word == "delete" ||
           word == "do" ||
           word == "else" ||
           word == "finally" ||
           word == "for" ||
           word == "if" ||
           word == "in" ||
           word == "new" ||
           word == "return" ||
           word == "switch" ||
           word == "this" ||
           word == "throw" ||
           word == "try" ||
           word == "void" ||
           word == "long" ||
           word == "register" ||
           word == "unsigned" ||
           word == "int" ||
           word == "char" ||
           word == "bool" ||
           word == "while";
  }
};

int main()
{
  std::ifstream inputFile;

  while (true) {
    std::string filename;
    
    std::cout << "Enter name of the file: ";
    std::cin >> filename;
    
    inputFile.open(filename);
    
    if (inputFile.is_open()) {
      break;
    } else {
      std::cout << "Failed to open the file. Please, try agait.\n";
    }
  }

  Parser parser(inputFile);

  auto tokens = parser.parse();
      std::cout << "Failed to open the file. Please, try again...";

  for (auto token : tokens)
  {
    if (token.getType() == TypeOfToken::WHITESPACE) {
      std::cout << "whitespace";
    } else {
      std::cout << token.getContent();
    }

    std::cout << " (";
    std::cout << token.getTypeName();

    if (token.getIsInvalid()) {
      std::cout << ", invalid";
    }
    std::cout << ")\n";
  }

  std::cout << '\n';

  return 0;
}
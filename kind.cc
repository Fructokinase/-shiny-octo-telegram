#include "kind.h"
#include <sstream>
using std::string;
using std::istringstream;
using std::cout;
using std::endl;

// Use the annonymous namespace to prevent external linking
namespace {
  // Template function to convert from a string to some Type
  // Primarily used for string to integer conversions
  template <typename T>
  T fromString(const string& s, bool hex = false){
    istringstream iss(s);
    T n;
    if(iss >> n)
      return n;
    else
      throw string("ERROR: Type not convertible from string.");
  }
  // Strings representing the Kind of Token for printing
  const string kindStrings[] ={
    "ID", //: a string consisting of a letter (in the range a-z or A-Z) followed by zero or more letters and digits (in the range 0-9), but not equal to "wain", "int", "if", "else", "while", "println", "return", "NULL", "new" or "delete".
    "NUM", //: a string consisting of a single digit (in the range 0-9) or two or more digits, the first of which is not 0; the numeric value of a NUM token cannot exceed 2^31-1
    "LPAREN", //: the string "("
    "RPAREN", //: the string ")"
    "LBRACE", //: the string "{"
    "RBRACE", //: the string "}"
    "RETURN", //: the string "return" (in lower case)
    "IF", //: the string "if"
    "ELSE", //: the string "else"
    "WHILE", //: the string "while"
    "PRINTLN", //: the string "println"
    "WAIN", //: the string "wain"
    "BECOMES", //: the string "="
    "INT", //: the string "int"
    "EQ", //: the string "=="
    "NE", //: the string "!="
    "LT", //: the string "<"
    "GT", //: the string ">"
    "LE", //: the string "<="
    "GE", //: the string ">="
    "PLUS", //: the string "+"
    "MINUS", //: the string "-"
    "STAR", //: the string "*"
    "SLASH", //: the string "/"
    "PCT", //: the string "%"
    "COMMA", //: the string ","
    "SEMI", //: the string ";"
    "NEW", //: the string "new"
    "DELETE", //: the string "delete"
    "LBRACK", //: the string "["
    "RBRACK", //: the string "]"
    "AMP", //: the string "&"
    "NULL", //: the string "NULL"
    "ERR",
    "WHITESPACE"
  };
}

ASM::Token::Token(ASM::Kind kind, string lexeme)
: kind(kind), lexeme(lexeme){}

// Cannot convert Tokens that do not represent numbers
// to an Integer
int ASM::Token::toInt() const{
  throw "ERROR: attempt to convert non-integer token: " + lexeme;
}

string ASM::Token::toString() const{
  return ::kindStrings[kind];
}

string ASM::Token::getLexeme() const{
  return lexeme;
}

ASM::Kind ASM::Token::getKind() const{
  return kind;
}

ASM::IntToken::IntToken(ASM::Kind kind, string lexeme)
: Token(kind,lexeme){
  long l= ::fromString<long>(lexeme);
  if(l > 2147483647) throw "ERROR: constant out of range: " + lexeme;
}

// Ensure that the IntToken is a valid Integer for conversion
// Arguably, we could do this check in the constructor
int ASM::IntToken::toInt() const{
  long l= ::fromString<long>(lexeme);
  if(l > 2147483647) throw "ERROR: constant out of range: " + lexeme;
  return int(l);
}

// Print the pertinent information to the ostream
std::ostream& ASM::operator<<(std::ostream& out, const ASM::Token& t){
  out << t.toString()  << " "<< t.getLexeme();

  return out;
}

// Make a new Token depending on the Kind provided
// Integral Kinds correspond to the appropriate Token type
ASM::Token* ASM::Token::makeToken(ASM::Kind kind, string lexeme){
  switch(kind){
    case NUM:
      return new IntToken(kind,lexeme);
    default:
      return new Token(kind,lexeme);
  }

}

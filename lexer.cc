#include "lexer.h"
#include "kind.h"
#include <algorithm>
using std::string;
using std::vector;
using std::cout;
using std::endl;

// Use the annonymous namespace to prevent external linking
namespace {
  // An array represents the Token kind that each state represents
  // The ERR Kind represents an invalid token
  ASM::Kind stateKinds[] = {
    ASM::ERR,            //ASM::ST_ERR,
    ASM::ERR, //ASM::ST_START,                // start
    ASM::ID, //ASM::ST_ID,
    ASM::ID, //ASM::ST_singleAlphabet,
    ASM::NUM, //ASM::ST_zero,
    ASM::NUM, //ASM::ST_1digitInt,
    ASM::NUM, //ASM::ST_morethan1digitInt,
    ASM::ID, //ASM::ST_w,
    ASM::ID, //ASM::ST_wa,
    ASM::ID, //ASM::ST_wai,
    ASM::WAIN, //ASM::ST_wain,
    ASM::ID, //ASM::ST_wh,
    ASM::ID, //ASM::ST_whi,
    ASM::ID, //ASM::ST_whil,
    ASM::WHILE, //ASM::ST_while,
    ASM::ID, //ASM::ST_i,
    ASM::IF, // ASM::ST_if,
    ASM::ID, //ASM::ST_in,
    ASM::INT, // ASM::ST_int,
    ASM::ID, //ASM::ST_e,
    ASM::ID, //ASM::ST_el,
    ASM::ID, //ASM::ST_els,
    ASM::ELSE, //ASM::ST_else,
    ASM::ID, //ASM::ST_p,
    ASM::ID, //ASM::ST_pr,
    ASM::ID, //ASM::ST_pri,
    ASM::ID, //ASM::ID, //ASM::ST_prin,
    ASM::ID, //ASM::ST_print,
    ASM::ID, //ASM::ST_printl,
    ASM::PRINTLN, //ASM::ST_println,
    ASM::ID, //ASM::ST_r,
    ASM::ID, //ASM::ST_re,
    ASM::ID, //ASM::ST_ret,
    ASM::ID, //ASM::ST_retu,
    ASM::ID, //ASM::ST_retur,
    ASM::RETURN, //ASM::ST_return,
    ASM::ID, //ASM::ST_N,
    ASM::ID, //ASM::ST_NU,
    ASM::ID, //ASM::ST_NUL,
    ASM::NUL, //ASM::ST_NULL,
    ASM::ID, //ASM::ST_n,
    ASM::ID, //ASM::ST_ne,
    ASM::NEW, //ASM::ST_new,
    ASM::ID, //ASM::ST_d,
    ASM::ID, //ASM::ST_de,
    ASM::ID, //ASM::ST_del,
    ASM::ID, //ASM::ST_dele,
    ASM::ID, //ASM::ST_delet,
    ASM::DELETE, //ASM::ST_delete,
    ASM::ERR,                      //ASM::ST_NOT,
    ASM::WHITESPACE, //ST_WHITESPACE
    ASM::WHITESPACE, // ST_COMMENT
    ASM::LPAREN, //ST_LPAREN, //: the string "("
    ASM::RPAREN, //ST_RPAREN, //: the string ")"
    ASM::LBRACE, //ST_LBRACE, //: the string "{"
    ASM::RBRACE, //ST_RBRACE, //: the string "}"
    ASM::BECOMES, //ST_BECOMES, //: the string "="
    ASM::EQ, //ST_EQ, //: the string "=="
    ASM::NE, //ST_NE, //: the string "!="
    ASM::LT, //ST_LT, //: the string "<"
    ASM::GT, //ST_GT, //: the string ">"
    ASM::LE, //ST_LE, //: the string "<="
    ASM::GE, //ST_GE, //: the string ">="
    ASM::PLUS, //ST_PLUS, //: the string "+"
    ASM::MINUS, //ST_MINUS, //: the string "-"
    ASM::STAR, //ST_STAR, //: the string "*"
    ASM::SLASH, //ST_SLASH, //: the string "/"
    ASM::PCT, //ST_PCT, //: the string "%"
    ASM::COMMA, //ST_COMMA, //: the string ","
    ASM::SEMI, //ST_SEMI, //: the string ";"
    ASM::LBRACK, //ST_LBRACK, //: the string "["
    ASM::RBRACK, //ST_RBRACK, //: the string "]"
    ASM::AMP, //ST_AMP, //: the string "&"
    ASM::ERR // ST_zero_intermediate
  };
  const string whitespace = "\t\n\r ";
  const string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  const string no_f = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdeghijklmnopqrstuvwxyz";
  const string no_n = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmopqrstuvwxyz";
  const string no_f_n = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdeghijklmopqrstuvwxyz";
  const string no_t = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrsuvwxyz";
  const string no_l = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
  const string no_a_h = "ABCDEFGHIJKLMNOPQRSTUVWXYZbcdefgijklmnopqrstuvwxyz";
  const string no_a = "ABCDEFGHIJKLMNOPQRSTUVWXYZbcdefghijklmnopqrstuvwxyz";
  const string no_i = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghjklmnopqrstuvwxyz";
  const string no_e = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdfghijklmnopqrstuvwxyz";
  const string no_s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrtuvwxyz";
  const string no_r = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqstuvwxyz";
  const string no_u = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstvwxyz";
  const string no_w = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvxyz";

  const string no_U = "ABCDEFGHIJKLMNOPQRSTVWXYZabcdefghijklmnopqrstuvwxyz";
  const string no_L = "ABCDEFGHIJKMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  const string id_start = "ABCDEFGHIJKLMOPQRSTUVWXYZabcfghjklmoqstuvxyz";

  const string digits = "0123456789";
  const string hexDigits = "0123456789ABCDEFabcdef";
  const string oneToNine =  "123456789";
  const string singleChar = "(){}[];&%+-*/,";
  const string twoCharPossible = "<>=";

}

ASM::Lexer::Lexer(){
  // Set default transitions to the Error state
  for(int i=0; i < maxStates; ++i){
    for(int j=0; j < maxTrans; ++j){
      delta[i][j] = ST_ERR;
    }
  }
  // Change transitions as appropriate for the MIPS recognizer
  setTrans( ST_START,      "0",     ST_zero);
  setTrans( ST_START,   oneToNine,  ST_1digitInt);
  setTrans( ST_1digitInt,  digits,  ST_morethan1digitInt);
  setTrans( ST_morethan1digitInt,  digits,  ST_morethan1digitInt);

  setTrans( ST_START,  id_start,       ST_ID);
  setTrans( ST_ID,     letters+digits, ST_ID);

  setTrans( ST_START,  "i",     ST_i);
  setTrans( ST_i,      "f",     ST_if);
  setTrans( ST_i,      "n",     ST_in);
  setTrans( ST_in,     "t",     ST_int);

  setTrans( ST_i,   no_f_n+digits, ST_ID);
  setTrans( ST_in,  no_t+digits, ST_ID);
  setTrans( ST_if, letters+digits, ST_ID);
  setTrans( ST_int, letters+digits, ST_ID);


  setTrans( ST_START,  "w",     ST_w);
  setTrans( ST_w,      "a",     ST_wa);
  setTrans( ST_wa,     "i",     ST_wai);
  setTrans( ST_wai,    "n",     ST_wain);

  setTrans( ST_w,   no_a_h+digits, ST_ID);
  setTrans( ST_wa,  no_i+digits, ST_ID);
  setTrans( ST_wai, no_n+digits, ST_ID);
  setTrans( ST_wain,letters+digits, ST_ID);

  setTrans( ST_w,      "h",     ST_wh);
  setTrans( ST_wh,     "i",     ST_whi);
  setTrans( ST_whi,    "l",     ST_whil);
  setTrans( ST_whil,   "e",     ST_while);

  setTrans( ST_wh,   no_i+digits, ST_ID);
  setTrans( ST_whi,  no_l+digits, ST_ID);
  setTrans( ST_whil, no_e+digits, ST_ID);
  setTrans( ST_while,letters+digits, ST_ID);

  setTrans( ST_START,  "e",     ST_e);
  setTrans( ST_e,      "l",     ST_el);
  setTrans( ST_el,     "s",     ST_els);
  setTrans( ST_els,    "e",     ST_else);

  setTrans( ST_e,   no_l+digits, ST_ID);
  setTrans( ST_el,  no_s+digits, ST_ID);
  setTrans( ST_els, no_e+digits, ST_ID);
  setTrans( ST_else,letters+digits, ST_ID);


  setTrans( ST_START,  "p",     ST_p);
  setTrans( ST_p,      "r",     ST_pr);
  setTrans( ST_pr,     "i",     ST_pri);
  setTrans( ST_pri,    "n",     ST_prin);
  setTrans( ST_prin,   "t",     ST_print);
  setTrans( ST_print,  "l",     ST_printl);
  setTrans( ST_printl, "n",     ST_println);

  setTrans( ST_p,     no_r+digits, ST_ID);
  setTrans( ST_pr,    no_i+digits, ST_ID);
  setTrans( ST_pri,   no_n+digits, ST_ID);
  setTrans( ST_prin,  no_t+digits, ST_ID);
  setTrans( ST_print, no_l+digits, ST_ID);
  setTrans( ST_printl,no_n+digits, ST_ID);
  setTrans( ST_println,letters+digits, ST_ID);

  setTrans( ST_START,  "r",     ST_r);
  setTrans( ST_r,      "e",     ST_re);
  setTrans( ST_re,     "t",     ST_ret);
  setTrans( ST_ret,    "u",     ST_retu);
  setTrans( ST_retu,   "r",     ST_retur);
  setTrans( ST_retur,  "n",     ST_return);

  setTrans( ST_r,     no_e+digits, ST_ID);
  setTrans( ST_re,    no_t+digits, ST_ID);
  setTrans( ST_ret,   no_u+digits, ST_ID);
  setTrans( ST_retu,  no_r+digits, ST_ID);
  setTrans( ST_retur, no_n+digits, ST_ID);
  setTrans( ST_return,letters+digits, ST_ID);

  setTrans( ST_START,  "n",     ST_n);
  setTrans( ST_n,      "e",     ST_ne);
  setTrans( ST_ne,      "w",     ST_new);

  setTrans( ST_n,     no_e+digits, ST_ID);
  setTrans( ST_ne,    no_w+digits, ST_ID);
  setTrans( ST_new,letters+digits, ST_ID);

  setTrans( ST_START,  "N",     ST_N);
  setTrans( ST_N,      "U",     ST_NU);
  setTrans( ST_NU,     "L",     ST_NUL);
  setTrans( ST_NUL,    "L",     ST_NULL);

  setTrans( ST_N,     no_U+digits, ST_ID);
  setTrans( ST_NU,    no_L+digits, ST_ID);
  setTrans( ST_NUL,   no_L+digits, ST_ID);
  setTrans( ST_NULL,letters+digits, ST_ID);

  setTrans( ST_START,  "d",     ST_d);
  setTrans( ST_d,      "e",     ST_de);
  setTrans( ST_de,     "l",     ST_del);
  setTrans( ST_del,    "e",     ST_dele);
  setTrans( ST_dele,   "t",     ST_delet);
  setTrans( ST_delet,  "e",     ST_delete);

  setTrans( ST_d,     no_e+digits, ST_ID);
  setTrans( ST_de,    no_l+digits, ST_ID);
  setTrans( ST_del,   no_e+digits, ST_ID);
  setTrans( ST_dele,  no_t+digits, ST_ID);
  setTrans( ST_delet, no_e+digits, ST_ID);
  setTrans( ST_delete,letters+digits, ST_ID);

  // comparisons
  setTrans( ST_START,  "<",     ST_LT);
  setTrans( ST_LT,     "=",     ST_LE);
  setTrans( ST_START,  ">",     ST_GT);
  setTrans( ST_GT,     "=",     ST_GE);
  setTrans( ST_START,  "!",     ST_NOT);
  setTrans( ST_NOT,    "=",     ST_NE);
  setTrans( ST_START,  "=",     ST_BECOMES);
  setTrans( ST_BECOMES,"=",     ST_EQ);

  //add subract multiple divide
  setTrans( ST_START,  "+",     ST_PLUS);
  setTrans( ST_START,  "-",     ST_MINUS);
  setTrans( ST_START,  "*",     ST_STAR);
  setTrans( ST_START,  "/",     ST_SLASH);

  //comments
  setTrans( ST_SLASH,  "/",     ST_COMMENT);


  // [ ] { } ( )
  setTrans( ST_START,  "[",     ST_LBRACK);
  setTrans( ST_START,  "]",     ST_RBRACK);
  setTrans( ST_START,  "{",     ST_LBRACE);
  setTrans( ST_START,  "}",     ST_RBRACE);
  setTrans( ST_START,  "(",     ST_LPAREN);
  setTrans( ST_START,  ")",     ST_RPAREN);

  // ; , & %
  setTrans( ST_START,  ";",     ST_SEMI);
  setTrans( ST_START,  ",",     ST_COMMA);
  setTrans( ST_START,  "&",     ST_AMP);
  setTrans( ST_START,  "%",     ST_PCT);


  setTrans( ST_WHITESPACE, whitespace, ST_WHITESPACE );
  setTrans( ST_START,      whitespace, ST_WHITESPACE );

  // A comment can only ever lead to the comment state
  for(int j=0; j < maxTrans; ++j) delta[ST_COMMENT][j] = ST_COMMENT;
}

// Set the transitions from one state to another state based upon characters in the
// given string
void ASM::Lexer::setTrans(ASM::State from, const string& chars, ASM::State to){
  for(string::const_iterator it = chars.begin(); it != chars.end(); ++it)
    delta[from][static_cast<unsigned int>(*it)] = to;
}

bool isSign(ASM::Kind k){
  return k == ASM::EQ || k == ASM::NE || k == ASM::LT || k == ASM::LE || k == ASM::GT || k == ASM::GE || k== ASM::BECOMES;
}

bool isKeyword(ASM::Kind k){
  return k == ASM::INT || k == ASM::IF || k == ASM::WHILE || k == ASM::PRINTLN || k == ASM::RETURN ||
   k == ASM::NUL || k== ASM::ELSE || k == ASM::NEW || k == ASM::WAIN || k == ASM::DELETE;
}

// Scan a line of input (as a string) and return a vector
// of Tokens representing the MIPS instruction in that line
vector<ASM::Token*> ASM::Lexer::scan(const string& line){

  Kind previous;
  Kind currKind = WHITESPACE;

  // Return vector
  vector<Token*> ret;
  if(line.size() == 0) return ret;
  // Always begin at the start state
  State currState = ST_START;
  // startIter represents the beginning of the next Token
  // that is to be recognized. Initially, this is the beginning
  // of the line.
  // Use a const_iterator since we cannot change the input line
  string::const_iterator startIter = line.begin();
  // Loop over the the line
  for(string::const_iterator it = line.begin();;){
    // Assume the next state is the error state
    State nextState = ST_ERR;
    // If we aren't done then get the transition from the current
    // state to the next state based upon the current character of
    //input
    if(it != line.end())
      nextState = delta[currState][static_cast<unsigned int>(*it)];
    // If there is no valid transition then we have reach then end of a
    // Token and can add a new Token to the return vector
    if(ST_ERR == nextState){
      // Get the kind corresponding to the current state
      previous = currKind;
      currKind = stateKinds[currState];
      // If we are in an Error state then we have reached an invalid
      // Token - so we throw and error and delete the Tokens parsed
      // thus far
      if(ERR == currKind){
        vector<Token*>::iterator vit;
        for(vit = ret.begin(); vit != ret.end(); ++vit)
          delete *vit;
        throw "ERROR in lexing after reading " + string(line.begin(),it);
      }

      if(isSign(currKind) && isSign(previous)){
        throw "ERROR: signs must be space separated: " + string(startIter, it);
      }

      if(currKind == NUM && previous == NUM){
        throw "ERROR: " + string(line.begin(),it) + " has invalid number";
      }

      if(currKind == ID && previous == NUM){
        throw "ERROR: ID begin with digit " + string(line.begin(),it);
      }

      // If we are not in Whitespace then we push back a new token
      // based upon the kind of the State we end in
      // Whitespace is ignored for practical purposes
      if(WHITESPACE != currKind)
        ret.push_back(Token::makeToken(currKind,string(startIter,it)));
      // Start of next Token begins here
      startIter = it;
      // Revert to start state to begin recognizing next token
      currState = ST_START;
      if(it == line.end()) break;
    } else {
      // Otherwise we proceed to the next state and increment the iterator
      currState = nextState;
      ++it;
    }
  }
  return ret;
}

#ifndef __LEXER_H__
#define __LEXER_H__
#include <string>
#include <vector>

namespace ASM{
  // The different states the the MIPS recognizer uses
  // Judicious use of the pimpl idiom or the Bridge pattern
  // would allow us to place this in the implementation file
  // However, that's more complexity than is necessary
  enum State {
    ST_ERR,
    ST_START,
    ST_ID,
    ST_singleAlphabet,
    ST_zero,
    ST_1digitInt,
    ST_morethan1digitInt,
    ST_w,
    ST_wa,
    ST_wai,
    ST_wain,
    ST_wh,
    ST_whi,
    ST_whil,
    ST_while,
    ST_i,
    ST_if,
    ST_in,
    ST_int,
    ST_e,
    ST_el,
    ST_els,
    ST_else,
    ST_p,
    ST_pr,
    ST_pri,
    ST_prin,
    ST_print,
    ST_printl,
    ST_println,
    ST_r,
    ST_re,
    ST_ret,
    ST_retu,
    ST_retur,
    ST_return,
    ST_N,
    ST_NU,
    ST_NUL,
    ST_NULL,
    ST_n,
    ST_ne,
    ST_new,
    ST_d,
    ST_de,
    ST_del,
    ST_dele,
    ST_delet,
    ST_delete,
    ST_NOT,
    ST_WHITESPACE,
    ST_COMMENT,
    ST_LPAREN, //: the string "("
    ST_RPAREN, //: the string ")"
    ST_LBRACE, //: the string "{"
    ST_RBRACE, //: the string "}"
    ST_BECOMES, //: the string "="
    ST_EQ, //: the string "=="
    ST_NE, //: the string "!="
    ST_LT, //: the string "<"
    ST_GT, //: the string ">"
    ST_LE, //: the string "<="
    ST_GE, //: the string ">="
    ST_PLUS, //: the string "+"
    ST_MINUS, //: the string "-"
    ST_STAR, //: the string "*"
    ST_SLASH, //: the string "/"
    ST_PCT, //: the string "%"
    ST_COMMA, //: the string ","
    ST_SEMI, //: the string ";"
    ST_LBRACK, //: the string "["
    ST_RBRACK, //: the string "]"
    ST_AMP, //: the string "&"
    ST_zero_intermediate,
    ST_wain_intermediate,
    ST_if_intermediate,
    ST_int_intermediate,
    ST_else_intermediate,
    ST_while_intermediate,
    ST_println_intermediate,
    ST_return_intermediate,
    ST_NULL_intermediate,
    ST_new_intermediate,
    ST_delete_intermediate
  };
  // Forward declare the Token class to reduce compilation dependencies
  class Token;

  // Class representing a MIPS recognizer
  class Lexer {
    // At most 52 states and 256 transitions (max number of characters in ASCII)
    static const int maxStates = 116;
    static const int maxTrans = 256;
    // Transition function
    State delta[maxStates][maxTrans];
    // Private method to set the transitions based upon characters in the
    // given string
    void setTrans(State from, const std::string& chars, State to);

  public:
    Lexer();
    // Output a vector of Tokens representing the Tokens present in the
    // given line
    std::vector<Token*> scan(const std::string& line);
  };
}

#endif

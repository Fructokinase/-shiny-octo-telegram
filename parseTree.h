#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "TransitionTable.h"
#include "ProductionRules.h"

#ifndef __parseTree
#define __parseTree

using namespace std;

// needed to construct the parse tree
struct parseNode{
  string key;
  string val;
  bool isDecleration;

  // if a node is "dcl" then
  // decleration is a (name, type) pair
  pair<string, string> decleration;
  string typeResolution;

  vector<parseNode*> children;

  parseNode(string key, string val): key(key), val(val), isDecleration(false){
    decleration = make_pair("","");
    typeResolution = string("");
  }

  void insertChild(string key, string val){
    children.push_back(new parseNode(key, val));
  }

  parseNode* ithChild(int i){
    return children[i];
  }

  bool isLeaf(){
    return children.size() == 0;
  }

  ~parseNode(){
    for(int i = 0; i < children.size(); i++){
      delete children[i];
    }
  }

};

class ParseTree{
  TransitionTable& tt;
  ProductionRules& pr;
  parseNode* root;

  // map to determine whether or not a symbol is terminal
  map<string, bool > terminalDictionary;

  //symbol table for variables
  map<string, string> symbolTable;

  // helper function that creates a parse tree
  // given a node pointer
  void graft(istream& in, parseNode* p);

  //travserses the tree and fill the symbol table
  void fillSymbolTableHelper(parseNode* p);

  // returns the type of the final evaluation of the expression
  // consumes a pointer to a expr node
  string resolveExprType(parseNode* p);

  // determine the type of the final value of a term node
  string resolveTermType(parseNode* p);

  // dtermines the type of the final value of a factor node
  string resolveFactorType(parseNode* p);

  // resolve lvalue nodes
  string resolveLValue(parseNode* p);

  // resolves types and check for invalid type error
  // using post-order traverse
  // only when the children are resolved will an the current node be resolved
  void resolveTree(parseNode* p);

  // add helpful infoamtion to the tree
  // if the node contains dcl, dclType(Oneof "int" "int*") is added
  void prepareTree(parseNode *p);

  //throws and error if an undeclared variable is used
  void checkDeclerationSaftyHelper(parseNode* p);

  // prints the tree in order
  // if the current node is the LHS of a rule, print the rule
  // if the current node is the LHS of a terminal, print the terminal then value
  void printInOrderHelper(parseNode* p);

  // look for statements and see if it is valid
  void checkStatements(parseNode* p);



  // ########### ########### ########### ########### ###########
  // code generation variables begin
  // ########### ########### ########### ########### ###########
  const int RETURN_CHILD = 11;
  const int STATEMENT_CHILD = 9;
  const int DCLS_CHILD = 8;

  // pointer to the node where the root is the main
  parseNode* mainNode;
  string firstParamType;

  map<string, int> locationTable;
  int variableLocation;

  // given an lvalue node, find the variable name of lvalue
  string getLvalueVarName(parseNode*);

  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  void generateDclsCode(parseNode*);

  void generateStatementCode(parseNode*);
  void generateSingleStatementCode(parseNode*);
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // generate code for expr node
  void generateExprCode(parseNode*, int);

  // generate code for term node
  void generateTermCode(parseNode*, int);

  // generate code for factor node
  void generateFactorCode(parseNode*, int);

  // generate code for NUM node
  void generateNUMCode(parseNode*, int);

  // generate code for ID node
  void generateIDCode(parseNode*, int);

  void generateLvalueCode(parseNode*, int);


  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // ****************
  // tests
  void generateComparisonCode(parseNode*);
  // ****************
  // loops
  int loopNumber = 0;
  // IFs
  int ifNumber = 0;
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


  // return
  string returnValue;
  void makeReturn();

  // ########### ########### ########### ########### ###########
  // code generation variables end
  // ########### ########### ########### ########### ###########


  // ***********************************************************
  // optmization variables
  vector<parseNode*> assignmentRhsExpr;
  void findRhsExpr(parseNode*);
  // optmization variables end
  // ***********************************************************


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// end helper function
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
public:
  ParseTree(TransitionTable& tt, ProductionRules& pr):tt(tt), pr(pr),
  variableLocation(0){
  }

  void init(istream& in);

  // initialization
  void setTerminal(vector<string> t);

  // initialization
  void setNonTerminal(vector<string> nt);

  void checkContext();

  void fillSymbolTable();

  void printInOrder();

  void printSymbolTable();

  // below are the functions for code generation

  void makeProlog();

  void makeBody();

  void makeEpilog();

  // optimization
  void optimize();
};

#endif

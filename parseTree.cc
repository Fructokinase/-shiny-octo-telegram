
#include "parseTree.h"

void ParseTree::graft(istream& in, parseNode* p){

  string terminal;
  string terminalVal;

  string line;
  getline(in, line);

  istringstream iss(line);

  string LHS;
  string temp;
  vector<string> RHS;

  iss >> LHS;
  while(iss >> temp){
    RHS.push_back(temp);
  }

  for(int i = 0; i < RHS.size(); i++){
    // get the next line in case of terminal
    if(terminalDictionary[RHS[i]] == true){
      in >> terminal;
      in >> terminalVal;
      p->insertChild(terminal, terminalVal);
      getline(in, line);
      continue;
    }

    if(terminalDictionary[RHS[i]] == false){
      p->insertChild(RHS[i], "");
      graft(in, p->ithChild(i));
    }
  }
}

void ParseTree::fillSymbolTableHelper(parseNode* p){
  if(p == nullptr){
    return;
  }

  string currentKey = p->key;

  // if the current node is dcl,this is a decleration node
  // find out what the type and value is, insert it into symbol table
  if(currentKey == "dcl"){
    pair<string, string> currentDcl = p->decleration;

    // record the relative location of the variable declared
    pair<string, int> currentDclLocation = make_pair(currentDcl.first, variableLocation);

    if(variableLocation == 0){
      firstParamType = currentDcl.second;
    }

    variableLocation = variableLocation - 4;

    for(auto const &entry : symbolTable) {
      if(entry.first == currentDcl.first){
        throw(string("ERROR: " + currentDcl.first  +" is already defined"));
      }
    }
    symbolTable.insert(currentDcl);
    locationTable.insert(currentDclLocation);
  }

  //check if the identifier is defined
  if(currentKey == "ID" && p->isDecleration == false){
    string IDname = p->val;

    if(symbolTable.find(IDname) == symbolTable.end()){
      throw(string("ERROR: " + IDname + " is used before decleration"));
    }
  }

  for(int i = 0; i < p->children.size(); i++){
    fillSymbolTableHelper(p->ithChild(i));
  }
}


string ParseTree::resolveExprType(parseNode* p){

  string finalType;

  // for rule expr -> term
  if(p->children.size() == 1){
    finalType = resolveTermType(p->ithChild(0));
    p->typeResolution = finalType;
    return finalType;
  }

  // for rule expr -> expr PLUS term
  if(p->ithChild(1)->key == "PLUS"){
    string addPresmise1 = resolveExprType(p->ithChild(0));
    string addPresmise2 = resolveTermType(p->ithChild(2));

    if(addPresmise1 == "int" && addPresmise2 == "int"){
      p->typeResolution = "int";
      return "int";
    }

    if(addPresmise1 == "int*" && addPresmise2 == "int"){
      p->typeResolution = "int*";
      return "int*";
    }

    if(addPresmise1 == "int" && addPresmise2 == "int*"){
      p->typeResolution = "int*";
      return "int*";
    }
  }

  // for rule expr -> expr MINUS term
  if(p->ithChild(1)->key == "MINUS"){
    string minusPresmise1 = resolveExprType(p->ithChild(0));
    string minusPresmise2 = resolveTermType(p->ithChild(2));

    if(minusPresmise1 == "int" && minusPresmise2 == "int"){
      p->typeResolution = "int";
      return "int";
    }

    if(minusPresmise1 == "int*" && minusPresmise2 == "int"){
      p->typeResolution = "int*";
      return "int*";
    }

    if(minusPresmise1 == "int*" && minusPresmise2 == "int*"){
      p->typeResolution = "int";
      return "int";
    }
  }

  //anything else is invalid
  throw(string("ERROR: invalid expression"));
};

string ParseTree::resolveTermType(parseNode* p){

  string finalType;

  // for rule term -> factor
  if(p->children.size() == 1){
    finalType = resolveFactorType(p->ithChild(0));
    p->typeResolution = finalType;
    return finalType;
  }

  // for rule term -> term STAR factor
  if(p->ithChild(1)->key == "STAR" ||
     p->ithChild(1)->key == "SLASH" ||
     p->ithChild(1)->key == "PCT"){
    string presmise1 = resolveTermType(p->ithChild(0));
    string presmise2 = resolveFactorType(p->ithChild(2));

    if(presmise1 == "int" && presmise2 == "int"){
      p->typeResolution = "int";
      return "int";
    }
  }

  //anything else is invalid
  throw(string("ERROR: invalid term"));
};

string ParseTree::resolveFactorType(parseNode* p){

  parseNode* firstRHS = p->ithChild(0);
  string finalType;

  //rule factor ->ID
  //     factor ID LPAREN RPAREN
  //     factor ID LPAREN arglist RPAREN
  if(firstRHS->key == "ID"){
    //if the name of the identifier is not found then throw
    if(symbolTable.find(firstRHS->val) == symbolTable.end()){
      throw(string("ERROR: unable to find '" + firstRHS->val + "' in the symbol table"));
    }

    //wain cannot call itself recursively
    if(firstRHS->val == "wain"){
      throw(string("ERROR: wain cannot call itself recursively"));
    }

    p->typeResolution = symbolTable[firstRHS->val];
    return symbolTable[firstRHS->val];
  }

  // rule factor -> NUM
  if(firstRHS->key == "NUM"){
    p->typeResolution = "int";
    return "int";
  }

  // rule factor -> NULL
  if(firstRHS->key == "NULL"){
    p->typeResolution = "int*";
    return "int*";
  }

  // rule factor -> LPAREN expr RPAREN
  if(firstRHS->key == "LPAREN"){
    finalType = resolveExprType(p->ithChild(1));
    p->typeResolution = finalType;
    return finalType;
  }

  // rule factor -> AMP lvalue
  if(firstRHS->key == "AMP"){
    parseNode* secondRHS = p->ithChild(1);

    if(secondRHS->typeResolution == ""){
      secondRHS->typeResolution = resolveLValue(secondRHS);
    }

    if(secondRHS->typeResolution != "" && secondRHS->typeResolution != "int"){

      throw(string("ERROR: expected a int but got int*"));
    }
    p->typeResolution = "int*";
    return "int*";
  }

  //rule factor -> NEW INT LBRACK expr RBRACK
  if(firstRHS->key == "NEW"){
    parseNode* fourthRHS = p->ithChild(3);
    if(fourthRHS->typeResolution != "" && fourthRHS->typeResolution != "int"){
      throw(string("ERROR: expected a int but got int*"));
    }
    p->typeResolution = "int*";
    return "int*";
  }

  //rule factor -> star factor
  if(firstRHS->key == "STAR"){
    parseNode* secondRHS = p->ithChild(1);

    if(secondRHS->typeResolution == ""){
      secondRHS->typeResolution = resolveFactorType(secondRHS);
    }

    if(secondRHS->typeResolution != "" && secondRHS->typeResolution != "int*"){
      throw(string("ERROR: expected a int* but got int"));
    }
    p->typeResolution = "int";
    return "int";
  }

  //anything else is invalid
  throw(string("ERROR: invalid factor"));
}

string ParseTree::resolveLValue(parseNode* p){

  string finalType;
  parseNode* firstRHS = p->ithChild(0);

  if(firstRHS->key == "ID"){
    //if the name of the identifier is not found then throw
    if(symbolTable.find(firstRHS->val) == symbolTable.end()){
      throw(string("ERROR: unable to find '" + firstRHS->val + "' in the symbol table"));
    }

    p->typeResolution = symbolTable[firstRHS->val];
    return symbolTable[firstRHS->val];
  }

  //rule lvalue -> STAR factor
  if(firstRHS->key == "STAR"){
    parseNode* secondRHS = p->ithChild(1);

    if(secondRHS->typeResolution == ""){
      secondRHS->typeResolution = resolveFactorType(secondRHS);
    }

    //can only dereference a pointer
    if(secondRHS->typeResolution != "int*"){
      throw(string("ERROR: invalid rule: lvalue -> STAR factor"));
    }

    p->typeResolution = "int";
    return "int";
  }

  if(firstRHS->key == "LPAREN"){
    finalType = resolveLValue(p->ithChild(1));
    p->typeResolution = finalType;
    return finalType;
  }

  //anything else is invalid
  throw(string("ERROR: invalid lvalue"));
}

void ParseTree::resolveTree(parseNode* p){

  string currentKey = p->key;

  for(int i = 0; i < p->children.size(); i++){
    resolveTree(p->ithChild(i));
  }

  // handles return expressions:
  // expr -> term -> factor id
  // A9P1
  if(currentKey == "main"){
    parseNode* returnExpr = p->ithChild(11);

    parseNode* returnTerm = returnExpr->ithChild(0);
    parseNode* returnFactor = returnTerm->ithChild(0);
    parseNode* returnID = returnFactor->ithChild(0);

    returnValue = returnID->val;
  }

  //resolve expressions that are unresolved
  if(currentKey == "expr" && p->typeResolution == ""){
    p->typeResolution = resolveExprType(p);
  }

  //resolve lvalues that are unresolved
  if(currentKey == "lvalue"){
    p->typeResolution = resolveLValue(p);
  }
}

void ParseTree::prepareTree(parseNode *p){

  if(p == nullptr){
    return;
  }

  string currentKey = p->key;


  // store the node if it is the main
  if(currentKey == "main"){
    mainNode = p;
  }

  if(currentKey == "dcl"){
    string type;
    parseNode* typeNode = p->ithChild(0);

    // case of type INT
    if(typeNode->children.size() == 1){
      type = "int";
    }

    // case of type INT STAR
    if(typeNode->children.size() == 2){
      type = "int*";
    }

    parseNode* valNode = p->ithChild(1);
    if(valNode->key != "ID"){
      throw(string("ERROR: expected an ID node, got" + valNode->key));
    }

    // mark the ID node as a decelration
    valNode->isDecleration = true;

    p->decleration = make_pair(valNode->val, type);
  }

  //recurse
  for(int i = 0; i < p->children.size(); i++){
    prepareTree(p->ithChild(i));
  }

}

void ParseTree::checkDeclerationSaftyHelper(parseNode* p){
  if(p == nullptr){
    return;
  }

  if(p->key == "ID" && p->isDecleration == false){

  }
}

void ParseTree::printInOrderHelper(parseNode* p){
  if(p == nullptr){
    return;
  }

  cout << p->key;

  //print out the value if it is a terminal
  if(terminalDictionary[p->key] == true){
    cout << " " << p->val;
  }

  //if not a terminal print the RHS of the rule
  for(int i = 0; i < p->children.size(); i++){
    cout << " " << (p->ithChild(i))->key;
  }
  cout << endl;

  //recurse in order of child
  for(int i = 0; i < p->children.size(); i++){
    printInOrderHelper(p->ithChild(i));
  }

}

// look for statements and see if it is valid
void ParseTree::checkStatements(parseNode* p){

  string currentKey = p->key;

  if(currentKey == "main"){
    parseNode* returnExpr = p->ithChild(11);
    parseNode* secondDcl = p->ithChild(5);

    if(secondDcl->decleration.second != "int"){
      throw(string("ERROR: second decleration must be int"));
    }

    if(returnExpr->typeResolution != "int"){
      throw(string("ERROR: return type must be int"));
    }

  }

  if(currentKey == "statement"){

    parseNode* firstRHS = p->ithChild(0);

    // rule statement -> lvalue BECOMES expr SEMI
    if(firstRHS->key == "lvalue"){
      parseNode* lvalueNode = p->ithChild(0);
      parseNode* exprNode = p->ithChild(2);

      if(lvalueNode->typeResolution == "" || exprNode->typeResolution == ""){
        throw(string("ERROR: unresolved assignment lvalue or expr"));
      }

      if(lvalueNode->typeResolution != exprNode->typeResolution){
        throw(string("ERROR: invalid assignment of different types"));
      }
    }


    //rule statement -> PRINTLN LPAREN expr RPAREN SEMI
    if(firstRHS->key == "PRINTLN"){
      parseNode* printExpr = p->ithChild(2);

      // make sure type of expr is determined
      if(printExpr->typeResolution == ""){
        throw(string("ERROR: attempting to print an unresolved expr"));
      }

      // make sure expr is int
      if(printExpr->typeResolution != "int"){
        throw(string("ERROR: cannot print a non-int"));
      }
    }

    //rule statement -> DELETE LBRACK RBRACK expr SEMI
    if(firstRHS->key == "DELETE"){
      parseNode* deleteExpr = p->ithChild(3);

      // make sure type of expr is determined
      if(deleteExpr->typeResolution == ""){
        throw(string("ERROR: attempting to delete an unresolved expr"));
      }

      // make sure expr is int*
      if(deleteExpr->typeResolution != "int*"){
        throw(string("ERROR: cannot delete a non-pointer"));
      }
    }

  }
  //end statement

  // begin test
  if(currentKey == "test"){

    parseNode* compareNode = p->ithChild(1);

    // rule test expr -> EQ expr
    // rule test expr -> NE expr
    // rule test expr -> GT expr
    // rule test expr -> LT expr
    // rule test expr -> GE expr
    // rule test expr -> LE expr
    if(compareNode->key == "EQ" || compareNode->key == "NE" ||
       compareNode->key == "GT" || compareNode->key == "LT" ||
       compareNode->key == "GE" || compareNode->key == "LE"){

         parseNode* presmise1 = p->ithChild(0);
         parseNode* presmise2 = p->ithChild(2);

         if(presmise1->typeResolution == "" || presmise2->typeResolution ==""){
           throw(string("ERROR: unresolved expression while attempting to compare"));
         }

         if(presmise1->typeResolution != presmise2->typeResolution){
           throw(string("ERROR: "+ compareNode->key + " must compare expressions of the same type"));
         }
    }
  }
  // end test

  // begin dcls
  if(currentKey == "dcls" && p->children.size() > 0){

    // this is either "NUM" or "NULL"
    parseNode* typeNode = p->ithChild(3);
    pair<string, string> dclNode = (p->ithChild(1))->decleration;

    // rule dcls -> dcls dcl BECOMES NUM SEMI
    if(typeNode->key == "NUM"){
      if(dclNode.second != "int"){
        throw(string("ERROR: attempted to declare a int as a non-int"));
      }
    }
    // rule dcls -> dcls dcl BECOMES NULL SEMI
    if(typeNode->key == "NULL"){
      if(dclNode.second != "int*"){
        throw(string("ERROR: attempted to declare a int* as a non-int*"));
      }
    }
  }
  // end dcls

  for(int i = 0; i < p->children.size(); i++){
    checkStatements(p->ithChild(i));
  }
}

// initialization of the tree
void ParseTree::init(istream& in){

  string terminal;
  string terminalVal;

  string line;
  getline(in, line);

  istringstream iss(line);

  string LHS;
  string temp;
  vector<string> RHS;

  iss >> LHS;
  while(iss >> temp){
    RHS.push_back(temp);
  }

  //create root node
  root = new parseNode(LHS, "");
  for(int i = 0; i < RHS.size(); i++){
    // get the next line in case of terminal
    if(terminalDictionary[RHS[i]] == true){
      in >> terminal;
      in >> terminalVal;
      root->insertChild(terminal, terminalVal);
      getline(in, line);
      continue;
    }

    //recurse for non-terminals
    if(terminalDictionary[RHS[i]] == false){
      root->insertChild(RHS[i],"");
      graft(in, root->ithChild(i));
    }
  }

  prepareTree(root);

}

// initialization
void ParseTree::setTerminal(vector<string> t){
  for(int i = 0; i < t.size();i++){
    terminalDictionary.insert(make_pair(t[i], true));
  }
}

// initialization
void ParseTree::setNonTerminal(vector<string> nt){
  for(int i = 0; i < nt.size();i++){
    terminalDictionary.insert(make_pair(nt[i], false));
  }
}

void ParseTree::checkContext(){
  resolveTree(root);
  checkStatements(root);
}

void ParseTree::fillSymbolTable(){
  fillSymbolTableHelper(root);
}

void ParseTree::printInOrder(){
  printInOrderHelper(root);
}

void ParseTree::printSymbolTable(){
  cerr << "wain" << endl;
  for(auto const &entry : symbolTable) {
    cerr << entry.first << " " << entry.second <<endl;
  }
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// CODE GENERATION BEGIN
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// CODE GENERATION HELPER BEGIN
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void ParseTree::generateExprCode(parseNode* exprNode, int context){

  string firstRHS = exprNode->ithChild(0)->key;

  // rule: expr -> term
  // conrext : return
  if(exprNode->children.size() == 1 && context == 0) {
    generateTermCode(exprNode->ithChild(0), 0);
    return;
  }

  // rule: expr -> expr PLUS term
  // conrext : return
  if(firstRHS == "expr"){

    string secondRHS = exprNode->ithChild(1)->key;

    cout << endl;

    // generate first RHS expr code, say expr2
    generateExprCode(exprNode->ithChild(0), 0);


    // push the result of expr2 onto the stack
    cout << "sw $3, -4($30)    ; save expr2 result on stack"  << endl;
    cout << "sub $30, $30, $4  ; PUSH STACK" << endl;


    // generate term code
    generateTermCode(exprNode->ithChild(2), 0);

    // pop the expr2 result from stack into $5
    cout << "lw $5, 0($30)     ; $5 = expr2 result" << endl;
    cout << "add $30, $30, $4  ; POP STACK " << endl;

    // rule: expr -> expr PLUS term
    if(secondRHS == "PLUS"){

      parseNode* RHSexpr = exprNode->ithChild(0);
      parseNode* RHSterm = exprNode->ithChild(2);

      // case: expr -> int PLUS int
      if(RHSexpr->typeResolution == "int" && RHSterm->typeResolution == "int"){
        cout << "add $3, $5, $3  ; $3 = expr2 + term" << endl;
      }

      // case: expr -> int* PLUS int
      if(RHSexpr->typeResolution == "int*" && RHSterm->typeResolution == "int"){
        cout << "mult $3, $4" << endl;
        cout << "mflo $3" << endl;
        cout << "add $3, $5, $3     ; $3 <- pointer addition result" << endl;
      }

      // case: expr -> int PLUS int*
      if(RHSexpr->typeResolution == "int" && RHSterm->typeResolution == "int*"){
        cout << "mult $5, $4" << endl;
        cout << "mflo $5" << endl;
        cout << "add $3, $5, $3     ; $3 <- pointer addition result" << endl;
      }

    }

    // rule: expr ->  expr MINUS term
    if(secondRHS == "MINUS"){

      parseNode* RHSexpr = exprNode->ithChild(0);
      parseNode* RHSterm = exprNode->ithChild(2);

      // case: expr -> int MINUS int
      if(RHSexpr->typeResolution == "int" && RHSterm->typeResolution == "int"){
        cout << "sub $3, $5, $3  ; $3 = expr2 - term" << endl;
      }

      // case: expr -> int* MINUS int
      if(RHSexpr->typeResolution == "int*" && RHSterm->typeResolution == "int"){
        cout << "mult $3, $4" << endl;
        cout << "mflo $3" << endl;
        cout << "sub $3, $5, $3  ; $3 = expr2 - term" << endl;
      }

      // case: expr -> int* MINUS int*
      if(RHSexpr->typeResolution == "int*" && RHSterm->typeResolution == "int*"){
        cout << "sub $3, $5, $3  ; $3 = expr2 - term" << endl;
        cout << "div $3, $4" << endl;
        cout << "mflo $3" << endl;
      }


    }

    cout << endl;
  }

}

void ParseTree::generateTermCode(parseNode* termNode, int context){

  cout << endl;

  string firstRHS = termNode->ithChild(0)->key;

  // rule: term -> factor
  // context : return
  if(termNode->children.size() == 1 && context == 0) {
    generateFactorCode(termNode->ithChild(0), 0);
    cout << endl;
    return;
  }

  // operand: (Oneof STAR SLASH PCT)
  string secondRHS = termNode->ithChild(1)->key;

  // generate code for term before the operand
  generateTermCode(termNode->ithChild(0), 0);

  // push the result of term2 onto the stack
  cout << "sw $3, -4($30)    ; save term2 result on stack"  << endl;
  cout << "sub $30, $30, $4  ; PUSH STACK" << endl;

  // generate code for the factor after the operand
  generateFactorCode(termNode->ithChild(2), 0);

  // pop the term2 result from stack into $5
  cout << "lw $5, 0($30)     ; $5 = term2 result" << endl;
  cout << "add $30, $30, $4  ; POP STACK " << endl;

  // factor result is in $3
  // term result is in $5

  // term -> term STAR factor
  if(secondRHS == "STAR"){
    cout << "mult $5, $3     ; term -> term STAR factor" << endl;
    cout << "mflo $3         ;" << endl;
  }

  // term -> term SLASH factor
  if(secondRHS == "SLASH"){
    cout << "div $5, $3      ; term -> term SLASH factor" << endl;
    cout << "mflo $3         ;" << endl;
  }

  // term -> term PCT factor
  if(secondRHS == "PCT"){
    cout << "div $5, $3      ; term -> term PCT factor" << endl;
    cout << "mfhi $3         ;" << endl;
  }

  cout << endl;

}

void ParseTree::generateFactorCode(parseNode* factorNode, int context){

  string firstRHS = factorNode->ithChild(0)->key;

  // rule: factor -> ID
  // context : return
  if(firstRHS == "ID" && context == 0) {
    generateIDCode(factorNode->ithChild(0), 0);
  }

  // rule: factor -> NUM
  // context : return
  if(firstRHS == "NUM" && context == 0) {
    generateNUMCode(factorNode->ithChild(0), 0);
  }


  // rule: factor -> LPAREN expr RPAREN
  if(firstRHS == "LPAREN"){
    generateExprCode(factorNode->ithChild(1), 0);
  }

  // rule: factor -> STAR factor
  if(firstRHS == "STAR"){
    // generate code for the pointer
    cout << "; dereferencing" << endl;
    generateFactorCode(factorNode->ithChild(1), 0);
    cout << "lw $3, 0($3)" << endl;
  }

  // rule: factor -> NULL
  if(firstRHS == "NULL"){
    cout << "add $3, $0, $11    ; $3 is NULL, can't dereference" << endl;
  }

  // rule: factor -> AMP lvalue
  if(firstRHS == "AMP"){
    parseNode* lvalueNode = factorNode->ithChild(1);

    string lvalueFirstRHS = (lvalueNode->ithChild(0))->key;

    // rule: lvalue -> ID
    if(lvalueFirstRHS == "ID"){

      string variableName = getLvalueVarName(lvalueNode);

      int offset = locationTable[variableName];

      cout << "; $3 stores address of " << variableName  << endl;
      cout << "lis $3" << endl;
      cout << ".word " << offset << endl;
      cout << "add $3, $3, $29" << endl;

    }

    // rule: lvalue -> STAR factor
    if(lvalueFirstRHS == "STAR"){
      // & ( * factor ) -> factor
      generateFactorCode(lvalueNode->ithChild(1), 0);
    }

    // rule: lvalue -> LPAREN lvalue RPAREN
    if(lvalueFirstRHS == "LPAREN"){
      string variableName = getLvalueVarName(lvalueNode);

      int offset = locationTable[variableName];

      cout << "lis $3" << endl;
      cout << ".word " << offset << endl;
      cout << "add $3, $3, $29" << endl;
    }

  }

  // rule: factor -> NEW INT LBRACK expr RBRACK
  if(firstRHS == "NEW"){

    // store the size of the new memory required in $1
    generateExprCode(factorNode->ithChild(3), 0);
    // change the location of the result to $1
    cout << "add $1, $3, $0"<< endl;


    // call new procedure
    // push return address onto the stack
    cout << "sw $31, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;

    // call new
    cout << "lis $10" << endl;
    cout << ".word new" << endl;
    cout << "jalr $10" << endl;

    // if success : $3 stores the address of the allocated memory
    // if fail    : $3 stores 0
    cout << "bne $3, $0, 1" << endl;
    // if allocation not successful then set $3 to 1
    cout << "add $3, $0, $11" << endl;
    // pop return address off stack and back to $31
    cout << "add $30, $30, $4" << endl;
    cout << "lw $31, -4($30)" << endl;

  }

}

void ParseTree::generateNUMCode(parseNode* NUMNode, int context){
  // context : return
  if(context == 0){
    cout << "lis $3      ;  NUM = "  << NUMNode->val << endl;
    cout << ".word " << NUMNode->val << endl;
  }
}

void ParseTree::generateIDCode(parseNode* IDNode, int context){

  // context : return
  if(context == 0){

    string currentVarName = IDNode->val;
    int returnOffset = locationTable[currentVarName];

    cout << "lw $3, " << returnOffset << "($29)"<< endl;
  }
}

void ParseTree::generateStatementCode(parseNode* statementsNode){

  // base case : do nothing when there are no more statements
  if(statementsNode->children.size() == 0){
    return;
  }

  string firstRHS = statementsNode->ithChild(0)->key;

  // rule : statements -> statements statement
  // if there are still statements then generate the code
  //   for those statements first
  if(firstRHS == "statements"){
    generateStatementCode(statementsNode->ithChild(0));
  }

  // handle the actual statement
  generateSingleStatementCode(statementsNode->ithChild(1));

}

void ParseTree::generateSingleStatementCode(parseNode* statementNode){

  string firstRHS = statementNode->ithChild(0)->key;

  // rule : statement -> PRINTLN LPAREN expr RPAREN SEMI
  if(firstRHS == "PRINTLN"){
    cout << endl << "; PRINTLN " << endl;
    generateExprCode(statementNode->ithChild(2), 0);

    // push return addresss ($31) onto the stack
    cout << "sw $31, -4($30)"<< endl;
    cout << "sub $30, $30, $4    ; PUSH STACK" << endl;

    // transfer the result of expr code from $3 to $1
    cout << "add $1, $3, $0  ; transfer expr result for print"<< endl;
    cout << "lis $10" << endl;
    cout << ".word print" << endl;
    cout << "jalr $10" << endl;

    // pop the return address off stack and back into $31
    cout << "add $30, $30, $4    ; POP STACK" << endl;
    cout << "lw $31, -4($30)" << endl;

    cout << "; END PRINTLN " << endl << endl;
  }

  // rule: statement -> lvalue BECOMES expr SEMI
  if(firstRHS == "lvalue"){

    parseNode* lvalueNode = statementNode->ithChild(0);
    string lvalueFirstRHS = lvalueNode->ithChild(0)->key;

    // generate the expression code and store the result in $3
    generateExprCode(statementNode->ithChild(2), 0);

    if(lvalueFirstRHS == "STAR"){

      // push $3 onto the stack
      cout << "sw $3, -4($30)       ; expr result pushed onto stack" << endl;
      cout << "sub $30, $30, $4" << endl;

      // generate the address of the pointer and store it in $3
      generateLvalueCode(lvalueNode, 0);

      // pop back to $5
      cout << "add $30, $30, $4" << endl;
      cout << "lw $5, -4($30)" << endl;

      // $5 contains expr result
      // $3 contains lvalue address
      cout << "sw $5, 0($3)" << endl;
    }

    // rule: lvalue -> LPAREN lvalue RPAREN
    else if(lvalueFirstRHS == "LPAREN"){

      // push $3 onto the stack
      cout << "sw $3, -4($30)       ; expr result pushed onto stack" << endl;
      cout << "sub $30, $30, $4" << endl;

      // generate the address of the pointer and store it in $3
      generateLvalueCode(lvalueNode->ithChild(1), 0);

      // pop back to $5
      cout << "add $30, $30, $4" << endl;
      cout << "lw $5, -4($30)" << endl;

      // $5 contains expr result
      // $3 contains lvalue address
      cout << "sw $5, 0($3)" << endl;

    }

    // rule: lvalue -> ID
    else {
      // lavalue id : variable name
      string variableName = getLvalueVarName(lvalueNode);

      int offset = locationTable[variableName];
      cout << "; Reassigning the value of " << variableName << endl;
      cout << "sw $3, " << offset << "($29)" << endl;
    }

  }

  // rule: statement -> WHILE LPAREN test RPAREN LBRACE statements RBRACE
  if(firstRHS == "WHILE"){

    string loopLabel = "LOOP" + to_string(loopNumber);
    string endLabel = "END" + to_string(loopNumber);
    loopNumber++;

    cout << loopLabel << ":" << endl;

    // WHILE CONDITION
    generateComparisonCode(statementNode->ithChild(2));

    cout << "beq $3, $0, " << endLabel << endl;

    // WHILE BODY
    generateStatementCode(statementNode->ithChild(5));
    cout << "add $0, $0, $0" << endl;

    cout << "beq $0, $0, " << loopLabel << endl;
    cout << endLabel << ":" << endl;
  }

  // rule: statement -> IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
  if(firstRHS == "IF"){

    string ifElse = "IF" + to_string(ifNumber);
    string ifEnd = "endIF" + to_string(ifNumber);
    ifNumber++;

    // IF CONDITION
    generateComparisonCode(statementNode->ithChild(2));

    cout << "beq $3, $0, " << ifElse << endl;

    // first statement
    generateStatementCode(statementNode->ithChild(5));

    cout << "beq $0, $0, " << ifEnd << endl;
    cout << ifElse << ":" << endl;

    // second statement
    generateStatementCode(statementNode->ithChild(9));

    cout << ifEnd << ":" << endl;

  }

  // rule: statement -> DELETE LBRACK RBRACK expr SEMI
  if(firstRHS == "DELETE"){

    // load the expression to $3
    generateExprCode(statementNode->ithChild(3), 0);

    //check if $3 is NULL
    cout << "; if expr is NULL then skip delete" << endl;
    cout << "beq $3, $11, 8"<< endl;

    // copy result to $1
    cout << "add $1, $3, $0" << endl;

    // call delete procedure
    // push return address onto the stack
    cout << "sw $31, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;

    // call delete
    cout << "lis $10" << endl;
    cout << ".word delete" << endl;
    cout << "jalr $10" << endl;

    // pop return address off stack and back to $31
    cout << "add $30, $30, $4" << endl;
    cout << "lw $31, -4($30)" << endl;
  }
}

void ParseTree::generateComparisonCode(parseNode* testNode){
  string test = testNode->ithChild(1)->key;

  // generate code for LHS
  generateExprCode(testNode->ithChild(0), 0);

  // push LHS result onto the stack
  cout << "sw $3, -4($30)"  << endl;
  cout << "sub $30, $30, $4"<< endl;

  // generate code for RHS
  generateExprCode(testNode->ithChild(2), 0);

  // pop LHS result from the stack to $5
  cout << "add $30, $30, $4" << endl;
  cout << "lw $5, -4($30)" << endl;

  // LHS -> $5
  // RHS -> $3

  // since type(LHS) == type(RHS) from context analysis
  // the type of comparison is can be checked by the type of LHS
  // comparing int  -> comparisonType == 0
  // comparing int* -> comparisonType == 1
  int comparisonType = (testNode->ithChild(0))->typeResolution == "int" ? 0 : 1;
  string sltType = comparisonType == 0 ? "slt" : "sltu";

  if(test == "LT"){
    cout << "; LT " << endl;
    cout << sltType  <<" $3, $5, $3  ; if $5 < $3 then $3 == 1" << endl;
    cout << "; END LT" << endl;
  }

  if(test == "EQ"){
    cout << "; EQ " << endl;
    cout << sltType << " $6, $3, $5" << endl;
    cout << sltType << " $7, $5, $3" << endl;
    cout << "add $3, $6, $7" << endl;
    cout << "sub $3, $11, $3" << endl;
    cout << "; END EQ" << endl;
  }

  if(test == "NE"){
    cout << "; NE " << endl;
    cout << sltType << " $6, $3, $5" << endl;
    cout << sltType << " $7, $5, $3" << endl;
    cout << "add $3, $6, $7" << endl;
    cout << "; END NE " << endl;
  }

  if(test == "LE"){
    cout << "; EQ" << endl;
    cout << sltType << " $3, $3, $5" << endl;
    cout << "sub $3, $11, $3" << endl;
    cout << "; END EQ" << endl;
  }

  if(test == "GE"){
    cout << "; GE" << endl;
    cout << sltType << " $3, $5, $3" << endl;
    cout << "sub $3, $11, $3" << endl;
    cout << "; END GE" << endl;
  }

  if(test == "GT"){
    cout << "; GT " << endl;
    cout << sltType << " $3, $3, $5  ; if $3 < $5 then $3 == 1" << endl;
    cout << "; END GT " << endl;
  }


}

string ParseTree::getLvalueVarName(parseNode* lvalueNode){
  string lvalueFirstRHS = lvalueNode->ithChild(0)->key;

  // rule: lvalue -> ID
  if(lvalueFirstRHS == "ID"){
    return lvalueNode->ithChild(0)->val;
  }

  // rule: lvalue -> LPAREN lvalue RPAREN
  if(lvalueFirstRHS == "LPAREN"){
    return getLvalueVarName(lvalueNode->ithChild(1));
  }

  throw(string("ERROR: lvalue"));
  return "ERROR";
}


// result: lvaluecode is now in $3
// context == 1 -> store the value of the lavalue in $3
// context == 0 -> store the address of the lavalue in $3
void ParseTree::generateLvalueCode(parseNode* lvalueNode, int context){

  string firstRHS = lvalueNode->ithChild(0)->key;

  if(firstRHS == "STAR"){
    // generate factor code
    // rule : lvalue -> STAR factor
    generateFactorCode(lvalueNode->ithChild(1), 0);
  }

  if(firstRHS == "LPAREN"){
    // recurse
    generateLvalueCode(lvalueNode->ithChild(1), context);
  }

  if(firstRHS == "ID"){
    string variableName = lvalueNode->ithChild(0)->val;
    int offset = locationTable[variableName];

    // load the address to $3
    cout << "lw $3, " << offset << "($29)     ; address of " << variableName << endl;

  }

}

/////////////////////////////////////////////////////////////////
// DCLS
/////////////////////////////////////////////////////////////////
void ParseTree::generateDclsCode(parseNode* dclsNode){

  // base case : do nothing when there are no more decelrations
  if(dclsNode->children.size() == 0){
    return;
  }

  string thirdRHS = dclsNode->ithChild(3)->key;

  // rule : dcls -> dcls dcl BECOMES NUM SEMI
  if(thirdRHS == "NUM"){
    // generate code for the first dcls
    generateDclsCode(dclsNode->ithChild(0));

    // make a int decleration
    generateNUMCode(dclsNode->ithChild(3), 0);
    // NUM value is now stored in $3

    // go to the dcl node and extract the decleration pair
    string variableName = (dclsNode->ithChild(1)->decleration).first;
    int offset = locationTable[variableName];

    cout << "sw $3, " << offset << "($29)    ; declare " << variableName << endl;
    cout << "sub $30, $30, $4  " << endl;

  }

  // rule : dcls -> dcls dcl BECOMES NULL SEMI
  if(thirdRHS == "NULL"){
    // generate code for the first dcls
    generateDclsCode(dclsNode->ithChild(0));

    // make a int* decleration
    string variableName = (dclsNode->ithChild(1))->decleration.first;
    int offset = locationTable[variableName];

    cout << "sw $11, " <<  offset <<  "($29)     ; setting NULL"<< endl;
    cout << "sub $30, $30, $4  " << endl;
  }


}
/////////////////////////////////////////////////////////////////
// END DCLS
/////////////////////////////////////////////////////////////////

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// CODE GENERATION HELPER END
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void ParseTree::makeProlog(){

  // import print function
  cout << ".import print" << endl;

  // import init function
  cout << ".import init" << endl;
  cout << ".import new" << endl;
  cout << ".import delete" << endl;

  // constant 4 is stored in $4
  cout << "lis $4" << endl;
  cout << ".word 4" << endl;

  // constant 1 is stored in $11
  cout << "lis $11" << endl;
  cout << ".word 1" << endl;

  // constant -1 is stored in $21
  cout << "lis $21" << endl;
  cout << ".word -1" << endl;


  cout << "sub $29, $30, $4" << endl;

  //store a in $1
  cout << "sw $1, -4($30)" << endl;
  cout << "sub $30, $30, $4" << endl;

  //store b in $2
  cout << "sw $2, -4($30)" << endl;
  cout << "sub $30, $30, $4" << endl;

  // call init
  cout << "; call init" << endl;
  cout << "lis $10" << endl;
  cout << ".word init" << endl;

  // set $2 to 0 if the first parameter is int
  if(firstParamType == "int"){
    // push $2 to stack
    cout << "; push $2 to stack" << endl;
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "add $2, $0, $0" << endl;
  }

  cout << "sw $31, -4($30)" << endl;
  cout << "sub $30, $30, $4" << endl;
  cout << "jalr $10" << endl;

  cout << "add $30, $30, $4" << endl;
  cout << "lw $31, -4($30)" << endl;

  // set $2 back if the first parameter is int
  if(firstParamType == "int"){
    cout << "; load $2 back on to $2 from stack" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "lw $2, -4($30)" << endl;
  }
}

void ParseTree::makeBody(){

  // handle decelrations
  cout << endl << "; BEGIN DECLERATIONS" << endl;
  generateDclsCode(mainNode->ithChild(DCLS_CHILD));
  cout << "; END   DECLERATIONS" << endl << endl;

  // handle statements
  generateStatementCode(mainNode->ithChild(STATEMENT_CHILD));

  // return
  cout << "; RETURN" << endl;
  generateExprCode(mainNode->ithChild(RETURN_CHILD), 0);
  cout << "; END RETURN " << endl;
}

void ParseTree::makeEpilog(){

  // add the stack pointer back up
  cout << "add $30, $29, $4" << endl;

  // return
  cout << "jr $31" << endl;
}
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// CODE GENERATION END
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// OPTIMIZATION
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
void ParseTree::optimize(){
  findRhsExpr(root);

  // cout << "~ ~ ~ AssignmentRHS" << endl;
  // for(int i = 0; i < assignmentRhsExpr.size(); i++){
  //   printInOrderHelper(assignmentRhsExpr[i]);
  //   cout << " ~ ~ " << endl;
  // }

}

void ParseTree::findRhsExpr(parseNode* p){

  if(p == nullptr){
    return;
  }

  if(p->key == "statement"){

    //only care if first child is lvalue
    // rule: statement -> lvalue BECOMES expr SEMI
    if(p->ithChild(0)->key != "lvalue"){
      return;
    }

    assignmentRhsExpr.push_back(p);


    return;
  }

  for(int i = 0; i < p->children.size(); i++){
    findRhsExpr(p->ithChild(i));
  }

}
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// OPTIMIZATION END
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

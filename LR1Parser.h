#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "TransitionTable.h"
#include "ProductionRules.h"
#include <queue>

#ifndef __LR1Parser
#define __LR1Parser

using namespace std;


// needed to construct the parse tree
struct parseNode{
  string key;
  string val;

  vector<parseNode*> children;

  parseNode(string key, string val): key(key), val(val){
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

class LR1Parser{
  stack<int> statesStack;

  TransitionTable& tt;
  ProductionRules& pr;

  //keeps track of what production was used
  vector<int> productionUsed;

  // unread input
  vector<string> symbolStack;
  vector<string> readInput;
  queue<string>& unReadInput;
  vector<string> values;

  // map to determine whether or not a symbol is terminal
  map<string, bool > terminalDictionary;

  //parseTree
  parseNode* root;

  //symbol table for variables
  map<string, string> symbolTable;

public:
  LR1Parser(TransitionTable& tt, ProductionRules& pr, queue<string>& tokens)
  :tt(tt), pr(pr), unReadInput(tokens){
    statesStack.push(0);
  }

  parseNode* getParseTree(){
    return root;
  }

  void setTerminal(vector<string> t){
    for(int i = 0; i < t.size();i++){
      terminalDictionary.insert(make_pair(t[i], true));
    }
  }

  void setNonTerminal(vector<string> nt){
    for(int i = 0; i < nt.size();i++){
      terminalDictionary.insert(make_pair(nt[i], false));
    }
  }

  void setValues(vector<string>& v){
    values = v;
  }

  // print all current information about the parser
  void printEveryThing(string input){
    cout << endl;
    cout << "stack: bottom >>" ;
    for(int i = 0; i < symbolStack.size(); i++){
      cout << " "<< symbolStack[i];
    }
    cout << " << top " << endl;
    cout << " current state : " << statesStack.top() << " and input: " << input << endl;
    cout << "next unread input is " << unReadInput.front() << endl;
    cout << endl;
  }


  // keep taking actions until all unread inputs are exausted
  void parse(){
    //keep parsing until there are no more tokens
    while(unReadInput.size() > 0){
      string nextInput = unReadInput.front();

      //insert
      action(nextInput, true);
    }

    //make sure EOF if found
    if(symbolStack.back() != "EOF"){
      throw string("ERROR at ") + to_string(symbolStack.size());
    }
  }

  void action(string token, bool shift){

      //for debug
      //printEveryThing(token);

      // get the transition from the transition table
      int currentState = statesStack.top();

      //feed the current state and the input symbol into the state machine
      pair<string, int> currentTransition = tt.nextAction(currentState, token);

      // make sure that the transition is valid
      if(currentTransition.second == -1){
        throw string("ERROR at ") + to_string(readInput.size());
      }

      // shift
      if(currentTransition.first == "shift"){
        //shift from unread input to symbol stack
        symbolStack.push_back(token);

        // only shift terminals
        if(shift) {
          unReadInput.pop();

          //shift from unread input to read input
          readInput.push_back(token);

        }

        //add the current state to the states stack
        statesStack.push(currentTransition.second);
      }

      //
      if(currentTransition.first == "reduce"){

        // get the production rule number from the transition
        // and use ProductionRule to find out what LHS to reduce to
        int productionRuleNumber = currentTransition.second;
        int RHSlen = pr.getRHSLen(productionRuleNumber);
        string LHS = pr.getLHS(productionRuleNumber);


        // for each one the RHS on stack, pop it off
        for(int i = 0; i < RHSlen; i++){

          symbolStack.pop_back();

          // pop off states stack
          statesStack.pop();
        }

        // record down the production rule number that was used
        // for later purposes
        productionUsed.push_back(productionRuleNumber);

        // push LHS back on the stack with recursion
        action(LHS, false);

      }

  }

  void printParseNode(parseNode* p){

    if(p == nullptr) return;

    //if leaf then print the node key and val
    if(p->isLeaf()){
      cout << p->key << " " << p->val << endl;
      return;
    }

    //otherwise print the rule
    cout << p->key;
    for(int i =  p->children.size()- 1; i >= 0; i--){
      parseNode* c = p->ithChild(i);
      cout << " " << c->key;
    }
    cout << endl;

    for(int i = p->children.size()- 1; i>= 0; i--){
      printParseNode(p->ithChild(i));
    }

  }

  void printParseTree(){
    printParseNode(root);
  }


  //a helper function for createParseTree
  void graft(parseNode* current, vector<string>& RHS){

    if(current == nullptr) return;

    for(int i = RHS.size() - 1; i >= 0; i--){

      int ithChild = RHS.size() - i - 1;

      string currentTerm = RHS[i];

      if(terminalDictionary[currentTerm]){
        current->insertChild(currentTerm, values.back());
        values.pop_back();
      } else {
        current->insertChild(currentTerm, "");
      }

      //if a non-terminal is met then recurse
      if(terminalDictionary[currentTerm] == false){

        //get the next production
        int nextRule = productionUsed.back();
        productionUsed.pop_back();

        string nextLHS = pr.getLHS(nextRule);
        vector<string> nextRHS = pr.getRHS(nextRule);

        //make sure the current term matches the LHS
        //of the next term, as in a valid lr1 file
        if(currentTerm != nextLHS){
          throw(string("Unacceptable rule"));
        }

        graft(current->ithChild(ithChild), nextRHS);

      }

    }
  }

  //make the parse tree after the tokens are parsed
  void createParseTree(){
    root = new parseNode(pr.getStart(), pr.getStart());
    root->insertChild("EOF", "EOF");

    int firstRule = productionUsed.back();
    productionUsed.pop_back();

    string LHS = pr.getLHS(firstRule);
    vector<string> RHS = pr.getRHS(firstRule);

    root->insertChild(LHS, "");
    graft(root->ithChild(1), RHS);

    //child <- LHS
    // for each RHS, LHS.insertChild(RHS)
    root->insertChild("BOF", "BOF");
  }


  //print in order, all the production rules used
  void printProductionUsed(){
    for(int i = 0; i < productionUsed.size(); i++){
      cout << pr.to_string(productionUsed[i]) << endl;
    }

    //output the start rule
    cout << pr.getStart();
    for(int i = 0; i < symbolStack.size(); i++){
      cout << " "<< symbolStack[i];
    }
    cout << endl;
  }

  // traverses the parse tree and detects for ID
  // fill in the appropriate type to the symbol table
  void symbolTableHelper(parseNode* p, bool insert, string t){

    //end of the tree
    if(p == nullptr){
      return;
    }

    // when the current key is dcl let the recursed function
    // insert the correct type
    bool ins = false;
    string type = "";

    string currentKey = p->key;

    if(currentKey == "ID" && insert){
      symbolTableInsert(p->val, t);
    } else if(currentKey == "ID") {
      // non-declerations of ID must be found in symbol table
      if( symbolTable.find(p->val) == symbolTable.end()){
        throw(string("ERROR: " + p->val + " is used before decleration"));
      }
    }

    // currentKey == dcl implies that the rule is
    // dcl -> type ID
    if(currentKey == "dcl"){
      ins = true;

      parseNode* typeChild = p->ithChild(1);

      // look for INT
      if(typeChild->children.size() == 1) {
        type = "int";
      }

      // look for INT STAR
      if(typeChild->children.size() == 2){
        type = "int*";
      }

    }

    for(int i = p->children.size() - 1; i >= 0 ; i--){
      symbolTableHelper(p->ithChild(i), ins, type);
    }

  }

  void symbolTableInsert(string val, string type){
    for(auto const &entry : symbolTable) {
      if(entry.first == val){
        throw(string("ERROR: " + val  +" is already defined"));
      }
    }
    symbolTable.insert(make_pair(val, type));
  }

  void createSymbolTable(){
    symbolTableHelper(root, false, "");
  }


  void printSymbolTable(){
    cerr << "wain" << endl;
    for(auto const &entry : symbolTable) {
      cerr << entry.first << " " << entry.second <<endl;
    }
  }

};

#endif

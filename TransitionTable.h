#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include "ProductionRules.h"

#ifndef __TransitionTable
#define __TransitionTable

using namespace std;

// transition table with states of DFA as rows and
// possible inputs as columns
// each cell is a pair of information such that:
//   cell.first  -> action: Oneof("shift", "reduce")
//   cell.second -> if action shift, this is a state number
//                  if action is reduce, this is a rule number
class TransitionTable{
  int stateSize;
  int tokenSize;
  vector< vector <pair <string, int> > > transition;
  map<string, int> tokenEnum;
  vector<string> tokens;
public:
  TransitionTable(int numStates): stateSize(numStates){
  }

  void init(vector<string>& terminals, vector<string>& nonTerminals){
    tokenSize = terminals.size() + nonTerminals.size();

    // the enumeration of tokens are based on order in terminals
    // and then non terminals
    int tokenCounter = 0;
    //terminals
    for(int i = 0; i < terminals.size(); i++){
      tokenEnum.insert(make_pair(terminals[i], tokenCounter));
      tokens.push_back(terminals[i]);
      tokenCounter++;
    }

    //non terminals
    for(int i = 0; i < nonTerminals.size(); i++){
      tokenEnum.insert(make_pair(nonTerminals[i], tokenCounter));
      tokens.push_back(nonTerminals[i]);
      tokenCounter++;
    }

    for(int i = 0; i < stateSize; i++){
      vector<pair<string, int> > temp;
      for(int j = 0; j < tokenSize; j++){
        temp.push_back(make_pair("", -1));
      }
      transition.push_back(temp);
    }
  }

  //result is either a state number or a production rule number
  void insert(int state, string token, string action, int result){
    int tokenIndex = tokenEnum[token];
    transition[state][tokenIndex] = make_pair(action, result);
  }

  // string representation of a cell in the transition table
  string pairToString(int i, int j){
    string action = transition[i][j].first;
    int result = transition[i][j].second;
    return string("(") + action + string(",") + std::to_string(result) + string(")");
  }

  pair<string, int > nextAction(int currentState, string input){
    return transition[currentState][tokenEnum[input]];
  }

  // for p3
  string lrReaction(int n, string x, ProductionRules& pr){
    pair<string, int> currentState = transition[n][tokenEnum[x]];

    if(currentState.first == "shift"){
      return currentState.first + string(" ") + std::to_string(currentState.second);
    }

    if(currentState.first == "reduce"){
      int productionRuleNumber = currentState.second;
      return currentState.first + string(" ") +  pr.to_string(currentState.second);
    }

    return string("error");
  }

  //print the transisiton table
  void print(){

    cout << "   ";
    for(int i = 0; i < tokens.size(); i++){
      cout << std::right << std::setw(13) << tokens[i];
    };
    cout << endl;

    for(int i = 0; i < stateSize; i++){
      cout << i;

      if(i < 10){
        cout << "  ";
      } else {
        cout << " ";
      }

      for(int j = 0; j < tokenSize; j++){
        cout << std::right << std::setw(13) << pairToString(i, j);
      }
      cout<< endl;
    }
  }

};

#endif

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <queue>
#include <fstream>

#include "ProductionRules.h"
#include "TransitionTable.h"
#include "LR1Parser.h"

using namespace std;

// skips the next line in stream
void skipLine(istream& in){
  string line;
  getline(in, line);
}

int main(){

  ProductionRules pr;
  TransitionTable tt(0);
  string token;

  vector<string> terminals;
  vector<string> nonTerminals;

  //read wlp4.lr1
  ifstream fileReader;
  fileReader.open("./wlp4.lr1");


  // read terminals
  int numTerminals;

  fileReader >> numTerminals;
  for(int i = 0; i < numTerminals; i++){
    fileReader >> token;
    terminals.push_back(token);
  }
  // end reading terminals


  // read non terminals
  int numNonTerminals;

  fileReader >> numNonTerminals;
  for(int i = 0; i < numNonTerminals; i++){
    fileReader >> token;
    nonTerminals.push_back(token);
  }
  // end reading non terminals

  string start;
  fileReader >> start;
  pr.setStart(start);

  // begin reading production rules
  int numProduction;

  fileReader >> numProduction;
  skipLine(fileReader);

  for(int i = 0; i < numProduction; i++){
    string line;
    vector<string> RHS;

    getline(fileReader, line);
    istringstream ss(line);

    string LHS;
    ss >> LHS;
    while (ss >> token){
      RHS.push_back(token);
    }

    pr.insert(i, make_pair(LHS, RHS));
  }
  // end reading production


  int numStates;
  int numReduceActions;

  fileReader >> numStates;
  fileReader >> numReduceActions;
  skipLine(fileReader);

  TransitionTable tt_temp(numStates);
  tt_temp.init(terminals, nonTerminals);

  //constructs the transition table
  for(int i = 0; i < numReduceActions; i++){
    string transitionRule;
    getline(fileReader, transitionRule);
    istringstream iss(transitionRule);

    int state;
    string action;
    int result;

    iss >> state;
    iss >> token;
    iss >> action;
    iss >> result;

    tt_temp.insert(state, token, action, result);

  }
  fileReader.close();

  tt = tt_temp;

  queue<string> tokens;
  vector<string> values;

  //insert beginning of file
  tokens.push("BOF");

  string value;

  while(cin >> token && cin >> value){
    tokens.push(token);
    values.push_back(value);
  }

  //insert end of file
  tokens.push("EOF");

  //initialize the wlp4 parser and set the values
  LR1Parser wlp4Parser(tt, pr, tokens);
  wlp4Parser.setTerminal(terminals);
  wlp4Parser.setNonTerminal(nonTerminals);
  wlp4Parser.setValues(values);

  try{
    wlp4Parser.parse();
    //wlp4Parser.printProductionUsed();
    wlp4Parser.createParseTree();
    wlp4Parser.printParseTree();
  } catch(const string s){
    cerr << s << endl;
  }
  // while(tokens.size() > 0){
  //   cout << tokens.front() << " ";
  //   tokens.pop();
  // }cout << endl;

}

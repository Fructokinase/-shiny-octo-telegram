#include <string>
#include <vector>
#include <map>
#include <iostream>

#ifndef __ProductionRules
#define __ProductionRules

using namespace std;

class ProductionRules{
  map<int, pair<string, vector<string> > > rules;
  string start;
public:
  void insert(int n, pair<string, vector<string> > rule){
    rules.insert(make_pair(n, rule));
  }

  //set the staring state
  void setStart(string s){
    start = s;
  }

  string getStart(){
    return start;
  }

  //get the LHS of the i-th rule
  string getLHS(int i){
    return rules[i].first;
  }

  //get the RHS of the i-th rule in a vector form
  vector<string> getRHS(int i){
    return rules[i].second;
  }

  //get the length of the RHS of the i-th rule
  int getRHSLen(int i){
    return rules[i].second.size();
  }

  string to_string(int ruleNumber){
    pair<string, vector<string> > currentRule = rules[ruleNumber];

    //LHS
    string result = currentRule.first;

    //RHS
    vector<string> RHS = currentRule.second;
    for(int i = 0; i <  RHS.size(); i++){
      result = result + string(" ") + RHS[i];
    }

    return result;
  }

  void print(){
    for(int i = 0; i < rules.size(); i++){

      //production number
      cout << i;
      cout << " ";

      // current production rule
      pair<string, vector<string> > currentRule = rules[i];

      // LHS
      cout << currentRule.first;

      // RHS
      vector<string> RHS = currentRule.second;
      for(int i = 0; i < RHS.size(); i++){
        cout << " ";
        cout << RHS[i];
      }

      cout << endl;

    }
  }
};

#endif

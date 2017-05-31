#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <string>


using namespace std;

struct State{
  string name;
  bool isAccepting;
  map<string, string> nextStates;
  State(string s, bool isAccepting):name(s),isAccepting(isAccepting){};
};

class StateMachine{
  vector<string> alphabet;
  vector<State> states;
  State start;
  map<string, int> stateIndexOf;
public:
  StateMachine():start("start", false){}

  void insertAlphabet(string s);

  void insertState(string s);

  void setStartingState(string s);

  void setFinalState(string s);

  void insertTransition(string s, string input, string end);

  string isValid(string line);

  void printStates();

  void printAlphabets();

  void printTransitions();


};

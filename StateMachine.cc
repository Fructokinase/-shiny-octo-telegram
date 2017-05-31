#include "StateMachine.h"

void StateMachine::insertAlphabet(string s){
  alphabet.push_back(s);
}

void StateMachine::insertState(string s){
  states.push_back(State(s, false));
  stateIndexOf.insert(make_pair(s, states.size() - 1));
}

void StateMachine::setStartingState(string s){
  start = states[stateIndexOf[s]];
}

void StateMachine::setFinalState(string s){
  states[stateIndexOf[s]].isAccepting = true;
}

void StateMachine::insertTransition(string s, string input, string end){
  states[stateIndexOf[s]].nextStates.insert(make_pair(input,end));
}

string StateMachine::isValid(string line){
  string current = start.name;

  string next_input;
  stringstream ss(line);

  vector<string> tokens;

  while(ss >> next_input){
    if(states[stateIndexOf[current]].nextStates.find(next_input) == states[stateIndexOf[current]].nextStates.end()){
      return "false";
    }
    current = states[stateIndexOf[current]].nextStates[next_input];
  }

  if(states[stateIndexOf[current]].isAccepting){
    return "true";
  }

  return "false";
}




void StateMachine::printStates(){
  for(int i = 0; i < states.size(); i++){
    cout << states[i].name << " ";
  }
  cout << endl;
}

void StateMachine::printAlphabets(){
  for(int i = 0; i < alphabet.size(); i++){
    cout << alphabet[i] << " ";
  }
  cout << endl;
}

void StateMachine::printTransitions(){
  for(int i = 0; i < states.size();i++){
    typedef std::map<string, string>::iterator labelIt;
    for(labelIt it = states[i].nextStates.begin(); it != states[i].nextStates.end(); it++) {
       cout << states[i].name << " " << (it->first)  << " " << (it->second) << endl;
    }
  }
}

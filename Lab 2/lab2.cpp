#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <queue>

struct Transition {
    int prevState;
    char symbol;
    int nextState;
};

struct FSM {
    int numStates;
    int startState;
    std::set<int> finalStates;
    std::vector<Transition> transitions;
};

FSM readFSM(const std::string& filename) {
    FSM fsm;
    std::ifstream file(filename);
    if (file.is_open()) {
        int numFinalStates;
        int numAlphabetSymbols;
        
        file >> numAlphabetSymbols >> fsm.numStates >> fsm.startState >> numFinalStates;

        for (int i = 0; i < numFinalStates; i++) {
            int finalState;
            file >> finalState;
            fsm.finalStates.insert(finalState);
        }

        Transition t;
        while (file >> t.prevState >> t.symbol >> t.nextState) {
            fsm.transitions.push_back(t);
        }
    }
    return fsm;
}

void DFS(int state, const std::vector<std::vector<int>>& graph, std::vector<bool>& visited) {
    visited[state] = true;
    for (int next : graph[state]) {
        if (!visited[next]) {
            DFS(next, graph, visited);
        }
    }
}


std::vector<bool> findDeadEndStates(const FSM& fsm, const std::vector<std::vector<int>>& graph) {
    std::vector<bool> isDeadEndState(fsm.numStates, true);
    
    for (int finalState : fsm.finalStates) {
        std::vector<bool> reachableFrom(fsm.numStates, false);
        DFS(finalState, graph, reachableFrom);
        for (int i = 0; i < fsm.numStates; i++) {
            if(reachableFrom[i]){
                isDeadEndState[i] = false;
            }
        }
    }
    
    return isDeadEndState;
}

int main() {
    FSM fsm = readFSM("./automata.txt"); 
    
    std::vector<std::vector<int>> graph(fsm.numStates);
    
    for (const Transition& t : fsm.transitions) {
        graph[t.prevState].push_back(t.nextState);
    }

    std::vector<bool> visited(fsm.numStates, false);
    DFS(fsm.startState, graph, visited);
    
    std::cout << "Unreachable states: ";
    for (int i = 0; i < fsm.numStates; i++) {
        if (!visited[i]) {
            std::cout << i << " ";
        }
    }

    std::cout << std::endl;

    std::vector<std::vector<int>> graphReversed(fsm.numStates);
    
    for (const Transition& t : fsm.transitions) {
        graphReversed[t.nextState].push_back(t.prevState);
    }

    std::vector<bool> DeadEndStates = findDeadEndStates(fsm, graphReversed);

    std::cout << "Dead end states: ";
    for (int i = 0; i < fsm.numStates; i++) {
        if (DeadEndStates[i]) {
            std::cout << i << " ";
        }
    }
    std::cout << std::endl;

    std::cin.get(); 

    return 0;
}
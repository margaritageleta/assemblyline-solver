//
// Margarita Geleta
//
#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <chrono>

using namespace std;
/* ==Type definitions== */
typedef vector<vector<int>> matrix;
typedef pair<int, int> Station;             // C_e and N_e.
                                            // Pre: C_e <= N_e.

/* ==Global variables== */
int min_cost = numeric_limits<int>::max();          // Time variable.
auto start = chrono::high_resolution_clock::now();  // Time variable.

vector<int> LINE;
matrix UPGRADES;
vector<Station> STATIONS;
vector<int> VEHICLES;
int C, M, K;
char* argv_2;

/* ==Functions== */
// Write down a given Assembly Line:
void process1 (const vector<int>& line);
// Write down a given Assembly Line, its penal and computation time in an output file:
void process (const vector<int>& solution);
// Compute the penalization of an Assembly Line in a given range for a given Upgrade:
int compute_cost (const vector<int>& line, int upgrade, int lower_bound, int upper_bound);
// Compute the penalization for a given Assembly Line, according to a given Upgrade:
int penalization (const vector<int>& line, int upgrade, int index);
// Compute the extra cost to the total. If -1 then exceeded.
int add (const vector<int>& line, int index, int cost);
// Generate Assembly Lines for a given Multiplicity of each class of vehicles:
void generate (vector<int>& multiplicity, vector<int>& solution, int index, int cost);
// Set the Initial Conditions for generating Assembly Lines:
void assembly ();

//=========================================================================//
void process1 (const vector<int>& line) {
    cout << "{";
    for (int i = 0; i < int(line.size()); ++i)
        (i == 0)? cout << line[i] : cout << "," << line[i];
    cout << "}";
}
//=========================================================================//
void process (const vector<int>& solution) {
    
    chrono::duration<double> diff = chrono::high_resolution_clock::now() - start;
    
    ofstream out(argv_2);
    out.setf(ios::fixed);
    out.precision(6);
    out << min_cost << " " << diff.count() << endl;
    for (int i = 0; i < int(solution.size()); ++i)
        (i == 0)? out << solution[i] : out << " " << solution[i];
    out.close();
}
//=========================================================================//
int compute_cost (const vector<int>& line, int upgrade,
                  int lower_bound, int upper_bound) {
    int counter = 0;
    for (int i = lower_bound; i <= upper_bound; ++i) {
        if (UPGRADES[line[i]][upgrade] == 1) ++counter;
    }
    return counter;
}
//=========================================================================//
int penalization (const vector<int>& line, int upgrade, int index) {
    if (index < 0) return 0;
    
    int penal = 0;
    int counter = 0;
    int c_e = STATIONS[upgrade].first;
    int n_e = STATIONS[upgrade].second;
    
    int lower_bound = index - n_e + 1;
    int upper_bound = index;
    
    if (lower_bound <= 0) lower_bound = 0;
    counter = compute_cost(line, upgrade, lower_bound, upper_bound);
    if (counter > c_e) penal += counter - c_e;
    return penal;
}
//=========================================================================//
int add (const vector<int>& line, int index, int cost) {
    int extra_cost = 0;
    for (int upgrade = 0; upgrade < M; ++upgrade) {
        extra_cost += penalization (line, upgrade, index);
        
        if (extra_cost + cost >= min_cost) return -1;
    }
    return extra_cost;
}
//=========================================================================//
void generate (vector<int>& multiplicity, vector<int>& line, int index,
               int cost) {
    if (index == C) {
        for (int upgrade = 0; upgrade < M; ++upgrade) {
            int penal = 0;
            int counter = 0;
            int c_e = STATIONS[upgrade].first;
            int n_e = STATIONS[upgrade].second;
            int lower_bound = index - n_e + 1;
            int upper_bound = index - 1;
            
            for (int i = lower_bound; i <= upper_bound; ++i) {
                counter += compute_cost(line, upgrade, i, upper_bound);
                if (counter > c_e) penal += counter - c_e;
                if (penal + cost >= min_cost) return;
                counter = 0;
            }
            cost += penal;
        }
        min_cost = cost;
        process(line);
        LINE = line;
    }
    
    else {
        for (int k = 0; k < K; ++k) {
            if (multiplicity[k] > 0) {
                line[index] = k;
                --multiplicity[k];
                int extra_cost = add(line, index, cost);
                if (extra_cost != -1) generate(multiplicity, line, index + 1, cost + extra_cost);
                ++multiplicity[k];
                line[index] = -1;
            }
        }
    }
}
//=========================================================================//
void assembly () {
    // Vector which will contain the assembly line of vehicles.
    vector<int> line (C, -1);
    
    start = chrono::high_resolution_clock::now();
    generate (VEHICLES, line, 0, 0);
}
//=========================================================================//
int main (int argc, char** argv) {
    ifstream in(argv[1]);
    
    in >> C >> M >> K;
    UPGRADES = matrix (K, vector<int>(M));
    VEHICLES = vector<int> (K);
    
    STATIONS = vector<Station> (M);
    
    for (int i = 0; i < M; ++i) in >> STATIONS[i].first;
    for (int i = 0; i < M; ++i) in >> STATIONS[i].second;
    
    int type;
    for (int i = 0; i < K; ++i) {
        in >> type;
        in >> VEHICLES[i];
        for (int upgrade = 0; upgrade < M; ++upgrade) in >> UPGRADES[i][upgrade];
    }
    
    LINE = vector<int>();
    
    argv_2 = argv[2];
    assembly ();
}
//=========================================================================//

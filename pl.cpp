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
/* ==Constants== */

/* ==Global variables== */
int min_cost = numeric_limits<int>::max();
auto start = chrono::high_resolution_clock::now();

vector<int> LINE;
matrix UPGRADES;
vector<Station> STATIONS;
vector<int> VEHICLES;
int C, M, K;
char* argv_2;

/* ==Functions== */

// Write down a given Assembly Line:
void process1 (const vector<int>& line);
// Compute the penalization for a given Assembly Line, according to a given Upgrade:
int penalization (const vector<int>& line, int upgrade, int index);
// Generate Assembly Lines for a given Workload:
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
    //cout << "computing cost" << endl;
    int counter = 0;
    for (int i = lower_bound; i <= upper_bound; ++i) {
        //cout << "line[i] "<< line[i] << endl;
        //cout << "UPGRADES[line[i]][upgrade] "<< UPGRADES[line[i]][upgrade] << endl;
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
    
    //cout << "<" << lower_bound << "," << upper_bound << ">:= ";
    
    if (lower_bound <= 0) lower_bound = 0;
    counter = compute_cost(line, upgrade, lower_bound, upper_bound);
    if (counter > c_e){
        //cout << "ok" << endl;
        penal += counter - c_e;
    }
    return penal;
}
//=========================================================================//
int add (const vector<int>& line, int index, int cost) {
    int extra_cost = 0;
    for (int upgrade = 0; upgrade < M; ++upgrade) {
        //cout << "check penal of upgrade " << upgrade << " on index " << index << endl;
        extra_cost += penalization (line, upgrade, index);
        
        if (extra_cost + cost >= min_cost) {
            //cout << "EXCEEDED" << endl;
            return -1;
        }
    }
    //cout << extra_cost << endl;
    return extra_cost;
}
//=========================================================================//
void generate (vector<int>& multiplicity, vector<int>& line, int index,
               int cost) {
    
    //cout << endl;
    if (index == C ) {
        for (int upgrade = 0; upgrade < M; ++upgrade) {
            //cout << "y" << endl;
            int penal = 0;
            int counter = 0;
            int c_e = STATIONS[upgrade].first;
            int n_e = STATIONS[upgrade].second;
            int lower_bound = index - n_e + 1;
            int upper_bound = index - 1;
            
            for (int i = lower_bound; i <= upper_bound; ++i) {
                //cout << "<" << i << "," << upper_bound << "> :=: ";
                counter += compute_cost(line, upgrade, i, upper_bound);
                if (counter > c_e) penal += counter - c_e;
                //cout << penal << endl;
                if (penal + cost >= min_cost) return;
                counter = 0;
            }
            cost += penal;
        }
        
        min_cost = cost;
        // cout << ">>> MIN COST: " << min_cost << endl;
        process(line);
        
        LINE = line;
    }
    
    else {
        for (int k = 0; k < K; ++k) {
            if (multiplicity[k] > 0) {
                line[index] = k;
                --multiplicity[k];
                
                //cout << "   COST: " << cost << endl;
                //cout << endl;
                //process1 (line);
                //cout << endl;
                int extra_cost = add(line, index, cost);
                
                if (extra_cost != -1) {
                    generate(multiplicity, line, index + 1, cost + extra_cost);
                }
                ++multiplicity[k];
                line[index] = -1;
            }
        }
    }
}
//=========================================================================//
void assembly () {
    vector<int> multiplicity (K);        // Contains how many cars of each class K.
    for (int i = 0; i < K; ++i) multiplicity[i] = VEHICLES[i];
    
    vector<int> line (C, -1);  // Vector which will contain the assembly line of vehicles.
    
    start = chrono::high_resolution_clock::now();
    generate (multiplicity, line, 0, 0);
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
    
    int cost = 0;
    for (int i = 0; i < int(LINE.size()); ++i) {
        cout << "index " << i << endl;
        for (int u = 0; u < M; ++u) {
            cout << "cost update " << u << " is " << penalization(LINE, u, i) << endl;
            cost += penalization(LINE, u, i);
        }
        if (i == int(LINE.size()) - 1){
                for (int u = 0; u < M; ++u) {
                    int penal = 0;
                    int counter = 0;
                    int c_e = STATIONS[u].first;
                    int n_e = STATIONS[u].second;
                    int lower_bound = i - n_e + 1;
                    int upper_bound = i - 1;
                    
                    for (int index = lower_bound; index <= upper_bound; ++index) {
                        // cout << "<" << i << "," << upper_bound << "> :=: ";
                        counter += compute_cost(LINE, u, index, upper_bound);
                        if (counter > c_e) penal += counter - c_e;
                        counter = 0;
                    }
                    cost += penal;
                    cout << "COST UP 2 NOW: " << cost << endl;
                    cout << endl;
                }
        }
        cout << "COST UP 2 NOW: " << cost << endl;
        cout << endl;
    }
    cout << endl;
    cout << "Total penal is " << cost << endl;
    
    cout << endl;
}
//=========================================================================//

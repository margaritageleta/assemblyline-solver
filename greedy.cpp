#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <chrono>
#include <stdlib.h>

using namespace std;
/* ==Type definitions== */
typedef vector<vector<int>> matrix;
typedef pair<int, int> Station;             // C_e and N_e.
// Pre: C_e <= N_e.

/* ==Global variables== */
int min_cost = 0;
auto start = chrono::high_resolution_clock::now();
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
    for (int i = lower_bound; i <= upper_bound; ++i)
        if (UPGRADES[line[i]][upgrade] == 1) ++counter;
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
    if (counter > c_e){
        penal += counter - c_e;
    }
    return penal;
}
//=========================================================================//
int class_with_max_cars () {
    int max_k = INT_MIN;
    int max = -1;
    for (int i = 0; i < K; ++i) {
        if (VEHICLES[i] > max_k) {
            max_k = VEHICLES[i];
            max = i;
        } }
    return max;
}
//=========================================================================//
int class_with_max_cars (int k1, int k2) {
    // Choose k1 if k1 has more cars.
    if (VEHICLES[k1] > VEHICLES[k2]) return k1;
    // tie?
    return k2;
}
//=========================================================================//
// Chooses minimizing the penalization.
// In case of a tie, chooses the class with more cars.
int choose (int index, vector<int>& line) {
    // Approach: choose the class with more cars:
    if (index == 0)  return class_with_max_cars();
    int min_c = INT_MAX;        // Minimum cost found up to now in this call.
    int min_k = -1;             // The class with minimum cost found up 2 now.
    for (int k = 0; k < K; ++k) {
        if (VEHICLES[k] > 0) {
            int cost_k = 0;     // Cost of that class of vehicle.
            line[index] = k;    // Insert the vehicle to be able to compute the costs.
            for (int u = 0; u < M; ++u) {
                // Special case for last position:
                if (index == int(line.size()) - 1) {
                    int penal = 0;
                    int counter = 0;
                    int c_e = STATIONS[u].first;
                    int n_e = STATIONS[u].second;
                    int lower_bound = index - n_e + 1;
                    int upper_bound = index;
                    // Calculate the cost of last position:
                    for (int i = lower_bound; i <= upper_bound; ++i) {
                        counter += compute_cost(line, u, i, upper_bound);
                        // If exceeds, then we have a penalization:
                        if (counter > c_e) penal += counter - c_e;
                        counter = 0;
                    }
                    cost_k += penal; // Increase the cost by the penalizations found.
                }
                // For the rest of positions:
                else {
                    int penal = penalization(line, u, index);
                    cost_k += penal;
                    if (cost_k > min_c) break;
                }
            }
            // Case when class k has less cost than the class chosen currently:
            if (cost_k < min_c) {
                if (min_c != INT_MAX) min_cost -= min_c;
                min_c = cost_k;
                // Increase minimum cost by cost of class k and
                // set the class of minimum cost equal to k.
                min_cost += min_c;
                min_k = k;
            }
            // In case of a tie, we choose the class with more cars
            // in order to try to distribute cars uniformly.
            else if (cost_k == min_c) {
                int k2 = class_with_max_cars (k, min_k);
                if (k2 == k) {
                    min_cost -= min_c;
                    min_c = cost_k;
                    // Increase minimum cost by cost of class k and
                    // set the class of minimum cost equal to k.
                    min_cost += min_c;
                    min_k = k;
                } } } }
    return min_k;
}
//=========================================================================//
void generate (vector<int>& line) {
    int end = line.size();
    for (int index = 0; index < end; ++index) {
        int value = choose(index, line);
        line[index] = value;
        --VEHICLES[value];
        
        process(line);
        process1(line);
        cout << " " << min_cost << endl;
    }
}
//=========================================================================//
void assembly () {
    vector<int> multiplicity (K);        // Contains how many cars of each class K.
    for (int i = 0; i < K; ++i) multiplicity[i] = VEHICLES[i];
    
    vector<int> line (C, -1);  // Vector which will contain the assembly line of vehicles.
    start = chrono::high_resolution_clock::now();
    generate (line);
}
//=========================================================================//
int main (int argc, char** argv) {
    srand (time(NULL));
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
    
    argv_2 = argv[2];
    assembly ();
    
    cout << endl;
}
//=========================================================================//
/* RESULTS FOR STRATEGIES
            #1  #2  opt
 ----------------------
    MED-1   16  25  9
    MED-2   37  47  15
    MED-3   62  71  38
    MED-4   35  35  25
    MED-5   31  33  16
    MED-6   51  51  25
    MED-7   24  24  14
    MED-8   32  33  10
    MED-9   42  34  12
    MED-10  14  14  9
 
            #1  #2  opt
 ----------------------
    HRD-1   480 468
    HRD-2   74  73
    HRD-3   39  43
    HRD-4   77  103
    HRD-5   136 140
    HRD-6   44  51
    HRD-7   71  71
    HRD-8   91  103
    HRD-9   320 316
    HRD-10  266 268
*/






















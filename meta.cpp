#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <chrono>
#include <stdlib.h>
#include <cmath>

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
//=========================================================================//
void process1 (const vector<int>& line) {
    cout << "{";
    for (int i = 0; i < int(line.size()); ++i)
        (i == 0)? cout << line[i] : cout << "," << line[i];
    cout << "}";
}
//=========================================================================//
void process (const vector<int>& solution, int c) {
    
    chrono::duration<double> diff = chrono::high_resolution_clock::now() - start;
    
    ofstream out(argv_2);
    out.setf(ios::fixed);
    out.precision(6);
    out << c << " " << diff.count() << endl;
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
// energy == cost.
double probability (int cost, int new_cost, double T) {
    // If the new solution is better, accept it.
    if (new_cost < cost) return 1.0;
    // Probability by Boltzmann distribution:
    return exp((cost - new_cost) / T);
}
//=========================================================================//
int cost (const vector<int>& solution) {
    int c = 0;
    for (int i = 0; i < int(solution.size()); ++i) {
        for (int u = 0; u < M; ++u) {
            if (i == int(solution.size()) - 1){
                int penal = 0;
                int counter = 0;
                int c_e = STATIONS[u].first;
                int n_e = STATIONS[u].second;
                int lower_bound = i - n_e + 1;
                int upper_bound = i;
                    
                for (int index = lower_bound; index <= upper_bound; ++index) {
                    counter += compute_cost(solution, u, index, upper_bound);
                    if (counter > c_e) penal += counter - c_e;
                    counter = 0;
                }
                c += penal;
            }
            else c += penalization(solution, u, i);
        }
    }
    return c;
}
//=========================================================================//
vector<int> generate_initial_solution(int n) {
    vector<int> initial_solution (n);
    int index = 0;
    while (index < n) {
        int k = rand() % K;
        if (VEHICLES[k] > 0) {
            initial_solution[index] = k;
            --VEHICLES[k];
            ++index;
        }
    }
    cout << "INITIAL SOLUTION SPAWN: ";
    process1(initial_solution);
    cout << endl;
    return initial_solution;
}
//=========================================================================//
vector<int> random_neighbour_of (const vector<int>& solution) {
    int N = solution.size();
    int pos1 = rand() % N;
    int pos2 = rand() % N;
    vector<int> new_solution;
    
    if (pos1 == pos2) new_solution = random_neighbour_of (solution);
    
    int value1 = solution[pos1];
    int value2 = solution[pos2];
    
    new_solution = solution;
    // swap.
    new_solution[pos1] = value2;
    new_solution[pos2] = value1;
    
    //cout << "TAKE A NEIGHBOR: ";
    process1(new_solution);
    cout << endl;
    return new_solution;
}
//=========================================================================//
vector<int> local_search (const vector<int>& solution, double& T0) {
    cout << "s'' = ";
    vector<int> new_solution = random_neighbour_of (solution);
    int energy = cost(solution);
    int new_energy = cost(new_solution);
    
    double prob = probability(energy, new_energy, T0);
    double choose = rand()/(RAND_MAX + 1.0);
    
    if (prob > choose) return new_solution;
    return solution;
}
//=========================================================================//
void generate (vector<int>& solution) {
    cout << "S = ";
    solution = generate_initial_solution(int(solution.size()));
    
    int k_limit = 10000; // Neighborhood limit.
    int k = 1;
    
    // Simulated Annealing parameters
    double T0 = 10000;       // Initial Temperature of the system.
    double T_cool_system = 0; // Minimum temperature;
    double alpha = 0.6;     // Cooling rate;
    
    vector<int> new_solution, new_solution2;
    while (k < k_limit and T0 > T_cool_system) {
        cout << "Neighborhood " << k << endl;
        cout << "Temperature " << T0 << endl;
        
        cout << "s' = ";
        new_solution = random_neighbour_of (solution);
        new_solution2 = local_search(new_solution, T0);
        T0 *= alpha; // Cool the system.
        
        int cost_sol = cost(solution);
        int cost_new = cost(new_solution2);
        cout << "   COST Of CURRENT SOLUTION: " << cost_sol << endl;
        cout << "   COST Of NEW SOLUTION: " << cost_new << endl;
        if (cost_new < cost_sol) {
            solution = new_solution2;
            cout << "out print " << cost_new << endl;
            process (solution, cost_new);
            k = 1;
        }
        else {
            cout << "go to another neighborhood" << endl;
            k = k + 1;
        }
        cout << "------------------------------------------" << endl;
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
    
    // COST = vector<int> (K);
    assembly ();
    
    cout << endl;
}
//=========================================================================//
/* RESULTS FOR STRATEGIES
            gy      meta   exh
 ------------------------------
 MED-1      11      9       9
 MED-2      30      15      15
 MED-3      61      38      38
 MED-4      29      26      25
 MED-5      23      17      16
 MED-6      44      25      25
 MED-7      24      14      14
 MED-8      18      11      10
 MED-9      40      12      12
 MED-10     12      9       9
 
            gy      meta   exh
 ------------------------------
 HRD-1      19      13      -
 HRD-2      4       1       -
 HRD-3      2       0       -
 HRD-4      10      8      -
 HRD-5      116     66      -
 HRD-6      0       0       -
 HRD-7      0       0       -
 HRD-8      0       2       -
 HRD-9      196     143     -
 HRD-10     6       0       -
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <random>
#include <format>

using namespace std;

// Implement simulated annealing algorithm to optimize schedule.
class ScheduleAnnealer {
public:
    ScheduleAnnealer(vector<vector<string>>& schedule, int num_teams) 
        : schedule(schedule), num_teams(num_teams), schedule_length(schedule.size()), distribution(0.0, 1.0) {
        setup_annealer();
        srand(1); // TODO - change seed to be random unless specified
    }

    void anneal() {
        cout << "Initial Cost: " << calculate_schedule_cost() << endl;
        cout << "Initial Gap Distribution: " << calculate_gap_dist() << " (min/25/50/75/max)" << endl;
        
        int iters_per_print = iters_per_temp / 10;
        float cum_cost = calculate_schedule_cost();
        
        for (float temperature = initial_temperature; temperature > min_temperature; temperature *= cooling_rate) {
            int num_accepted = 0;
            for (int iter = 0; iter < iters_per_temp; iter++) {
                vector<int> candidate_swap = choose_swap();
                int idx1 = candidate_swap[0];
                int idx2 = candidate_swap[1];
                float swap_cost_change = calculate_cost_change(idx1, idx2);
                if (distribution(generator) < exp(-swap_cost_change / temperature)) {
                    num_accepted++;
                    cum_cost += swap_cost_change;
                    // Update the team_to_games map.
                    for (int i = 0; i < 2; i++) {
                        string team1 = schedule[idx1][i];
                        string team2 = schedule[idx2][i];
                        
                        if (team1 != schedule[idx2][0] && team1 != schedule[idx2][1]) {
                            team_to_games[team1].erase(idx1);
                            team_to_games[team1].insert(idx2);
                        }
                        if (team2 != schedule[idx1][0] && team2 != schedule[idx1][1]) {
                            team_to_games[team2].erase(idx2);
                            team_to_games[team2].insert(idx1);
                        }
                    }
                    swap(schedule[idx1], schedule[idx2]);
                }

                if (iter % iters_per_print == 0 && iter != 0) {
                    cout << "Iteration " << iter << " for temperature " << temperature << ": Acceptance Rate = " 
                        << (float) num_accepted / iters_per_print << endl;
                    num_accepted = 0;
                }
            }
        }
        
        cout << "Final Cost: " << calculate_schedule_cost() << endl;
        cout << "Final Gap Distribution: " << calculate_gap_dist() << " (min/25/50/75/max)" << endl;
    }

private:
    /*=========================
        Schedule Data
      =========================*/
    vector<vector<string>>& schedule;
    int num_teams;
    int schedule_length;

    /*=========================
        Annealing Parameters
      =========================*/
    // Control how much to punish large deviations from the target number of games played. Specifically, the 
    // deviation from the target number of games played is raised to the deviation_exponent power. e.g. for 
    // deviation_exponent = 2.0, the cost of a given schedule is the total squared deviation from the target
    // games played.
    float deviation_exponent = 3.0;
    // Initial temperature for the simulated annealing algorithm.
    float initial_temperature = 100.0;
    // Number of iterations per temperature.
    int iters_per_temp = 100000;
    // Cooling rate for the simulated annealing algorithm.
    float cooling_rate = 0.3;
    // Temperature at which the annealing process stops.
    float min_temperature = 1.0;

    /*=========================
        Pre-Processing Data
      =========================*/
    // Track the indices of games for each team.
    unordered_map<string, set<int>> team_to_games;

    /*=========================
        Helper Functions
      =========================*/
    // Generator for random numbers.
    default_random_engine generator;
    uniform_real_distribution<double> distribution;
    
    // Pre-process schedule to improve efficiency of annealing algorithm.
    void setup_annealer() {
        for (int i = 0; i < schedule_length; i++) {
            string team1 = schedule[i][0];
            string team2 = schedule[i][1];
            if (team_to_games.find(team1) == team_to_games.end()) {
                set<int> games = {i};
                team_to_games[team1] = games;
            } else {
                team_to_games[team1].insert(i);
            }
            if (team_to_games.find(team2) == team_to_games.end()) {
                set<int> games = {i};
                team_to_games[team2] = games;
            } else {
                team_to_games[team2].insert(i);
            }
        }
    }

    // Choose a swap of indices.
    vector<int> choose_swap() {
        int idx1, idx2;
        do {
            idx1 = rand() % schedule_length;
            idx2 = rand() % schedule_length;
        } while (idx1 == idx2);
        return {idx1, idx2};
    }

    // This function returns the values at the closest index less than and greater than the given 
    // index (so, this function returns a vector with length 2).
    vector<int> get_lower_upper_bounds(set<int>& games, int idx, vector<int> idxs_to_exclude = {}) {
        // Determine the closest index greater than the given index.
        set<int>::iterator upper_bound_iter = upper_bound(games.begin(), games.end(), idx);
        // If the index is not allowed as the upper bound, push it forward one.
        while (find(idxs_to_exclude.begin(), idxs_to_exclude.end(), *upper_bound_iter) != idxs_to_exclude.end()) {
            upper_bound_iter = next(upper_bound_iter);
        }
        
        set<int>::iterator lower_bound_iter;
        int upper_bound;
        int lower_bound;
        
        if (upper_bound_iter == games.end() || *upper_bound_iter < idx) {
            // If there are no indices greater than the given index, the upper value is -1 and the 
            // lower index is the last index in the set.
            lower_bound_iter = prev(games.end());
            upper_bound = -1;
        } else {
            // Otherwise, the found index contains the upper value and the lower index is the index 
            // right before the upper index.
            lower_bound_iter = prev(upper_bound_iter);
            upper_bound = *upper_bound_iter;
        }

        // If the index is not allowed as the lower bound, push it backward one.
        while (lower_bound_iter != prev(games.begin()) && find(idxs_to_exclude.begin(), idxs_to_exclude.end(), *lower_bound_iter) != idxs_to_exclude.end()) {
            lower_bound_iter = prev(lower_bound_iter);
        }
        
        if (lower_bound_iter == prev(games.begin())) {
            // If the lower index is out of range, set the lower bound to -1.
            lower_bound = -1;
        } else {
            // If the lower index is in range, set the lower bound to the value at the lower index.
            lower_bound = *lower_bound_iter;
        }
        
        return {lower_bound, upper_bound};
    }

    // Calculate the change in cost from swapping the matchups at idx1 and idx2.
    float calculate_cost_change(int idx1, int idx2) {
        // If no swap is being made, the score won't change.
        if (idx1 == idx2) {
            return 0.0;
        }
        
        // Ensure that idx1 is less than idx2.
        if (idx1 > idx2) {
           swap(idx1, idx2);
        }

        vector<string> game1 = schedule[idx1];
        vector<string> game2 = schedule[idx2];
        
        float cost_change = 0.0;

        // Calculate the cost change for each team in the first game.
        for (string team : game1) {
            // If the team is in the second game as well, then the swap won't affect the cost.
            if (team == game2[0] || team == game2[1]) {
                continue;
            }

            set<int>& curr_games = team_to_games[team];

            // Determine the indices of games with this team to the left and right of this game (-1 if 
            // it doesn't exist).
            vector<int> from_bounds = get_lower_upper_bounds(curr_games, idx1, {idx1});
            int from_lower_bound_idx = from_bounds[0];
            int from_upper_bound_idx = from_bounds[1];

            // Determine the indices of games with this team to the left and right of the new swapped 
            // position of this game (-1 if it doesn't exist).
            vector<int> to_bounds = get_lower_upper_bounds(curr_games, idx2, {idx1});
            int to_lower_bound_idx = to_bounds[0];
            int to_upper_bound_idx = to_bounds[1];
            
            // First, calculate the change in cost of removing the current game.
            if (from_lower_bound_idx == -1) {
                assert(from_upper_bound_idx != -1);
                // By removing this game, the cost will be reduced by the cost based on the gap between this 
                // game and the next game with this team.
                cost_change -= cost_func(from_upper_bound_idx - idx1);
            } else if (from_upper_bound_idx == -1) {
                assert(from_lower_bound_idx != -1);
                // By removing this game, the cost will be reduced by the cost based on the gap between this 
                // game and the previous game with this team.
                cost_change -= cost_func(idx1 - from_lower_bound_idx);
            } else {
                // By removing this game, the cost will be reduced by the costs based on both the gap with the 
                // next game and the gap with the previous game.
                cost_change -= cost_func(from_upper_bound_idx - idx1) + cost_func(idx1 - from_lower_bound_idx);
                // Now, only a single gap remains - the gap between the next and previous games. Increase the cost 
                // based on this gap.
                cost_change += cost_func(from_upper_bound_idx - from_lower_bound_idx);
            }

            // Second, calculate the change in cost of adding the new game.
            if (to_lower_bound_idx == -1) {
                assert(to_upper_bound_idx != -1);
                // By adding this game, the cost will be increased by the cost based on the gap between this 
                // game and the next game with this team.
                cost_change += cost_func(to_upper_bound_idx - idx2);
            } else if (to_upper_bound_idx == -1) {
                assert(to_lower_bound_idx != -1);
                // By adding this game, the cost will be increased by the cost based on the gap between this 
                // game and the previous game with this team.
                cost_change += cost_func(idx2 - to_lower_bound_idx);
            } else {
                // By adding this game, the cost will be increased by the costs based on both the gap with the 
                // next game and the gap with the previous game.
                cost_change += cost_func(to_upper_bound_idx - idx2) + cost_func(idx2 - to_lower_bound_idx);
                // Now, the gap the next and previous games has been split. Decrease the cost based on the gap 
                // that was originally present (since that gap has now been split and no longer exists).
                cost_change -= cost_func(to_upper_bound_idx - to_lower_bound_idx);
            }
        }

        // Calculate the cost change for each team in the second game.
        for (string team : game2) {
            // If the team is in the first game as well, then the swap won't affect the cost.
            if (team == game1[0] || team == game1[1]) {
                continue;
            }

            set<int>& curr_games = team_to_games[team];

            // Determine the indices of games with this team to the left and right of this game (-1 if 
            // it doesn't exist).
            vector<int> from_bounds = get_lower_upper_bounds(curr_games, idx2, {idx2});
            int from_lower_bound_idx = from_bounds[0];
            int from_upper_bound_idx = from_bounds[1];

            // Determine the indices of games with this team to the left and right of the new swapped 
            // position of this game (-1 if it doesn't exist).
            vector<int> to_bounds = get_lower_upper_bounds(curr_games, idx1, {idx2});
            int to_lower_bound_idx = to_bounds[0];
            int to_upper_bound_idx = to_bounds[1];
            
            // First, calculate the change in cost of removing the current game.
            if (from_lower_bound_idx == -1) {
                assert(from_upper_bound_idx != -1);
                // By removing this game, the cost will be reduced by the cost based on the gap between this 
                // game and the next game with this team.
                cost_change -= cost_func(from_upper_bound_idx - idx2);
            } else if (from_upper_bound_idx == -1) {
                assert(from_lower_bound_idx != -1);
                // By removing this game, the cost will be reduced by the cost based on the gap between this 
                // game and the previous game with this team.
                cost_change -= cost_func(idx2 - from_lower_bound_idx);
            } else {
                // By removing this game, the cost will be reduced by the costs based on both the gap with the 
                // next game and the gap with the previous game.
                cost_change -= cost_func(from_upper_bound_idx - idx2) + cost_func(idx2 - from_lower_bound_idx);
                // Now, only a single gap remains - the gap between the next and previous games. Increase the cost 
                // based on this gap.
                cost_change += cost_func(from_upper_bound_idx - from_lower_bound_idx);
            }

            // Second, calculate the change in cost of adding the new game.
            if (to_lower_bound_idx == -1) {
                assert(to_upper_bound_idx != -1);
                // By adding this game, the cost will be increased by the cost based on the gap between this 
                // game and the next game with this team.
                cost_change += cost_func(to_upper_bound_idx - idx1);
            } else if (to_upper_bound_idx == -1) {
                assert(to_lower_bound_idx != -1);
                // By adding this game, the cost will be increased by the cost based on the gap between this 
                // game and the previous game with this team.
                cost_change += cost_func(idx1 - to_lower_bound_idx);
            } else {
                // By adding this game, the cost will be increased by the costs based on both the gap with the 
                // next game and the gap with the previous game.
                cost_change += cost_func(to_upper_bound_idx - idx1) + cost_func(idx1 - to_lower_bound_idx);
                // Now, the gap the next and previous games has been split. Decrease the cost based on the gap 
                // that was originally present (since that gap has now been split and no longer exists).
                cost_change -= cost_func(to_upper_bound_idx - to_lower_bound_idx);
            }
        }
        
        return cost_change;
    }

    // Calculate the cost of the entire schedule as a function of the distances between games.
    float calculate_schedule_cost(bool print_gaps = false) {
        float schedule_cost = 0.0;
        for (const auto& curr : team_to_games) {
            if (print_gaps) { cout << curr.first << ":"; }
            // Iterate over the set of games for the current team and calculate the cost.
            auto it = curr.second.begin();
            auto prev = it;
            ++it;
            for (; it != curr.second.end(); ++it) {
                int gap = *it - *prev;
                schedule_cost += cost_func(gap);
                if (print_gaps) { cout << " " << gap; }
                prev = it;
            }
            if (print_gaps) { cout << endl; }
        }
        return schedule_cost;
    }

    // Calculate distribution statistics for the gaps.
    string calculate_gap_dist() {
        vector<int> gaps;
        for (const auto& curr : team_to_games) {
            // Iterate over the set of games for the current team and calculate the gap.
            auto it = curr.second.begin();
            auto prev = it;
            ++it;
            for (; it != curr.second.end(); ++it) {
                int gap = *it - *prev;
                gaps.push_back(gap);
                prev = it;
            }
        }

        sort(gaps.begin(), gaps.end());

        int min_gap = *gaps.begin();
        int percentile_25_gap = *next(gaps.begin(), gaps.size() / 4);
        int percentile_50_gap = *next(gaps.begin(), gaps.size() / 2);
        int percentile_75_gap = *next(gaps.begin(), 3 * gaps.size() / 4);
        int max_gap = *prev(gaps.end());

        return to_string(min_gap) + '/' + to_string(percentile_25_gap) + '/' + to_string(percentile_50_gap) + '/' + to_string(percentile_75_gap) + '/' + to_string(max_gap);
    }

    // This is the cost function used to evaluate a single gap in the schedule.
    float cost_func(int gap) {
        // The target gap between conseuquent games for a given team is equal to num_teams / 2.
        return pow(abs((num_teams / 2.0) - abs(gap)), deviation_exponent);
    }

    // Print the current schedule.
    void print_schedule() {
        for (int i = 0; i < schedule_length; i++) {
            cout << schedule[i][0] << " vs " << schedule[i][1] << endl;
        }
    }

    void print_team_to_games() {
        for (const auto& curr : team_to_games) {
            cout << curr.first << ": ";
            for (int game : curr.second) {
                cout << game << " ";
            }
            cout << endl;
        }
    }
};

vector<vector<string>> parse_schedule(string schedule_str) {
    stringstream ss(schedule_str);
    vector<vector<string>> schedule;
    
    int split_loc;
    int i = 0;
    string t;
    while (getline(ss, t, ';')) {
        split_loc = t.find(',');
        string team1 = t.substr(0, split_loc);
        string team2 = t.substr(split_loc + 1);
        vector<string> game = {team1, team2};
        schedule.push_back(game);
        i++;
    }

    return schedule;
}


int main() {
    string schedule_str = "TB,NO;CHI,PIT;KC,NYJ;PIT,GB;DAL,NYJ;CAR,DAL;HOU,OAK;PIT,BAL;CLE,IND;MIN,BUF;IND,BAL;BAL,CIN;TB,DET;HOU,IND;NYJ,BUF;SF,SEA;PHI,WAS;MIA,TEN;CLE,CIN;WAS,CHI;DEN,CIN;NYG,CHI;CHI,DET;BAL,STL;MIN,SF;TB,ATL;NO,BAL;IND,OAK;WAS,PHI;NYG,PHI;WAS,DAL;NYG,DEN;ARI,MIN;GB,CLE;PHI,TEN;CIN,BUF;WAS,NYG;TB,BUF;CLE,KC;STL,ARI;KC,SF;CAR,NYG;DEN,NE;MIN,NO;OAK,JAX;SF,ATL;CLE,PHI;TB,PHI;NYG,WAS;SD,DEN;TB,CLE;SEA,SF;CIN,OAK;CHI,CIN;IND,HOU;NYG,DAL;NYJ,ARI;JAX,SD;TEN,DAL;BUF,ATL;PHI,CIN;ATL,STL;ATL,CHI;SEA,NO;SEA,ARI;TEN,CIN;CIN,CLE;CIN,IND;CIN,NYG;KC,SD;SF,PIT;OAK,SD;ARI,KC;CAR,ATL;CAR,NO;OAK,GB;STL,MIA;ARI,HOU;CAR,WAS;KC,DEN;CAR,MIA;CIN,BAL;NO,ATL;GB,DET;SF,BAL;SF,WAS;JAX,DEN;CIN,PIT;JAX,DET;JAX,SF;GB,TEN;GB,ATL;TEN,KC;ATL,SD;MIA,KC;HOU,JAX;WAS,GB;HOU,NE;SD,OAK;STL,NE;TEN,STL;OAK,KC;KC,OAK;NE,SEA;CLE,PIT;PIT,MIN;BAL,NYJ;CHI,STL;MIN,TB;NYG,JAX;WAS,TB;NYG,DET;CHI,SEA;BAL,CLE;SD,KC;CAR,TB;GB,MIN;DEN,OAK;ARI,NYG;DAL,WAS;NE,BUF;SD,BAL;SF,STL;CIN,CAR;STL,SF;NYJ,SD;HOU,TEN;MIN,DET;NO,PHI;SF,NYG;NYJ,MIA;SF,ARI;DET,SEA;ARI,SEA;DAL,NO;NE,DAL;SD,TEN;MIA,CHI;STL,TB;SEA,HOU;BUF,BAL;NE,CLE;GB,CHI;BUF,JAX;CLE,NO;TB,CAR;SEA,STL;IND,TB;NE,MIA;DEN,PHI;WAS,OAK;SD,NYG;STL,SEA;BUF,MIA;HOU,MIA;DET,IND;PIT,CLE;NO,SF;STL,CAR;PIT,CIN;ARI,SF;JAX,NE;CLE,BAL;BAL,PIT;DET,MIN;BAL,ATL;ATL,NO;PHI,NYG;MIN,NE;DEN,CLE;NE,SD;NYJ,DEN;ARI,DET;MIN,CHI;GB,ARI;PHI,DAL;DAL,NYG;DAL,MIN;BUF,NO;OAK,CAR;SD,WAS;HOU,MIN;SEA,JAX;PIT,IND;DAL,PHI;BUF,NE;DAL,ARI;ATL,NYJ;PHI,CAR;PHI,GB;WAS,IND;NYJ,HOU;OAK,DEN;CLE,TEN;DET,KC;NYJ,NE;JAX,TEN;MIA,NYJ;GB,DAL;NE,PIT;MIN,GB;BAL,MIA;CHI,DAL;ATL,TB;DET,GB;PIT,HOU;NO,OAK;IND,TEN;TEN,IND;NO,GB;CHI,MIN;SD,CIN;CHI,GB;ATL,CAR;SEA,ATL;DAL,DEN;IND,JAX;ARI,STL;NYG,SEA;PHI,STL;ATL,WAS;OAK,NYJ;MIA,DET;SEA,PHI;NYJ,WAS;TEN,HOU;TEN,JAX;TB,ARI;MIA,BUF;IND,ARI;KC,HOU;MIA,DEN;NO,CAR;NO,TB;BUF,NYJ;KC,PIT;DET,CHI;NE,NYJ;JAX,HOU;DET,CAR;TEN,PIT;DEN,SEA;HOU,CHI;DEN,SD;BAL,JAX;MIA,NE;OAK,BUF;STL,MIN;IND,BUF;DET,SF;CAR,SD;BUF,CLE;JAX,IND;PIT,MIA;DEN,KC;KC,TB";
    //string schedule_str = "C,D;B,C;A,C;A,D;B,D;A,B";
    vector<vector<string>> schedule = parse_schedule(schedule_str);
    ScheduleAnnealer annealer = ScheduleAnnealer(schedule, 32);
    annealer.anneal();

    return 0;
}
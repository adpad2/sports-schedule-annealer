#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
using namespace std;

// Implement simulated annealing algorithm to optimize schedule.
class ScheduleAnnealer {
    public:
        ScheduleAnnealer(vector<vector<string>>& schedule, int num_teams) 
            : schedule(schedule), num_teams(num_teams), schedule_length(schedule.size()) {
            setup_annealer();
        }

        void anneal() {
            // Implement simulated annealing algorithm
        }

    private:
        vector<vector<string>>& schedule;
        int num_teams;
        int schedule_length;

        // Track the number of games each team has played up to a given point in time.
        unordered_map<string, vector<int>> team_to_cum_games;

        // Pre-process schedule to improve efficiency of annealing algorithm.
        void setup_annealer() {
            // Track the number of games each team has played so far.
            unordered_map<string, int> team_to_curr_games;
            
            string team1;
            string team2;
            for (int i = 0; i < schedule_length; i++) {
                team1 = schedule[i][0];
                team2 = schedule[i][1];
                // Update the number of games played by team1.
                if (team_to_curr_games.find(team1) != team_to_curr_games.end()) {
                    team_to_curr_games[team1]++;
                } else {
                    team_to_curr_games[team1] = 1;
                    vector<int> games_for_curr_team(schedule_length, 0);
                    team_to_cum_games[team1] = games_for_curr_team;
                }
                // Update the number of games played by team2.
                if (team_to_curr_games.find(team2) != team_to_curr_games.end()) {
                    team_to_curr_games[team2]++;
                } else {
                    team_to_curr_games[team2] = 1;
                    vector<int> games_for_curr_team(schedule_length, 0);
                    team_to_cum_games[team2] = games_for_curr_team;
                }
                
                // Update the number of games played by each team up to this point for each team that has played games.
                for (const auto& curr_team : team_to_curr_games) {
                    team_to_cum_games[curr_team.first][i] = curr_team.second;
                }
            }
            
            for (int i = 0; i < 256; i++) {
                cout << schedule[i][0] << " vs " << schedule[i][1] << endl;
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
    vector<vector<string>> schedule = parse_schedule(schedule_str);
    ScheduleAnnealer annealer = ScheduleAnnealer(schedule, 32);

    return 0;
}
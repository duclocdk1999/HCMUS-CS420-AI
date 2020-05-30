// Wumpus level 1

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
//-----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

class Position {
    private:
        int row, column;
    public:
        Position() {
            row = 0;
            column = 0;
        }
        Position(int newRow, int newColumn) {
            this->row = newRow;
            this->column = newColumn;
        }
        void print() {
            cout << setw(2);
            cout << row;
            cout << setw(7);
            cout << column << "\n";
        }
        ~Position() {
            cout << "give me your money, I will teach you how to meow\n";
        }
};
//---------------------------------------------------------------------------------------------
string environment[100][100];           // save the state of matrix (wumpus, pit, agent, ...)
int n = 0;                              // limitation of the matrix
string prediction[100][100];            // predict the postion of wumpus + pit
bool isWumpus[100][100];                // if that position is detected as wumpus 100% 
int startRow = 10, startColumn = 1;     // the start position of agent

int score = 0;                          // +100 if gold is found, -100 for shooting an arrow,
                                        // -10000 if meet wumpus or pit or trapped (after 150 rooms)
int step = -1;                           

int dcolumn[] = {-1, 1, 0, 0};          // horizontal change (+1 or -1 or keep the same column)
int drow[] = {0, 0, -1, 1};             // vertical change (+1 or -1 or keep the same row)

map<string, string> soundOf;            
map<string, string> mystery;

vector<Position*> pathRecord;
//---------------------------------------------
void initEnvironment(const int &size) {

    // bound the area where our agent move
    for (int i = 0; i <= size + 1; i++) 
        for (int j = 0; j <= size + 1; j++)
            environment[i][j] = "X";
    
    // init isWumpus
    for (int i = 0; i <= size + 1; i++) 
        for (int j = 0; j <= size + 1; j++) 
            isWumpus[i][j] = false;
}
//---------------------------------------------
void readData() {
    ifstream fi;
    fi.open ("input.txt");

    fi >> n;
    initEnvironment(n);
    fi.ignore();
    string str;
    for (int i = 1; i <= n; i++) 
        for (int j = 1; j <= n; j++) {
            if (j < n) 
                getline(fi, str, '.');
            else 
                getline(fi, str, '\n');
            if (str != "A")
                environment[i][j] = str;
            else {
                environment[i][j] = "-";
                startRow = i;
                startColumn = j;
            }
        }
    prediction[startRow][startColumn] = "A";
    fi.close();
}
//-----------------------------------------
void display(string array[100][100]) {

    // display the real value in array
    for (int i = 1; i < n + 1; i++) {
        for (int j = 1; j < n + 1; j++) {
            if (array[i][j] == "W" || array[i][j] == "P")
                cout << RED;
            else 
                if (array[i][j] != "-") {
                    cout << GREEN;
                }
            cout << array[i][j] << RESET;
            //cout << setw(4);
            for (int len = 0; len < 4 - array[i][j].size(); len++)
                cout << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}
//--------------------------------------------------------------------------
void print(string prediction[100][100], int currentRow, int currentColumn) {

    // display to keep track on the prediction and the available state
    cin.get();
    system("clear");
    display(environment);

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (i == currentRow && j == currentColumn) {
                cout << GREEN "A" << RESET << "   ";
            }
            else 
            if (prediction[i][j] == "A") {
                cout << GREEN "-" << RESET << "   ";
            }
            else 
            if (prediction[i][j] == "") {
                cout << WHITE "-" << RESET << "   ";
            }
            else {
                cout << RED << prediction[i][j] << RESET;
                for (int len = 0; len < 4 - prediction[i][j].length(); len++) {
                    cout << WHITE " " << RESET;
                }
            }
            // cout << setw(4);
        }
        cout << "\n";
    }
    cout << "score: " << score << "\n";
    cout << "step: " << step << "\n";
}
//-----------------------------------------------
int countAdjancentWumpus(int currentRow, int currentColumn) {
    // count the number of wumpus around the current state

    if (currentRow <= 0 || currentRow > n || currentColumn <= 0 || currentColumn > n) {
        return 0;
    }

    int numberOfWumpusAround = 0;
    for (int i = 0; i < 4; i++) {
        int nextRow = currentRow + drow[i];
        int nextColumn = currentColumn + dcolumn[i];

        if (environment[nextRow][nextColumn] == "W") {
            numberOfWumpusAround ++;
        }
    }
    return numberOfWumpusAround;
}
//-----------------------------------------------
int countAdjancentPit(int currentRow, int currentColumn) {
    // count the number of Pit around the current state

    if (currentRow <= 0 || currentRow > n || currentColumn <= 0 || currentColumn > n) {
        return 0;
    }

    int numberOfPitAround = 0;
    for (int i = 0; i < 4; i++) {
        int nextRow = currentRow + drow[i];
        int nextColumn = currentColumn + dcolumn[i];

        if (environment[nextRow][nextColumn] == "P") {
            numberOfPitAround ++;
        }
    }
    return numberOfPitAround;
}
//-----------------------------------------------
int countUnpredictedRoom(int currentRow, int currentColumn) {
    // count the number of rooms where we have not predicted around

    if (currentRow <= 0 || currentRow > n || currentColumn <= 0 || currentColumn > n) {
        return 0;
    }

    int numberOfUnpredictedRoom = 0;
    for (int i = 0; i < 4; i++) {
        int nextRow = currentRow + drow[i];
        int nextColumn = currentColumn + dcolumn[i];

        // if our next state is still in valid area
        if (environment[nextRow][nextColumn] != "X") {
            
            if (prediction[nextRow][nextColumn] == "") {
                numberOfUnpredictedRoom ++;
            }
        }
    }
    return numberOfUnpredictedRoom;
}
//-----------------------------------------------
bool shootArrow(int currentRow, int currentColumn) {
    string currentState = environment[currentRow][currentColumn];

    // if the position where we shoot the arrow does not have any wumpus, no scream appear
    if (currentState != "W") {
        return false;
    }   

    // no more wumpus if we shoot arrow. Therefore, update isWumpus array
    isWumpus[currentRow][currentColumn] = false;
    
    string newState = "";
    if (countAdjancentPit(currentRow, currentColumn) > 0) {
        newState += "B";
    }
    if (countAdjancentWumpus(currentRow, currentColumn) > 0) {
        newState += "S";
    }
    environment[currentRow][currentColumn] = newState;
    if (newState == "")
        environment[currentRow][currentColumn] = "-";

    string nextState = "";
    for (int i = 0; i < 4; i++) {
        int nextRow = currentRow + drow[i];
        int nextColumn = currentColumn + dcolumn[i];
        nextState = environment[nextRow][nextColumn];

        // check if our predicted state is not out of valid area
        if (nextState != "X") {

            // if next state is not a bit, let check to remove the wumpus sound
            if (nextState != "P") {
                if (countAdjancentWumpus(nextRow, nextColumn) == 0) {
                    int lastCharIndex = nextState.length();
                    nextState.erase(remove(nextState.begin(), nextState.end(), 'S'), nextState.end());
                    environment[nextRow][nextColumn] = nextState;

                    // if nothing remain, init it as an empty state
                    if (nextState == "") {
                        environment[nextRow][nextColumn] = "-";
                    }
                }
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------------------------
void updateWPState(string &upState, string &downState, string &leftState, string &rightState, int currentRow, int currentColum) {
    
    // WP situation
    if (upState == "WP") {
        if (downState != "P" && leftState != "P" && rightState != "P") {
            upState = "P";
            return;
        }
        if (downState != "W" && leftState != "W" && rightState != "W") {
            upState = "W";
            isWumpus[currentRow - 1][currentColum] = true;
            return;
        }
    }
    if (downState == "WP") {
        if (upState != "P" && leftState != "P" && rightState != "P") {
            downState = "P";
            return;
        }
        if (upState != "W" && leftState != "W" && rightState != "W") {
            downState = "W";
            isWumpus[currentRow + 1][currentColum] = true;
            return;
        }
    }
    if (leftState == "WP") {
        if (downState != "P" && upState != "P" && rightState != "P") {
            leftState = "P";
            return;
        }
        if (downState != "W" && upState != "W" && rightState != "W") {
            leftState = "W";
            isWumpus[currentRow][currentColum - 1] = true;
            return;
        }
    }
    if (rightState == "WP") {
        if (downState != "P" && leftState != "P" && upState != "P") {
            rightState = "P";
            return;
        }
        if (downState != "W" && leftState != "W" && upState != "W") {
            rightState = "W";
            isWumpus[currentRow][currentColum + 1] = true;
            return;
        }
    }

    // W situation
    if (upState == "W") {
        if (downState != "W" && leftState != "W" && rightState != "W") {
            isWumpus[currentRow - 1][currentColum] = true;
            return;
        }
    }
    if (downState == "W") {
        if (upState != "W" && leftState != "W" && rightState != "W") {
            isWumpus[currentRow + 1][currentColum] = true;
            return;
        }
    }
    if (leftState == "W") {
        if (upState != "W" && downState != "W" && rightState != "W") {
            isWumpus[currentRow][currentColum - 1] = true;
            return;
        }
    }
    if (rightState == "W") {
        if (upState != "W" && leftState != "W" && downState != "W") {
            isWumpus[currentRow][currentColum + 1] = true;
            return;
        }
    }
}
//---------------------------------------------------------------------------------------------------
void tried(int currentRow, int currentColumn, int distanceFromStart) {
    
    step = step + 1;
    print(prediction, currentRow, currentColumn);
    pathRecord.push_back(new Position(currentRow, currentColumn));

    // check to prevent our agent from making more than 150 steps
    if (step + distanceFromStart >= 149)
        return;

    // get the current state (empty, B sound from Pit or S sound from Wumpus)
    string currentState = environment[currentRow][currentColumn];

    // check if currentState has gold
    if (currentState[0] == 'G') {
        score = score + 100;
    }

    // try all possible movements without shooting any arrow
    for (int i = 0; i < 4; i++) {
        
        int next_column = currentColumn + dcolumn[i];
        int next_row = currentRow + drow[i];

        // if our next state has been visited before, let check if WP conflict
        if (prediction[next_row][next_column] == "A") {
            if (environment[next_row][next_column] == "BS" || environment[next_row][next_column] == "SB"
                || environment[next_row][next_column] == "GSB" || environment[next_row][next_column] == "GBS"
                || environment[next_row][next_column] == "S") {

                string upState, downState, leftState, rightState;
                
                upState = prediction[next_row - 1][next_column];
                downState = prediction[next_row + 1][next_column];
                leftState = prediction[next_row][next_column - 1];
                rightState = prediction[next_row][next_column + 1];

                // if they are out of valid area, let ignore them by assigning random string (not P, not W)
                if (next_row - 1 <= 0) {
                    upState = "cau"; 
                }
                if (next_row + 1 > n) {
                    downState = "la";
                }
                if (next_column - 1 <= 0) {
                    leftState = "do";
                }
                if (next_column + 1 > n) {
                    rightState = "ngoc";
                }
                // check if 4 states has been predicted. If not, we can't conclude W or P
                if (upState != "" && downState != "" && leftState != "" && rightState != "") {
                    // check conflict and update
                    updateWPState(upState, downState, leftState, rightState, next_row, next_column);
                    
                    // update prediction
                    prediction[next_row - 1][next_column] = upState;
                    prediction[next_row + 1][next_column] = downState;
                    prediction[next_row][next_column - 1] = leftState;
                    prediction[next_row][next_column + 1] = rightState;
                }
            }
        }

        // if our next state is exactly wumpus
        if (isWumpus[next_row][next_column] == true && countUnpredictedRoom(next_row, next_column) > 0) {
            if (step + distanceFromStart < 150) {
                bool killWumpus = shootArrow(next_row, next_column);
                score = score - 100;
                
                // if wumpus is killed with a scream
                if (killWumpus) {
                    prediction[next_row][next_column] = "A";
                    tried(next_row, next_column, distanceFromStart + 1);
                    step = step + 1;
                    pathRecord.push_back(new Position(next_row, next_column));
                    print(prediction, currentRow, currentColumn);
                }
            }
        }

        // if our next state is in valid area and has not been visited yet
        if (environment[next_row][next_column] != "X" && prediction[next_row][next_column] != "A") {
            
            // if current state is empty or only has gold, that mean there are no dangers around our room.
            if (currentState == "-" || currentState == "G") {
                prediction[next_row][next_column] = "A";

                if (step + distanceFromStart < 150) {
                    tried(next_row, next_column, distanceFromStart + 1);
                    step = step + 1;
                    pathRecord.push_back(new Position(currentRow, currentColumn));
                    print(prediction, currentRow, currentColumn);
                }
            }
            // if current state recieves Breeze or Stench -> Wumpus or Pit is located around
            else {
                
                // predictState is determined by what we received in our standing room
                string predictState = mystery[currentState];

                // we tried to predict the state of adjacent rooms (Wumpus or Pit or safe)
                for (int j = 0; j < 4; j++) {
                    int nextRow = currentRow + drow[j];
                    int nextColumn = currentColumn + dcolumn[j];

                    // check if our next state is not out of valid area
                    if (environment[nextRow][nextColumn] != "X") {
                        
                        // if our next state has not been visited yet
                        if (prediction[nextRow][nextColumn] != "A") {

                            // if the next state has not been predicted yet
                            if (prediction[nextRow][nextColumn] == "") {
                                prediction[nextRow][nextColumn] = predictState;
                                print(prediction, currentRow, currentColumn);
                            }
                            else 
                            // if the sound at the current room is conflict with the prediction of the next room
                            if (currentState != soundOf[prediction[nextRow][nextColumn]]) {
                                
                                // if current state is not BS
                                if (currentState != "BS" && currentState != "SB") {
                                    if (prediction[nextRow][nextColumn] != "WP") {
                                        prediction[nextRow][nextColumn] = "A";
                                        if (step + distanceFromStart < 150) {    
                                            tried(nextRow, nextColumn, distanceFromStart + 1);
                                            step = step + 1;
                                            pathRecord.push_back(new Position(currentRow, currentColumn));
                                            print(prediction, currentRow, currentColumn);
                                        }
                                    }
                                    else {
                                        prediction[nextRow][nextColumn] = predictState;
                                        print(prediction, currentRow, currentColumn);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

/*
    // try shooting an arrow
    if (step + distanceFromStart < 150) {
        for (int i = 0; i < 4; i++) {
            int nextRow = currentRow + drow[i];
            int nextColumn = currentColumn + dcolumn[i];

            // if our next state is in valid area and is predicted as Wumpus (100%)
            if (environment[nextRow][nextColumn] != "X" && prediction[nextRow][nextColumn] == "W" && isWumpus[nextRow][nextColumn]) {
                if (countUnpredictedRoom(nextRow, nextColumn) > 0) {
                    
                    // make sure we won't lost in cave
                    if (step + distanceFromStart < 150) {
                        bool killWumpus = shootArrow(nextRow, nextColumn);
                        score = score - 100;
                        
                        // if wumpus is killed with a scream
                        if (killWumpus) {
                            prediction[nextRow][nextColumn] = "A";
                            tried(nextRow, nextColumn, distanceFromStart + 1);
                            step = step + 1;
                            pathRecord.push_back(new Position(currentRow, currentColumn));
                            print(prediction, currentRow, currentColumn);
                        }
                    }
                }
            }
        }
    }
*/
}
//--------------------------------------------------------------------------------------------------
int main() {
    soundOf["P"] = "B";
    soundOf["W"] = "S";
    soundOf["WP"] = "BS";

    mystery["B"] = "P";
    mystery["S"] = "W";
    mystery["BS"] = "WP";
    mystery["SB"] = "WP";
    mystery["GBS"] = "WP";
    mystery["GB"] = "P";
    mystery["GS"] = "W";

    readData();
    display(environment);

    // shootArrow(3, 3);
    // display(environment);

    // wumpus's action
    tried(startRow, startColumn, 0);

    // display result
    score = score + 10;                         // climb out of cave
    cout << "\nfinal score: " << score << "\n";
    cout << "final step: " << step << "\n";

    // display path
    cin.get();
    cout << "path record:\n";
    cout << "row" << setw(10) << "column\n";
    for (int i = 0; i < pathRecord.size(); i++) {
        pathRecord[i]->print();
    }
    return 0;
}
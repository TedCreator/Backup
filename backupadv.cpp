#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <vector>
using namespace std;

fstream root;
string userenv = getenv("USERPROFILE");
string backupdir = userenv + R"(\Documents\BACKUP\)";
string rootdir = backupdir + "gamelocations.txt";
string backuploc;

class Game {
    public: 
        string name;
        int line;
        string savepath;
        Game(){
            line = 0;
        }
        Game(int place, string game, string path){
            name = game;
            line = place;
            savepath = path;
        }
        Game(string game, string path){
            name = game;
            line = -1;
            savepath = path;
        }
        string getName(){
            return name;
        }
        int getLine(){
            return line;
        }
        string getPath(){
            return savepath;
        }
        void setName(string game){
            name = game;
        }
        void setLine(int place){
            line = place;
        }
        void setPath(string path){
            savepath = path;
        }
        ~Game(){

        }
        const string toString(){
            string ret;
            ret += to_string(line); 
            ret += " " + name + ":" + savepath;
            return ret;
        }
};
vector<Game> games;

string searchGames(string name){
    ifstream root;
    root.open(rootdir);
    size_t pos;
    string line;
    int count = 1;
    if(root.is_open()){
        while(getline(root, line)){
            pos = line.find(name);
            if(pos!=string::npos){
                return line; 
            }
            count++;
        }
    }
    root.close();
    return "";
}
void addGame(){
    root.open(rootdir, ios::app);
    string input;
    string game;
    Game newGame;
    cout << "Enter the name of the game: ";
    cin >> input;
    newGame.setName(input);
    string searchedGame = searchGames(input);
    if(searchedGame.size() < 1){
        game = input + "{";
        cout << "Enter game save folder you want backed up: ";
        cin.ignore();
        getline(cin, input);
        newGame.setPath(input);
        game += input + "}\n";
        root << game;
    } else { 
        cout << "Game " + input + " already exists with path: " << searchedGame.substr(input.size() + 2, (searchedGame.size() - (input.size() + 2)) - 1) << endl;
        cout << " Editing.";
    }

}
void editGame(string game){  

}
void deleteGame(){

}
void fillVector(){
    string line;
    int pos;
    string name;
    string path;
    Game newGame;
    int count = 1;
    while (getline(root, line)) {  
        pos = line.find('{');
        name = line.substr(0, pos);
        // newGame.setName(div);
        // cout << line;
        path = line.substr(pos + 1);
        path = path.substr(0, path.size() - 1);
        newGame = Game(count, name, path);
        games.push_back(newGame);
        count++;
    }
    root.close();
}
void zipUp(string saveloc, string backloc){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);

    string date = to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                + "-" + to_string(utc_tm.tm_hour-5) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s";
    
    //change to use tar command, may have to add quotes around stuff
    string cmd = "cd " + saveloc + " & 7z a " + date + ".zip";
    cmd += " & move \"" + date + ".zip\" " + backloc;
    system(cmd.c_str());
}

int main(){
    char userOption = ' ';
    root.open(rootdir, ios::in);
    fillVector();
    for(Game i: games){
        cout << i.toString() << endl;
    }
    while(userOption != 'q'){
        cout << endl << "0. Add/Edit Game" << endl << "-. Go to previous page" << endl << "+. Go to next page" << endl << "q. to quit" << endl << "Select an option: ";
        cin >> userOption;
        switch(userOption){
            case '0':
                addGame();
            break;

            case '-':
            
            break;

            case '+':

            break;
            case 'q':
            exit(0);
        }
        userOption = 'q';
    }
    
    root.close();
}




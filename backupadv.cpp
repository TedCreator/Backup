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
        string toCommit(){
            return name + "{" + savepath + "}";
        }
        const string toString(){
            string ret;
            ret += to_string(line); 
            ret += " " + name + " : " + savepath;
            return ret;
        }
};
vector<Game> games;

string searchRoot(string name){
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
Game searchGames(string game){
   for(auto & element : games){
        if(element.getName() == game){
            return element;
        }
    }
    return Game(0, "", "");
}
void editGame(string game){  
    string newName;
    for(auto & element : games){
        if(element.getName() == game){
            cout << "To keep name, enter nothing and press enter." << endl << "Edit the game name: ";
            cin >> newName;
            if(newName != ""){
                element.setName(newName);
            }
            cout << "To keep path, enter nothing and press enter." << endl << "Edit the game path: ";
            
        }
    }
}
void findPosInVector(Game game){
    for(int i = 0; i < games.size(); i++){
        if(games.at(i).getName() == game.getName()){
            games[i] = game;
        }
    }
    cout << game.toString();
}
Game editGame(Game game){
    string newName;
    string newPath;
    cout << "To keep name, enter x and press enter." << endl << "Edit the game name: ";
    cin >> newName;
    if(newName != "x"){
        game.setName(newName);
    }
    cout << "To keep path, enter x and press enter." << endl << "Edit the game path: ";
    cin >> newPath;
    if(newPath != "x"){
        game.setPath(newPath);
    }
    // cout << "New game entry is: " << game.toString() << endl;
    return game;
}

void addGame(){
    string input;
    string game;
    Game newGame;
    cout << "Enter the name of the game: ";
    cin >> input;
    newGame.setName(input);
    Game searchedGame = searchGames(input);
    if(searchedGame.getName().size() < 1){
        newGame.setName(input);
        cout << "Enter game save folder you want backed up: ";
        cin >> input;
        newGame.setPath(input);
        games.push_back(newGame);
    } else { 
        cout << "Game " + input + " already exists with path: " << searchedGame.getPath() << endl;
        cout << "Editing." << endl;
        findPosInVector(editGame(searchedGame));
    }
}
void deleteGame(string game){
    for(auto & element : games){
        if(element.getName() == game){
            games.erase(games.begin()+element.getLine() - 1);
        }
    }
}
void fillVector(){
    root.open(rootdir, ios::in);
    string line;
    int pos;
    string name;
    string path;
    Game newGame;
    int count = 1;
    while (getline(root, line)) {  
        pos = line.find('{');
        name = line.substr(0, pos);
        path = line.substr(pos + 1);
        path = path.substr(0, path.size() - 1);
        newGame = Game(count, name, path);
        games.push_back(newGame);
        count++;
    }
    root.close();
}
void commit(){
    root.open(rootdir, ios::out);
    for(auto & element : games){
        root.clear();
        root << element.toCommit() << endl;
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
    fillVector();
    for(Game i: games){
        cout << i.toCommit() << endl;
    }
    while(userOption != 'q'){
        cout << endl << "0. Add/Edit Game" << endl << "s. to save changes" << endl <<
             "q. to quit" << endl << "x. to save and quit" << endl << "Select an option: ";

        cin >> userOption;
        switch(userOption){
            case '0':
                addGame();
                for(Game i: games){
        cout << i.toCommit() << endl;
    }
            break;
            case 's':
                commit();
            break;
            case 'x':
                commit();
                exit(0);
            case 'q':
                exit(0);
        }
        // userOption = 'q';
    }
}
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
string backuploc = "\"" + backupdir + "\"";

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
            ret += ". " + name + " : " + savepath;
            return ret;
        }
};
vector<Game> games;
string stlow(string str){
    char ch;
    string ret = "";
    for (int i = 0; i < str.size(); i++) {
        ret += tolower(str.at(i));
    }
    return ret;
}

Game searchGames(string game){
   for(auto & element : games){
        if(stlow(element.getName()) == stlow(game)){
            return element;
        }
    }
    return Game(0, "", "");
}
int findPosInVector(Game game){
    for(int i = 0; i < games.size(); i++){
        if(games.at(i).getName() == game.getName()){
            return i;
        }
    }
    return -1;
}
Game editGame(Game game){
    string newName;
    string newPath;
    cout << "To keep name, enter x. " << endl << "Edit the game name: ";
    cin >> newName;
    if(newName != "x"){
        game.setName(newName);
    }
    cout << "To keep path, enter x." << endl << "Edit the game path: ";
    cin >> newPath;
    if(newPath != "x"){
        game.setPath(newPath);
    }
    cout << "New game entry is: " << game.toCommit() << endl;
    return game;
}
void addGame(){
    string input;
    string game;
    Game newGame;
    cout << "Enter the name of the game: ";
    cin.ignore();
    getline(cin, input);
    newGame.setName(input);
    Game searchedGame = searchGames(input);
    if(searchedGame.getName().size() < 1){
        newGame.setName(input);
        cout << "Enter game save folder you want backed up: ";
        getline(cin, input);
        newGame.setPath(input);
        games.push_back(newGame);
    } else { 
        cout << "Game " + input + " already exists with path: " << searchedGame.getPath() << endl;
        cout << "Editing." << endl;
        newGame = editGame(searchedGame);
        games.at(findPosInVector(searchedGame)) = newGame;
    }
}
void deleteGame(){
    string game;
    cout << "What game do you want to delete? " << endl << "Enter the name: ";
    cin >> game;
    game = stlow(game);
    cout << game;
    // for(auto & element : games){
    //     if(element.getName() == game){
    //         games.erase(games.begin()+element.getLine() - 1);
    //     }
    // }
}
void fillVector(){
    root.open(rootdir, ios::in);
    string line;
    int pos;
    string name;
    string path;
    int count = 1;
    while (getline(root, line)) {  
        pos = line.find('{');
        name = line.substr(0, pos);
        path = line.substr(pos + 1);
        path = path.substr(0, path.size() - 1);
        games.push_back(Game(count, name, path));
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
void zipUp(string saveloc, string gameName){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);

    //file name idea is "nameOfGame month-day-year-?h?m?s"
    string fileName = gameName + " " + to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                + "-" + to_string(utc_tm.tm_hour-5) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s";
    
    //change to use tar command
    string cmd = "cd \"" + saveloc + "\" & 7z a \"" + fileName + ".zip\"";
    cmd += " & move \"" + fileName + ".zip\" " + backupdir;
    system(cmd.c_str());
}
int main(){
    string userOption = " ";
    fillVector();
    for(int i = 0; i < games.size(); i++){
        cout << games.at(i).toString() << endl;
    }

    cout << endl << "0. Add/Edit Game" << endl << "~. to delete a game" << endl << "s. to save changes" << endl 
         << "x. to save and quit" << endl << "q. to quit without saving" << endl << "a. to backup all to folder" << endl;
    while(userOption != "q"){
        cout << "Select an option: ";
        cin >> userOption;
        userOption = stlow(userOption);
        switch(userOption.at(0)){
            case '0':
                addGame();
                for(Game i: games){cout << i.toCommit() << endl;}
            break;
            case '~':
                deleteGame();
            break;
            case 'a':
                for(Game i: games){
                    
                    zipUp(i.getPath(), i.getName());
                }
                cout << endl << "Backed up game(s): ";
                for(Game i : games){ 
                    cout << i.getName() << " ";
                }
                cout << "to path " << backupdir << endl;
            break;
            case 'x':
                commit();
                exit(0);
            case 's':
            commit();
            break;
            case 'q':
                exit(0);
            default:
                int userNum;
                try {
                    userNum = stoi(userOption);
                    if(userNum > games.size()){
                        cout << "Not a valid game";
                    } else {
                        cout << games.at(userNum - 1).toString() << endl;
                    }
                } catch (invalid_argument){
                    cout << "Not a valid input " << endl;
                }
                
            break;
        }
        // userOption = "q"; // for testing 
    }
}

// wrote but not using
// string searchRoot(string name){
//     ifstream root;
//     root.open(rootdir);
//     size_t pos;
//     string line;
//     int count = 1;
//     if(root.is_open()){
//         while(getline(root, line)){
//             pos = line.find(name);
//             if(pos!=string::npos){
//                 return line; 
//             }
//             count++;
//         }
//     }
//     root.close();
//     return "";
// }
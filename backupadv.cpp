#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;

ofstream root;
string userenv = getenv("USERPROFILE");
string backupdir = userenv + R"(\Documents\BACKUP\)";
string rootdir = backupdir + "gamelocations.txt";
string backuploc;

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
    string input;
    string game;
    cout << "Enter the name of the game: ";
    cin >> input;
    string searchedGame = searchGames(input);
    if(searchedGame.size() < 1){
        game = input + " {";
        cout << "Enter game save folder you want backed up: ";
        cin.ignore();
        getline(cin, input);
        game += input + "}\n";
        root << game;
    } else { 
        // cout << searchedGame << searchedGame.size();
        cout << "Game " + input + " already exists with path: " << searchedGame.substr(input.size() + 2, (searchedGame.size() - (input.size() + 2)) - 1);
    }

}
void editGame(){  
}


void zipUp(string saveloc){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);

    string date = to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                + "-" + to_string(utc_tm.tm_hour-5) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s";
    
    //change to use tar command
    string cmd = "cd " + saveloc + " & 7z a " + date + ".zip";
    cmd += " & move \"" + date + ".zip\" " + backuploc;
    system(cmd.c_str());
}

int main(){
    char userOption = ' ';
while(userOption != 'q'){
        if(!filesystem::exists(backupdir + "gamelocations.txt")) {
            root.open(backupdir + "gamelocations.txt");
        } else {
            root.open(backupdir + "gamelocations.txt", ios::app);
        }
        
        cout << endl << "0. Add/Edit Game" << endl << "-. Go to previous page" << endl << "+. Go to next page" << endl << "q to quit" << endl << "Select an option: ";
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

        // zipUp(backupdir);
        root.close();
}
}




#include <string>
#include <chrono>
#include <iostream>
using namespace std;

//This file is here to show the general idea of the main algorithm and idea for the program. 

int main(){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);
    
    string date = to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                + "-" + to_string(utc_tm.tm_hour-5) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s";


    string savedir = R"(%USERPROFILE%\Documents\My Games\Tiny Tina's Wonderlands\Saved\SaveGames)";
    string backupdir = R"("C:\Games\BACKUP\Wonderlands")";

    string cmd = "cd " + savedir + " & 7z a " + date + ".zip";
    cmd += " & move \"" + date + ".zip\" " + backupdir;
    // cout << cmd;
    system(cmd.c_str());
}

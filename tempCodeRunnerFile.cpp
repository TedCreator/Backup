#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;

fstream root;
string backupdir = R"(C:\Games\BACKUP\)";
string rootdir = backupdir + "gamelocations.txt";
string backuploc;
int main(){
    cout << rootdir;
}
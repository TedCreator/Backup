#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <vector>
//------------TODO-----------------
//
//CHANGE text file to be JSON/another data storing file format.
//CHANGE zipping method to use a zipping (ziplib?) library with similar functionality to 7zip
//       also fixes/removes using unsafe, non-portable & code injection vulnerable system() method
//


using namespace std; //bad practice, will change later

fstream root;
string userEnv = getenv("USERPROFILE");
string backupDir = R"(F:\BACKUP\)";
string rootDir = backupDir + "entrylocations.txt"; 
string backupPath = "\"" + backupDir + "\"";

class Entry {
    public: 
        string name;
        int line;
        string savepath;
        Entry(){
            line = 0;
        }
        ~Entry(){
        }
        Entry(int place, string entry, string path){
            name = entry;
            line = place;
            savepath = path;
        }
        Entry(string entry, string path){
            name = entry;
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
        void setName(string entry){
            name = entry;
        }
        void setLine(int place){
            line = place;
        }
        void setPath(string path){
            savepath = path;
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


vector<Entry> entrys;
string stlow(string str){
    char ch;
    string ret = "";
    for (int i = 0; i < str.size(); i++) {
        ret += tolower(str.at(i));
    }
    return ret;
}
Entry searchEntrys(string entry){
   for(auto & element : entrys){
        if(stlow(element.getName()) == stlow(entry)){
            return element;
        }
    }
    return Entry(0, "", "");
}
int findPosInVector(Entry entry){
    for(int i = 0; i < entrys.size(); i++){
        if(entrys.at(i).getName() == entry.getName()){
            return i;
        }
    }
    return -1;
}
Entry editEntry(Entry entry){
    string newName;
    string newPath;
    cout << "To keep name, enter x. " << endl << "Edit the Entry name: ";
    cin >> newName;
    if(newName != "x"){
        entry.setName(newName);
    }
    cout << "To keep path, enter x." << endl << "Edit the Entry path: ";
    cin >> newPath;
    if(newPath != "x"){
        entry.setPath(newPath);
    }
    cout << "New entry is: " << entry.toCommit() << endl;
    return entry;
}
void addEntry(){
    string input;
    string entry;
    Entry newEntry;
    cout << "Enter the name of the entry: ";
    cin.ignore();
    getline(cin, input);
    newEntry.setName(input);
    Entry searchedEntry = searchEntrys(input);
    if(searchedEntry.getName().size() < 1){
        newEntry.setName(input);
        cout << "Enter file path to be backed up: ";
        getline(cin, input);
        newEntry.setPath(input);
        entrys.push_back(newEntry);
    } else { 
        cout << "Entry " + input + " already exists with path: " << searchedEntry.getPath() << endl;
        cout << "Editing." << endl;
        newEntry = editEntry(searchedEntry);
        entrys.at(findPosInVector(searchedEntry)) = newEntry;
    }
}
void deleteEntry(){
    string entry;
    cout << "What entry do you want to delete? " << endl << "Enter the name: ";
    cin >> entry;
    entry = stlow(entry);
    for(auto & element : entrys){
        if(element.getName() == entry){
            entrys.erase(entrys.begin()+element.getLine() - 1);
        }
    }
}
void fillVector(){
    root.open(rootDir, ios::in);
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
        entrys.push_back(Entry(count, name, path));
        count++;
    }
    root.close();
}
void dbCommit(){ //file database committing method
    root.open(rootDir, ios::out);
    for(auto & element : entrys){
        root.clear();
        root << element.toCommit() << endl;
    }
    root.close();
}
void zipUp(string saveloc, string entryName){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);

    //file name idea is "nameOfEntry month-day-year-?h?m?s" .zip
    string fileName = entryName + " " + to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                + "-" + to_string(utc_tm.tm_hour-5) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s";

    //idea command is "cd file-location 7z a"
    string cmd = "cd \"" + saveloc + "\" & 7z a \"" + fileName + ".zip\"";
    cmd += " & move \"" + fileName + ".zip\" " + backupDir;
    cout << cmd;
    system(cmd.c_str()); //unsafe, non-portable method & vulnerable to code injection, but using for concept's sake.
}
string prompt(){
    for(int i = 0; i < entrys.size(); i++){
        cout << entrys.at(i).toString() << endl;
    }
    cout << "-----------------------------------------------------------------------------------" << endl 
    << "0. Add/Edit Entry" << endl << "~. to delete a entry" << endl 
    << "s. to save changes (current entry list will overwrite any since last save)" << endl 
    << "q. to save and quit" << endl << "x. to quit without saving" << endl << "a. to backup all to folder" << endl;
    cout << "Select an option: ";
    string userOption;
    cin >> userOption;
    return userOption;
}
int main(){
    string userOption = " ";
    fillVector();
    

    while(userOption != "q"){
        userOption = prompt();
        userOption = stlow(userOption);

        switch(userOption.at(0)){
            case '0':
                addEntry();
                for(Entry i: entrys){
                    cout << i.toCommit() << endl;
                }
            break;
            case '~':
                deleteEntry();
            break;
            case 'a':
                for(Entry i: entrys){
                    
                    zipUp(i.getPath(), i.getName());
                }
                cout << endl << "Backed up entry(s): ";
                for(Entry i : entrys){ 
                    cout << "\"" << i.getName() << "\"" << " ";
                }
                cout << "to path " << backupDir << endl;
            break;
            case 'q':
                dbCommit();
                exit(0);
            case 's':
            dbCommit();
            break;
            case 'x':
                cout << " ** Are you sure you want to end program? This will not save any of your entry changes. ** " 
                     << endl << "Enter the letter y to confirm: ";
                cin >> userOption;
                if(stlow(userOption).at(0) != 'y'){
                    userOption = " ";
                    break;
                }
                exit(0);
            default:
                int userNum;
                try {
                    userNum = stoi(userOption);
                    if(userNum > entrys.size()){
                        cout << " ** Not a valid entry ** " << endl;
                    } else {
                        cout << entrys.at(userNum - 1).toString() << endl;
                        zipUp(entrys.at(userNum - 1).getPath(), entrys.at(userNum - 1).getName());
                        prompt();
                    }
                } catch (invalid_argument){
                    cout << " ** Not a valid input **" << endl;
                }
            break;
        }
        // userOption = "q"; // hardcode for testing 
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
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <vector>
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
//------------TODO-----------------
//
//CURR -> CHANGE text file to be JSON/another data storing file format.
//CHANGE zipping method to use a zipping (ziplib?) library with similar functionality to 7zip
//       also fixes/removes using unsafe, non-portable & code injection vulnerable system() method
//


using namespace std; //bad practice, will change later

fstream root;
string userEnv = getenv("USERPROFILE");
string backupDir = R"(F:\BACKUP\)";
string rootDir = backupDir + "entrylocations.txt"; 
string backupPath = "\"" + backupDir + "\"";

struct Entry {
        int line;
        string name;
        string filepath;
        Entry(){
            line = 0;
        }
        ~Entry(){
        }
        Entry(int place, string entry, string path){
            name = entry;
            line = place;
            filepath = path;
        }
        Entry(string entry, string path){
            name = entry;
            line = -1;
            filepath = path;
        }
        void setName(string entry){
            name = entry;
        }
        void setLine(int place){
            line = place;
        }
        void setPath(string path){
            filepath = path;
        }
        string toCommit(){
            return name + "{" + filepath + "}";
        }
        const string toString(){
            string ret;
            ret += to_string(line); 
            ret += ". " + name + " : " + filepath;
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
        if(stlow(element.name) == stlow(entry)){
            return element;
        }
    }
    return Entry(0, "", "");
}
int findPosInVector(Entry entry){
    for(int i = 0; i < entrys.size(); i++){
        if(entrys.at(i).name == entry.name){
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
    if(searchedEntry.name.size() < 1){
        newEntry.setName(input);
        cout << "Enter file path to be backed up: ";
        getline(cin, input);
        newEntry.setPath(input);
        entrys.push_back(newEntry);
    } else { 
        cout << "Entry " + input + " already exists with path: " << searchedEntry.filepath << endl;
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
        if(element.name == entry){
            entrys.erase(entrys.begin() + element.line - 1);
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
string vectorToJSONString(const std::vector<Entry>& entries) {
    rapidjson::Document doc;
    doc.SetArray();

    //goes through each entry in the vector & adds the ID (line), name & filepath to the json entry.
    for (const auto& entry : entries) {
        rapidjson::Value val;
        val.SetObject();
         val.AddMember("ID", entry.line, doc.GetAllocator());
        val.AddMember("name", rapidjson::StringRef(entry.name.c_str()), doc.GetAllocator());
        val.AddMember("filepath", rapidjson::StringRef(entry.filepath.c_str()), doc.GetAllocator());
        doc.PushBack(val, doc.GetAllocator());
    }

    //converts the json entry to a string & returns it.
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}
void jsonCommit(){
    ofstream outFile("entrysdb.txt");
    if(outFile.is_open()){
        outFile << vectorToJSONString(entrys);
        outFile.close();
    } else {
        cout << "Error opening file" << endl;
    }
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
                    zipUp(i.filepath, i.name);
                }
                cout << endl << "Backed up entry(s): ";
                for(Entry i : entrys){ 
                    cout << "\"" << i.name << "\"" << " ";
                }
                cout << "to path " << backupDir << endl;
            break;
            case 'q':
                jsonCommit();
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
                        zipUp(entrys.at(userNum - 1).filepath, entrys.at(userNum - 1).name);
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
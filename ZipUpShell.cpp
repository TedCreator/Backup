#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <vector>

// External Libraries included from.
#include <windows.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" //for issues with iterator class being deprecated, but used by rapidjson
//https://rapidjson.org/
#include "extlibs/rapidjson/writer.h"
#include "extlibs/rapidjson/document.h"
#include "extlibs/rapidjson/stringbuffer.h"
#include "extlibs/rapidjson/istreamwrapper.h"
//Minizip library contrib of zlib library: https://www.zlib.net/
//https://chromium.googlesource.com/external/github.com/nmoinvaz/minizip/+/2.3.8/README.md
#include "extlibs/minizip/zip.h"

/*                         -----------------TODO-----------------
ADD functionality to choose where the json file is stored instead of the default documents folder.

                        -----------------Completed-----------------
CHANGED zipping method to use a zipping (minizip) library with similar functionality to 7zip
    also removes using unsafe, non-portable & code injection vulnerable system() method
CHANGED text file to be JSON data storage format using rapidjson library

*/
using namespace std; //convenient but bad practice, will change later
fstream json;
const string backupLocation = R"(./Files/)";
const string jsonDir = backupLocation + R"(entrysdb.json)";

struct Entry {
        int line;
        string name;
        string filepath;
        Entry(){
            name = "unnamed";
            line = 0;
            filepath = R"(no/path/given)";
        }
        ~Entry(){}
        Entry(int place, string entry, string path){
            name = entry;
            line = place;
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
            ret += to_string(line + 1); 
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
        newEntry.setLine(entrys.size());
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
            entrys.erase(entrys.begin() + element.line);
        }
    }
}
void fillVectorfromJSON(){
    using namespace rapidjson; //covers IStreamWrapper, Document, Sizetype, Value
    json.open(jsonDir, ios::in);
    if(json.is_open()){
        IStreamWrapper isw(json);
        Document doc;
        doc.ParseStream(isw);

        if(doc.IsArray()){
            for(SizeType i = 0; i < doc.Size(); i++){
                const Value& entry = doc[i];
                if(entry.IsObject() && entry.HasMember("ID") && entry.HasMember("name") && entry.HasMember("filepath")){
                    Entry vectorEntry;
                    vectorEntry.setLine(entry["ID"].GetInt());
                    vectorEntry.setName(entry["name"].GetString());
                    vectorEntry.setPath(entry["filepath"].GetString());
                    entrys.push_back(vectorEntry);
                }
            }
        }
    }
    json.close();
}
string vectorToJSONString() {
    using namespace rapidjson; //covers Document, Value, StringRef, StringBuffer
    Document doc;
    doc.SetArray();

    //converts the json object to a string & returns it.
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    //goes through each entry in the vector & adds the ID (line), name & filepath to the json entry.
    for (const auto& entry : entrys) {
        Value val;
        val.SetObject();
        val.AddMember("ID", entry.line, doc.GetAllocator());
        val.AddMember("name", StringRef(entry.name.c_str()), doc.GetAllocator());
        val.AddMember("filepath", StringRef(entry.filepath.c_str()), doc.GetAllocator());
        doc.PushBack(val, doc.GetAllocator());
    }
    doc.Accept(writer);

    //method to insert newline between entries to keep json readable
    string jsonBuffer = buffer.GetString();
    for (size_t i = 0; i < jsonBuffer.size() - 1; i++) {
        if (jsonBuffer[i] == '}') {
            jsonBuffer.insert(i + 2, "\n");
            i++; // skips the newline character
        }
    }
    
    return jsonBuffer; //auto casts to a string due to return type
}
void jsonCommit(){
    json.open(jsonDir, ios::out);
    if(json.is_open()){
        json << vectorToJSONString();
        json.close();
    } else {
        cout << "Error opening file" << endl;
    }
}
void recursiveZip(zipFile zip, const string& targetPath, const string& targetLoc){

    for(const auto& element : filesystem::directory_iterator(targetPath)){
        const string elementPath = element.path().string();
        string elementName = elementPath.substr(targetLoc.size() + 1);
        FILE* source = fopen(elementPath.c_str(), "rb"); //Read + Binary mode
        if(element.is_regular_file()){
            zip_fileinfo zfi;
            memset(&zfi, 0, sizeof(zip_fileinfo));

            //reference https://github.com/madler/zlib/blob/master/contrib/minizip/minizip.c
            //reference https://chromium.googlesource.com/external/github.com/nmoinvaz/minizip/+/refs/heads/1.2/minizip.c
            zipOpenNewFileInZip3_64(
                zip,
                elementName.c_str(),
                &zfi,
                nullptr, 0, nullptr, 0, nullptr,
                Z_DEFLATED,
                Z_DEFAULT_COMPRESSION, 0,
                -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                nullptr, 0, 1
            );

            if(source){
                char buffer[8096];
                int bytes;
                
                while((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0){
                    zipWriteInFileInZip(zip, buffer, bytes);
                }
                fclose(source);
            } else {
                cerr << "Error writing to file" << endl;
            }
            // cout << element.path().string() << endl;
        } else if(element.is_directory()){
            recursiveZip(zip, elementPath, targetLoc);
        }
    }
}
void ZipUp(Entry entry){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm utc_tm = *gmtime(&tt);

    //file name idea is "nameOfEntry month-day-year-?h?m?s" .zip
    const string fileName = entry.name + " " + to_string(utc_tm.tm_mon + 1) + "-" + to_string(utc_tm.tm_mday) + "-" + to_string(utc_tm.tm_year + 1900) 
                    + " @ " + to_string(utc_tm.tm_hour-4) + "h" + to_string(utc_tm.tm_min) + "m" + to_string(utc_tm.tm_sec) + "s" + ".zip";
    const string zipPath = backupLocation + fileName;
    
    zipFile zip = zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
    if(!zip){
        cerr << "Error creating zip archive. Likely invalid save path." << endl;
        return;
    }
    recursiveZip(zip, entry.filepath, entry.filepath);
    zipClose(zip, nullptr);
}
string prompt(){
    for(int i = 0; i < entrys.size(); i++){
        cout << entrys.at(i).toString() << endl;
    }
    cout << "-----------Enter any above number from list to backup that entry-----------" << endl 
    << "0. Add/Edit Entry" << endl << "~. to delete a entry" << endl 
    << "s. to save changes (overwrites last save)" << endl 
    << "q. to save and quit" << endl << "x. to quit without saving" << endl << "a. to backup all to folder" << endl;
    cout << "Select an option: ";
    string userOption;
    cin >> userOption;
    return stlow(userOption);
}
int main(){
    string userOption = " "; //for first run
    //Creates the ./Backup directory if it doesn't already exist
    if(filesystem::exists(backupLocation) || filesystem::create_directory(backupLocation)){
        fillVectorfromJSON();
    }
    
    
    do{
        userOption = prompt();
        
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
                for(Entry ent: entrys){
                    ZipUp(ent);
                }
                cout << endl << " **** Backed up entry(s): ";
                for(Entry ent : entrys){ 
                    cout << "\"" << ent.name << "\"" << " ";
                }
                cout << "to path " << backupLocation << " **** " << endl;
            break;
            case 'q':
                jsonCommit();
                exit(0);
            case 's':
                jsonCommit();
            break;
            case 'x':
                cout << " **** Are you sure you want to end program? This will not save any of your entry changes. **** " 
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
                        cout << " **** Not a valid entry ****" << endl;
                    } else {
                        cout << entrys.at(userNum - 1).toString() << endl;
                        ZipUp(entrys.at(userNum - 1));
                    }
                } catch (invalid_argument){
                    cout << " **** Not a valid input ****" << endl;
                }
            break;
        }
        // userOption = "q"; // hardcode for testing 
    } while(userOption != "q");
}
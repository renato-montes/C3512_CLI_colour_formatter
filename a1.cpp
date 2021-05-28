#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/**
 * File: a1.cpp
 * Author: Renato Montes
 * Version: 2016-11-06
 * Course: COMP3512
 * Instructor: Albert Wei
 * 
 * Under Apache License 2.0.
 * (c) 2016 Renato Montes <renato233645@gmail.com>
 */

#ifndef DEFAULT_MODE
  #define DEFAULT_MODE ""
#endif

using namespace std;

/* Function Prototypes */
map<string,string>& loadConfigMap(ifstream&           inStream,
                                  map<string, string>& configMap);

void    displayDebug(map<string, string>& configMap);

string& formatCommand(string& command);
string& formatEscapeCode(string& escapeCode);
string  formatCommandForDisplay(const string& command);

void    leftRightTrim(string& str);

void    run(map<string, string>& configMap);

void    convertParaWithinLines(string& inLines);
void    findLineDivs(const string& inLines, vector<size_t>& lineDivs);

bool    hasValidCommandNesting(const string&         inLines, 
                               const vector<size_t>&     lineDivs, 
                               const map<string, string>& configMap);
bool    hasValidTopLevelStart( const string&         inLines, 
                               const vector<size_t>& lineDivs);
bool    hasValidTopLevelEnd(   const string&         inLines, 
                               const vector<size_t>& lineDivs);

void    convertCommands(string& inLines, const map<string, string>& configMap);
void    reconvertParaWithinLines(string& inLines);

/**
 * Entry point.
 * param: argc argument count
 * param: argv array of arguments the program was called with
 */
int main(int argc, char *argv[]){
    if (argc > 2) {
        cerr << "Usage: " << argv[0]
             << " [OPTIONAL: <configFileName>]" << endl;
        return 1;
    }

    string configFile("config");
    if (argc == 2)
        configFile = argv[1];

    ifstream inStream(configFile, ios::in|ios::binary);
    if (!inStream) {
        cerr << "Unable to open '" << configFile
             << "' for reading." << endl;
        return 2;
    }

    map<string,string> configMap;
    loadConfigMap(inStream, configMap);
    
#ifdef DEBUG
    displayDebug(configMap);
    return 0;
#endif

    run(configMap);
}

/**
 * Loads configuration file.
 * param: inStream input file stream for the configuration file
 * param: configMap the map to store the commands and ANSI escape codes
 * return: configuration map loaded with commands and ANSI escape codes
 */
map<string,string>& loadConfigMap(ifstream& inStream,
                                  map<string,string>& configMap){
    string line;
    while(getline(inStream,line)) {
        string command, escapeCode;
        istringstream iss(line);
        if(iss >> command)
            if(iss >> escapeCode)
                if(configMap.find(command) ==  configMap.end())
                    configMap[formatCommand(command)] = formatEscapeCode(escapeCode);
    }
    return configMap;
}

/*
 * Formats a command for storage in the configuration file.
 * param: command the command to be formatted
 * return: the command already formatted
 */
string& formatCommand(string& command) {
    size_t readPos = 0;
    while (readPos != string::npos) {
        readPos = command.find("(", readPos);
        if (readPos != string::npos) {
            command.replace(readPos,1,"\001");
            readPos++;
        }
    }
    readPos = 0;
    while (readPos != string::npos) {
        readPos = command.find(")", readPos);
        if (readPos != string::npos) {
            command.replace(readPos,2,"\002");
            readPos++;
        }
    }
    return command;
}

/**
 * Handles special cases for the ANSI escape codes in the configuration file.
 * param: escapeCode the escape code to be formatted
 * return: the formatted escape code
 */
string& formatEscapeCode(string& escapeCode){
    if(escapeCode.substr(0,2) == "\\e"){
        escapeCode = string("\033" + escapeCode.substr(2));
        return escapeCode;
    }
    if(escapeCode.substr(0,3) == "\\\\e"){
        escapeCode = string("\\e" + escapeCode.substr(3));
        return escapeCode;
    }
    return escapeCode;
}

/**
 * Formats a command from the configuration map for display.
 * param: command the command to be formatted
 * return: the formatted command
 */
string formatCommandForDisplay(const string& command) {
    string ret(command);
    size_t readPos = 0;
    while (readPos != string::npos) {
        readPos = ret.find("\001", readPos);
        if (readPos != string::npos) {
            ret.replace(readPos,1,"(");
            readPos++;
        }
    }
    readPos = 0;
    while (readPos != string::npos) {
        readPos = ret.find("\002", readPos);
        if (readPos != string::npos) {
            ret.replace(readPos,1,")");
            readPos++;
        }
    }
    return ret;
}

/**
 * Displays the commands loaded onto the configuration map in DEBUG mode.
 * param: configMap the configuration map
 */
void displayDebug(map<string,string>& configMap){
    for(const auto& p: configMap){
        cout << p.second << '('
             << formatCommandForDisplay(p.first)
             << ')' << DEFAULT_MODE << endl;
    }
}

/**
 * Trims white space at the left and right of the input lines.
 * param: inputLines the input lines
 */
void leftRightTrim(string& inLines) {
	inLines.erase(0, inLines.find_first_not_of(" \t\r\n"));
	inLines.erase(inLines.find_last_not_of(" \t\r\n") + 1);
}

/**
 * Converts parenthesis text (i.e. double parentheses) to codes.
 * param: inLines the input lines
 */
void convertParaWithinLines(string& inLines) {
    size_t readPos = 0;
    while (readPos != string::npos) {
        readPos = inLines.find("((", readPos);
        if (readPos != string::npos) {
            inLines.replace(readPos,2,"\001");
            readPos++;
        }
    }
    readPos = 0;
    while (readPos != string::npos) {
        readPos = inLines.find("))", readPos);
        if (readPos != string::npos) {
            inLines.erase(readPos, 2);
            inLines.insert(readPos, "\002");
            readPos++;
        }
    }
}

/**
 * Finds the newline characters and stores them in a vector.
 * param: inLines the input lines
 * param: lineDivs the vector for holding the positions of newline characters
 */
void findLineDivs(const string& inLines, vector<size_t>& lineDivs) {
    size_t i = 0;
    for (; i < inLines.size(); i++)
        if (inLines.at(i) == '\n')
            lineDivs.push_back(i);
    lineDivs.push_back(i);
}

/**
 * Determines whether the input lines have a valid beginning of a top-level
 * list.
 * param: inLines the input lines
 * param: lineDivs the vector holding the positions of newline characters
 */
bool hasValidTopLevelStart(const string& inLines, 
                           const vector<size_t>& lineDivs) {
    if (inLines.at(inLines.find_first_not_of(" \t\r\n")) != '(') {
        size_t errorPos = inLines.find_first_not_of(" \t\r\n");
        size_t errorLine = 0;
        for (size_t i = 0; i < lineDivs.size(); i++) {
            if (lineDivs.at(i) >= errorPos) {
                errorLine = i + 1;
                break;
            }
        }
        cerr << "ERROR: at input line " << errorLine << ", word found before "
             << "beginning of top-level list." << endl;
        return false;
    }
    return true;
}

/**
 * Prints an error when it's found in the nesting of commands.
 * param: readPos the current reading position in the input lines
 * param: lineDivs the vector holding the positions of newline characters
 * param: message the error message to be displayed
 */
void printCommandNestingError(const size_t readPos,
                              const vector<size_t>& lineDivs,
                              const string& message) {
    size_t errorLine = 0;
    for (size_t i = 0; i < lineDivs.size(); i++) {
        if (lineDivs.at(i) >= readPos) {
            errorLine = i + 1;
            break;
        }
    }
    cerr << "ERROR: at input line " << errorLine << ", " << message << endl;
}

/**
 * Determines whether the nesting of commands is valid (including whether the
 * commands exist in the configuration file/map).
 * param: inLines the input lines
 * param: lineDivs the vector holding the positions of newline characters
 * param: configMap the configuration map holding commands and ANSI escape codes
 */
bool hasValidCommandNesting(const string& inLines, 
                            const vector<size_t>& lineDivs, 
                            const map<string, string>& configMap) {     
    size_t readPos = 0;
    size_t lPara = 0;
    size_t rPara = 0;
    string command;
    while (readPos != lineDivs.back()) {
        if (inLines.at(readPos) == '(') {
            lPara++;
            readPos = inLines.find_first_not_of(" \t\r\n", readPos + 1);
            if (inLines.at(readPos) == ')') {
                printCommandNestingError(readPos, lineDivs,
                    "no command was found after left delimiter.");
                return false;
            }
            size_t commandStart = readPos;
            size_t commandEnd = inLines.find_first_of(" \t\r\n)", 
                                                      commandStart);
            for (size_t i = commandStart; i < commandEnd; i++) {
                command += inLines.at(i);
            }
            if (configMap.count(command) == 0) {
                printCommandNestingError(readPos, lineDivs,
                    "command entered was not found in config file.");
                return false;
            }
            command.clear();
            readPos = commandEnd;
        }
        if (inLines.at(readPos) == ')') {
            rPara++;
            if (rPara > lPara) {
                printCommandNestingError(readPos, lineDivs,
                    "too many right delimiters at end of top-level list");
                return false;
            }
        }
        readPos++;
    }

    readPos = lineDivs.back();
    if (lPara > rPara) {
        printCommandNestingError(readPos, lineDivs,
            "unmatched left delimiter.");
        return false;
    }

    return true;
}

/**
 * Determines whether the top-level list ends correctly.
 * param: inLines the input lines
 * param: lineDivs the vector holding the positions of newline characters
 */
bool hasValidTopLevelEnd(const string& inLines, 
                         const vector<size_t>& lineDivs) {
    if (inLines.at(inLines.find_last_not_of(" \t\r\n")) != ')') {
        size_t errorPos = inLines.find_last_not_of(" \t\r\n");
        size_t errorLine = 0;
        for (size_t i = 0; i < lineDivs.size(); i++) {
            if (lineDivs.at(i) >= errorPos) {
                errorLine = i + 1;
                break;
            }
        }
        cerr << "ERROR: at input line " << errorLine << ", word found after "
             << "end of top-level list." << endl;
        return false;
    }
    return true;
}

/*
 * Converts commands into their corresponding ANSI escape code in the map.
 * param: inLines the input lines
 * param: configMap the configuration map holding commands and ANSI escape codes
 */
void convertCommands(string& inLines, const map<string, string>& configMap) {
    size_t i = 0;
    string command;
    vector<string> commandStack = {DEFAULT_MODE};
    while (i < inLines.size()) {
        if (inLines.at(i) == '(') {
            inLines.erase(i, 1);
            size_t commandStart = inLines.find_first_not_of(" \t\r\n", i);
            size_t commandEnd = inLines.find_first_of(" \t\r\n", commandStart);
            for (size_t j = commandStart; j < commandEnd; j++) {
                command += inLines.at(j);
            }
            inLines.replace(commandStart, commandEnd - commandStart, 
                            configMap.at(command));
            commandStack.push_back(command);
            commandEnd = commandStart + configMap.at(command).size();
            size_t commandWsEnd = inLines.find_first_not_of(" \t\r\n", 
                                                            commandEnd);
            inLines.erase(commandEnd, commandWsEnd - commandEnd);
            command.clear();
        } else if (inLines.at(i) == ')') {
            inLines.erase(i, 1);
            commandStack.pop_back();
            if (commandStack.size() > 1)
                inLines.insert(i, configMap.at(commandStack.back()));
            else
                inLines.insert(i, DEFAULT_MODE);
        } else {
            i++;
        }
    }
}

/*
 * Reconverts parentheses within input lines.
 * param: inLines input lines
 */
void reconvertParaWithinLines(string& inLines) {
    size_t readPos = 0;
    while (readPos != string::npos) {
        readPos = inLines.find("\001", readPos);
        if (readPos != string::npos) {
            inLines.replace(readPos,1,"(");
            readPos++;
        }
    }
    readPos = 0;
      while (readPos != string::npos) {
        readPos = inLines.find("\002", readPos);
        if (readPos != string::npos) {
            inLines.replace(readPos,1,")");
            readPos++;
        }
    }
}

/**
 * Gets the input lines and processes them.
 * param: configMap the configuration map holding commands and ANSI escape codes
 */
void run(map<string, string>& configMap) {
    string         line;
    string         inLines;
    vector<size_t> lineDivs;

    while(getline(cin, line)) {
        inLines += line;
        inLines += '\n';
    }
    inLines.erase(inLines.size() - 1);

    if(inLines.size() == 0){
        cerr << "ERROR: input begins with the end-of-file character." << endl;
        return;
    }
    
    //conversion of (( and )) to non-printable characters
    convertParaWithinLines(inLines);

    //find the positions of end-of-line characters
    findLineDivs(inLines, lineDivs);

    //look for errors: words outside top-level (...) list
    if(!hasValidTopLevelStart(inLines, lineDivs)) return;

    //look for errors: (<whitespace>), too many ), too few )
    if(!hasValidCommandNesting(inLines, lineDivs, configMap)) return;

    //look for errors: words outside top-level (...) list at the end
    if(!hasValidTopLevelEnd(inLines, lineDivs)) return;

    //trim input
    leftRightTrim(inLines);

    //find the commands and convert them
    convertCommands(inLines, configMap);    

    //reconvert /001 and /002 to ( and )
    reconvertParaWithinLines(inLines);

    //print the thing to standard out
    cout << inLines;
    
    return;
}


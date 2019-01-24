#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>


enum Mode {r, w, a, d};
std::map<std::string, Mode> modemap;
std::map<uint32_t, std::string> commandList;
std::string filename;

void interpret(std::string words, Mode * mode);
void checkValidInputs(int argcount);
std::vector<std::string> split(std::string words);
void setupmap();
bool isNumber(std::string s);
void writetofile();
void printcontents();
uint32_t strtoint(std::string s);
void populatecommandlist();

int main(int argc, char *argv[]) {
    std::cout << "hulied v0.9" << std::endl;
    checkValidInputs(argc);
    setupmap();
    filename = argv[1];
    Mode * m = new Mode;
    * m = r;
    populatecommandlist();
    while(true){
        std::cout << ">";
        std::string input;
        std::getline(std::cin, input);
        interpret(input, m);
    }
    return 0;
}

void checkValidInputs(int argcount){
    if (argcount != 2){
        std::cout << "Invalid number of inputs" << std::endl;
        exit(1);
    }
}

void interpret(std::string words, Mode * mode){
    if(words == "q" || words == "quit"){
        exit(0);
    }

    if(words == ""){
        return;
    }

    if(words == "save"){
        writetofile();
        return;
    }

    if(words == "print"){
        printcontents();
        return;
    }

    if(words == "help"){
        std::cout << "Welcome to Hunter's Line Editor, or hulied (pronounced who lied).\n"
        << "There are 4 modes available: read(r), write(w), append(a), or delete(d).\n"
        << "To change modes, type mode followed by one of the mode letters.\n"
        << "The difference between write and append is write will overwrite lines that exist while append won't.\n"
        << "To write to the file at any time, type save. To print all the lines, type print.\n"
        << "You can also use print followed by a number to print that line or two numbers to print the range.\n"
        << "To add a new line or overwrite a line while in write mode, type the line number then the line.\n"
        << "To delete a line, type delete then the line number.\n"
        << "To quit this program, type either q or quit.\n"
        << std::endl;
        return;
    }

    std::vector<std::string> command = split(words);

    if(command[0] == "mode"){
        if(isNumber(command[1])){
            //*mode = command[1];
            std::cout << "Still need to implement this feature" << std::endl;
        }
        else {
            *mode = modemap[command[1]];
        }
        std::cout << "Mode changed to " << * mode << std::endl;
        return;
    }

    if(*mode == a && isNumber(command[0])){
        commandList.insert(std::pair<uint32_t, std::string>(strtoint(command[0]), command[1]));
        return;
    }

    if(*mode == w && isNumber(command[0])){
        uint32_t index = strtoint(command[0]);
        if(commandList.count(index) != 0)
            commandList.erase(index);
        commandList.insert(std::pair<uint32_t, std::string>(index, command[1]));
        return;
    }

    if(*mode == d && command[0] == "delete"){
        if(isNumber(command[1])){
            uint32_t index = strtoint(command[1]);
            commandList.erase(index);
            std::cout << "Deleted line " << index << std::endl;
        }
        return;
    }

    if(command[0] == "print"){
        int firstspace = command[1].find(' ');
        if(firstspace == -1){
            std::string commandAtThisPoint = commandList[strtoint(command[1])];
            if(commandAtThisPoint != "")
                std::cout << command[1] << " " << commandAtThisPoint << std::endl;
        }
        else{
            uint32_t lower, upper;
            auto temp = split(command[1]);
            lower = strtoint(temp[0]);
            upper = strtoint(temp[1]);
            if(lower > upper){
                std::cout << "Range out of order" << std::endl;
                return;
            }
            for(uint32_t x = lower; x <= upper; ++x){
                std::string commandAtThisPoint = commandList[x];
                if(commandAtThisPoint != "")
                    std::cout << x << " " << commandAtThisPoint << std::endl;
            }
        }
        return;
    }



}

std::vector<std::string> split(std::string words){
    int firstspace = words.find(' ');
    std::vector<std::string> temp;
    if(firstspace == -1){
        temp.push_back(words);
        temp.push_back("");
    }

    temp.push_back(words.substr(0, firstspace));
    temp.push_back(words.substr(firstspace+1));
    return temp;
}

void setupmap(){
    modemap.insert(std::pair<std::string, Mode>("r", r));
    modemap.insert(std::pair<std::string, Mode>("w", w));
    modemap.insert(std::pair<std::string, Mode>("a", a));
    modemap.insert(std::pair<std::string, Mode>("d", d));
}

bool isNumber(std::string s){
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    //Source of this function:
    //https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
}

void writetofile(){
    std::ofstream file;
    file.open(filename);
    for(std::map<uint32_t, std::string>::const_iterator iter = commandList.begin(); iter != commandList.end(); ++iter){
        file << iter->first << " " << iter->second << std::endl;
    }
    file.close();
}

void printcontents(){
    for(std::map<uint32_t, std::string>::const_iterator iter = commandList.begin(); iter != commandList.end(); ++iter){
        std::cout << iter->first << " " << iter->second << std::endl;
    }
}

uint32_t strtoint(std::string s){
    std::stringstream ss(s);
    uint32_t num;
    ss >> num;
    return num;
}

void populatecommandlist(){
    std::ifstream file;
    file.open(filename);
    uint32_t x;
    std::string y;
    while(file >> x){
        try {
            std::getline(file, y);
            y = y.substr(1);
        }
        catch (...){
            std::cout << "Invalid file format" << std::endl;
            exit(2);
        }
        commandList.insert(std::pair<uint32_t, std::string>(x, y));
    }

}
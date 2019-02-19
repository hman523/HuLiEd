/**
 * Author: Stephen Hunter Barbella (hman523)
 * Date Started: 1/29/19
 * Licence: MIT
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

// Things to add
// detecting files with line numbers and turn on the right mode
// Set prompt option

// Global variables
// ENUM: Mode
// Essentially the state you are in
// Modes: read, write, append, delete, shift
enum Mode { r, w, a, d, s };

// modemap essentially maps the modes to strings that they are equivelent to
std::map<std::string, Mode> modemap;

// command list is where all the lines you write are held
std::map<uint32_t, std::string> commandList;

// filename: the name of the file you ae editing
std::string filename;

// BasicMode: default is false
bool basicMode = false;

// verNum: the version number
const std::string verNum = "2.0";

// prompt: the prompt on the cli
std::string prompt = ">";

// Methods

// Interpret: gets the input, parses it, and executes the command
void interpret(std::string words, Mode *mode);

// checkValidInput: makes sure you only have one command line arg
void checkValidInputs(int argcount);

// Split: splits the string into the command and the parameter
std::vector<std::string> split(std::string words);

// setupmap: Sets up the mode map and populates it
void setupmap();

// isNumber: returns if the string is a number or not
bool isNumber(std::string s);

// writetofile: writes the contents of command list to the file
void writetofile();

// printcontents: prints the contents of command list
void printcontents();

// strtoint: converts a string that is a number into a uint32_t
uint32_t strtoint(std::string s);

// populatecommandlist: fills the command list with what is in the file already
void populatecommandlist();

// shift: shifts all lines down however many lines are specified
void shift(uint32_t offset);

// shift: shifts all lines down however many lines are specified starting at
// start
void shift(uint32_t offset, uint32_t start);

// turnBasicModeOn: turns basic mode (line numbers in file) on
void turnBasicModeOn();

// turnBasicModeOff: turns basic mode (line numbers in file) off
void turnBasicModeOff();

// detectAndSetBasicMode: used to check if the file is a numbered file
bool detectAndSetBasicMode();

// setPrompt: used to change the prompt
void setPrompt(std::string newPrompt);

// printHelp: prints help statement
void printHelp();

int main(int argc, char *argv[]) {
  std::cout << "hulied v" << verNum << std::endl;
  checkValidInputs(argc);
  setupmap();
  filename = argv[1];
  Mode *m = new Mode;
  // default mode is read
  *m = r;
  populatecommandlist();
  // interpretation loop
  while (true) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    interpret(input, m);
  }
  return 0;
}

void checkValidInputs(int argcount) {
  if (argcount != 2) {
    std::cout << "Invalid number of inputs" << std::endl;
    exit(1);
  }
}

void interpret(std::string words, Mode *mode) {
  // for the exiting of the program
  if (words == "q" || words == "quit") {
    exit(0);
  }

  // for empty string edge condition
  if (words == "") {
    std::cout << "?" << std::endl;
    return;
  }

  // saves the contents of the map to the file
  if (words == "save") {
    writetofile();
    return;
  }

  // prints all of the lines
  if (words == "print") {
    printcontents();
    return;
  }

  // prints the help statement
  if (words == "help") {
    printHelp();
  }

  // Now we split the input into command and parameter
  std::vector<std::string> command = split(words);

  if (command[0] == "lines") {
    if (command[1] == "on") {
      turnBasicModeOn();
      std::cout << "Line numbers in file turned on" << std::endl;
    } else if (command[1] == "off") {
      turnBasicModeOff();
      std::cout << "Line numbers in file turned off" << std::endl;

    } else {
      std::cout << "?" << std::endl;
    }
    return;
  }

  // this is used when we want to change the mode
  if (command[0] == "mode") {
    if (isNumber(command[1])) {
      //*mode = command[1];
      std::cout << "Still need to implement this feature" << std::endl;
    } else {
      *mode = modemap[command[1]];
    }
    std::cout << "Mode changed to " << *mode << std::endl;
    return;
  }

  if (command[0] == "prompt") {
    setPrompt(command[1]);
    return;
  }

  // this is used when inserting a line in append mode
  if (*mode == a && isNumber(command[0])) {
    commandList.insert(
        std::pair<uint32_t, std::string>(strtoint(command[0]), command[1]));
    return;
  }

  // This is used when writing a line while in write mode
  if (*mode == w && isNumber(command[0])) {
    uint32_t index = strtoint(command[0]);
    if (commandList.count(index) != 0)
      commandList.erase(index);
    commandList.insert(std::pair<uint32_t, std::string>(index, command[1]));
    return;
  }

  // this is for printing either 1 or a range of lines
  if (command[0] == "print") {
    int firstspace = command[1].find(' ');
    if (firstspace == -1) {
      try {
        std::string commandAtThisPoint = commandList.at(strtoint(command[1]));
        std::cout << command[1] << " " << commandAtThisPoint << std::endl;
      } catch (...) {
        std::cout << "Line does not exist" << std::endl;
      }
    } else {
      uint32_t lower, upper;
      auto temp = split(command[1]);
      lower = strtoint(temp[0]);
      upper = strtoint(temp[1]);
      if (lower > upper) {
        std::cout << "Range out of order" << std::endl;
        return;
      }
      for (uint32_t x = lower; x <= upper; ++x) {
        try {
          std::string commandAtThisPoint = commandList.at(x);
          std::cout << x << " " << commandAtThisPoint << std::endl;
        } catch (...) {
          // do nothing
        }
      }
    }
    return;
  }

  // this is for shifting either all or starting at a point of lines
  if (*mode == s && command[0] == "shift") {
    int firstspace = command[1].find(' ');
    if (firstspace == -1) {
      shift(strtoint(command[1]));
    } else {
      uint32_t first, second;
      auto temp = split(command[1]);
      first = strtoint(temp[0]);
      second = strtoint(temp[1]);
      shift(first, second);
    }
    return;
  }

  // used for deleting
  if (*mode == d && command[0] == "delete") {
    int firstspace = command[1].find(' ');
    if (firstspace == -1) {
      if (isNumber(command[1])) {
        try {
          uint32_t index = strtoint(command[1]);

          commandList.erase(index);
          std::cout << "Deleted line " << index << std::endl;
        } catch (...) {
        }
      }
      return;
    } else {
      uint32_t lower, upper;
      auto temp = split(command[1]);
      lower = strtoint(temp[0]);
      upper = strtoint(temp[1]);
      if (lower > upper) {
        std::cout << "Range out of order" << std::endl;
        return;
      }
      std::cout << "Deleting: ";
      for (uint32_t x = lower; x <= upper; ++x) {
        try {
          std::string commandAtThisPoint = commandList.at(x);
          commandList.erase(x);
          std::cout << x << ", ";
        } catch (...) {
        }
      }
      std::cout << std::endl;
    }
    return;
  }

  std::cout << "?" << std::endl;
  return;
}

// simple method for spliting the command and its parameter
std::vector<std::string> split(std::string words) {
  int firstspace = words.find(' ');
  std::vector<std::string> temp;
  if (firstspace == -1) {
    temp.push_back(words);
    temp.push_back("");
  }

  temp.push_back(words.substr(0, firstspace));
  temp.push_back(words.substr(firstspace + 1));
  return temp;
}

// maps a string to the enum that it represents
void setupmap() {
  modemap.insert(std::pair<std::string, Mode>("r", r));
  modemap.insert(std::pair<std::string, Mode>("w", w));
  modemap.insert(std::pair<std::string, Mode>("a", a));
  modemap.insert(std::pair<std::string, Mode>("d", d));
  modemap.insert(std::pair<std::string, Mode>("s", s));
}

// simple function to test if this string is a number
bool isNumber(std::string s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
                         return !std::isdigit(c);
                       }) == s.end();
  // Source of this function:
  // https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
}

// writes contents of the command list to the file
void writetofile() {
  std::ofstream file;
  file.open(filename);
  if (basicMode) {
    for (std::map<uint32_t, std::string>::const_iterator iter =
             commandList.begin();
         iter != commandList.end(); ++iter) {
      file << iter->first << " " << iter->second << std::endl;
    }
  } else {
    for (std::map<uint32_t, std::string>::const_iterator iter =
             commandList.begin();
         iter != commandList.end(); ++iter) {
      file << iter->second << std::endl;
    }
  }
  file.close();
}

// prints all of the lines
void printcontents() {
  for (std::map<uint32_t, std::string>::const_iterator iter =
           commandList.begin();
       iter != commandList.end(); ++iter) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
}

// converts the string to a uint32_t
uint32_t strtoint(std::string s) {
  std::stringstream ss(s);
  uint32_t num;
  ss >> num;
  return num;
}

// fills the command list with what's already in the file
void populatecommandlist() {
  detectAndSetBasicMode();
  std::ifstream file;
  file.open(filename);
  if (basicMode) {
    uint32_t x;
    std::string y;
    while (file >> x) {
      try {
        std::getline(file, y);
        y = y.substr(1);
      } catch (...) {
        std::cout << "Invalid file format" << std::endl;
        exit(2);
      }
      commandList.insert(std::pair<uint32_t, std::string>(x, y));
    }
  } else {
    std::string line;
    uint32_t lineNum = 1;
    while (std::getline(file, line)) {
      commandList.insert(std::pair<uint32_t, std::string>(lineNum, line));
      ++lineNum;
    }
  }
}

void shift(uint32_t offset) {
  std::map<uint32_t, std::string> temp;
  for (auto iter = commandList.begin(); iter != commandList.end(); ++iter) {
    temp.insert(
        std::pair<uint32_t, std::string>(iter->first + offset, iter->second));
  }
  std::swap(temp, commandList);
}

void shift(uint32_t offset, uint32_t start) {
  std::map<uint32_t, std::string> temp;
  auto pivotpoint = commandList.lower_bound(start);
  for (auto iter = commandList.begin(); iter != pivotpoint; ++iter) {
    temp.insert(std::pair<uint32_t, std::string>(iter->first, iter->second));
  }
  for (auto iter = pivotpoint; iter != commandList.end(); ++iter) {
    temp.insert(
        std::pair<uint32_t, std::string>(iter->first + offset, iter->second));
  }
  std::swap(temp, commandList);
}

void turnBasicModeOn() { basicMode = true; }

void turnBasicModeOff() { basicMode = false; }

bool detectAndSetBasicMode() {
  std::ifstream file;
  file.open(filename);
  char x;
  file >> x;
  if (x >= '0' && x <= '9') {
    turnBasicModeOn();
  } else {
    turnBasicModeOff();
  }
  if (basicMode) {
    std::cout << "Line mode on" << std::endl;
  } else {
    std::cout << "Line mode off" << std::endl;
  }
  return basicMode;
}

void setPrompt(std::string newPrompt) { prompt = newPrompt; }

void printHelp() {
  std::cout
      << "Welcome to Hunter's Line Editor, or hulied (pronounced who lied).\n"
      << "There are 5 modes available: read(r), write(w), append(a), "
         "delete(d), or shift(s).\n"
      << "To change the prompt, type prompt then the new prompt.\n"
      << "To change modes, type mode followed by one of the mode letters.\n"
      << "To turn on line mode (AKA Basic mode), type lines on.\n"
      << "To do the opposite, type lines off. The default when you turn it "
         "on is off.\n"
      << "The difference between write and append is write will overwrite "
         "lines that exist while append won't.\n"
      << "To write to the file at any time, type save. To print all the "
         "lines, type print.\n"
      << "You can also use print followed by a number to print that line or "
         "two numbers to print the range.\n"
      << "To add a new line or overwrite a line while in write mode, type "
         "the line number then the line.\n"
      << "To delete a line, type delete then the line number or two numbers "
         "to delete the range.\n"
      << "To shift all the lines, go into shift mode and type shift then how "
         "many lines you want to offset."
      << "To shift all lines after a certain point, type shift then the "
         "offset then the starting line.\n"
      << "To quit this program, type either q or quit.\n"
      << std::endl;
  return;
}
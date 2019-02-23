#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
//use the extern environment variable
extern char ** environ;
//myShell is the parent class contains one abstract methon called parse()

class myShell
{
 private:
  std::vector<std::string> path;              //vector to store all the possible path
  std::string first_command;                  //the command user input
  std::string argumentStr;                    //user input which part should be treated as arguments
  std::map<std::string, std::string> varMap;  //map to store variable and its value
 public:
  //constructor
  myShell() {}
  myShell(std::vector<std::string> path,
          std::string first_command,
          std::string argumentStr,
          std::map<std::string, std::string> map) :
      path(path),
      first_command(first_command),
      argumentStr(argumentStr),
      varMap(map){};
  //assignment operator

  //destructor

  //parse the user input to command and argument string and update first_command and argumentStr
  void parse(std::string user_command);
  //parse the argumentStr to vector arguments in correct format
  std::vector<std::string> parseArgument();
  //replace the variable with its value
  std::string replaceVar(std::string userInput);
  //execuate the process
  void execuate(char * argv);
  //get all the possible path and store them into vector path
  void getAllPossiblePath();
  //get the absolute path
  std::string getPath(std::string get_path);
  //allow external to read the first_command value
  std::string getFirstCommand();
  //allow external to read the argumentStr value
  std::string getArgumentStr();
  //cd command, parameter is the directory we want to change
  void cdCommand(std::string directory);
  //access to variable
  std::vector<std::string> dollarSplit(std::string toSplit);
  //check the variable name legal or not
  bool checkVarName(std::string name);
  //set command
  void setVar(std::string argument);
  //check whether the variable in varMap or not
  bool checkVarExit(std::string key);
  //export command
  void exportVar(std::string argument);
  //check value of variable can be converted to digital or not
  int checkDigit(std::string value);
  //inc command
  void incInterpret(std::string argument);
};

std::vector<std::string> myShell::dollarSplit(std::string toSplit) {
  std::vector<std::string> varVect;
  char * var_name = strtok((char *)toSplit.c_str(), "$");
  while (var_name != NULL) {
    std::string varName = var_name;
    varVect.push_back(varName);
    var_name = strtok(NULL, "$");
  }
  return varVect;
}

//check whether variable in varMap or not
bool myShell::checkVarExit(std::string key) {
  std::map<std::string, std::string>::iterator it;
  it = varMap.find(key);
  if (it == varMap.end()) {
    return false;
  }
  return true;
}

std::string myShell::replaceVar(std::string userInput) {
  std::string replaceInput;
  if (userInput.find("$") == std::string::npos) {
    std::cout << "raw not change:" << userInput << std::endl;
    return userInput;
  }
  std::string command = userInput.substr(0, userInput.find("$"));
  replaceInput.append(command);
  std::string toSplit = userInput.substr(userInput.find("$"));
  std::cout << "command=" << command << ",toSplice=" << toSplit << "." << std::endl;
  std::vector<std::string> toReplace = dollarSplit(toSplit);
  //  std::cout << "after dollarsplit" << std::endl;
  std::vector<std::string>::iterator it = toReplace.begin();
  bool exit = false;
  while (it != toReplace.end()) {
    for (size_t i = 0; i < (*it).size(); i++) {
      std::string keyToCheck = (*it).substr(0, (*it).size() - i);
      exit = checkVarExit(keyToCheck);
      if (exit == true) {
        replaceInput.append(varMap[keyToCheck]);
        break;
      }
    }

    //    if (exit == false) {
    //replaceInput.append("");
    // }
    ++it;
  }

  return replaceInput;
}
void myShell::parse(std::string user_command) {
  std::string userCommand = user_command;
  //find the first white space and before that is the command
  int first_blank_index = userCommand.find(" ");
  if (first_blank_index != -1) {
    //can find the white space
    //update the first_command of myShell class
    first_command = userCommand.substr(0, first_blank_index);
    //update the argumentStr in myShell class
    argumentStr = userCommand.substr(first_blank_index + 1);
  }
  else {
    //can not find the white blank
    first_command = userCommand;
    argumentStr = "";
  }
}
std::vector<std::string> myShell::parseArgument() {
  std::vector<std::string> arguments;
  //convert the string to vector of chars named writable
  std::vector<char> writable(argumentStr.begin(), argumentStr.end());

  //use queue as container for each correct-formated argument
  std::queue<char> container;
  //use iterator to scan all chars in writable vector
  std::vector<char>::iterator it = writable.begin();

  while (it != writable.end()) {
    //if it points to \ then push the char after it to queue
    //make sure it does not point to the laster char in vector
    if (*it == '\\' && it + 1 != writable.end()) {
      container.push(*(it + 1));
      //it add one
      it = it + 1;
    }
    //if it points to white space then split the arguments
    else if (*it == ' ') {
      std::string oneArgument = "";
      //convert all chars in queue to a string, add that string to arguments vector, empty the queue
      while (!container.empty()) {
        oneArgument.push_back(container.front());
        container.pop();
      }
      if (oneArgument != "") {
        arguments.push_back(oneArgument);
      }
    }
    //if it points to a nornal char then push that char to queue
    else {
      container.push(*it);
    }
    ++it;
  }
  //put the last argument into the vector arguments
  if (!container.empty()) {
    std::string lastArgument = "";
    //convert all chars in queue to a string, add that string to arguments vector, empty the queue
    while (!container.empty()) {
      lastArgument.push_back(container.front());
      container.pop();
    }
    if (lastArgument != "") {
      arguments.push_back(lastArgument);
    }
  }

  return arguments;
}

//execuate
void myShell::execuate(char * argv) {
  //pid of current process
  std::cout << "in execuate, input is " << argv << "." << std::endl;
  pid_t curr_pid = getpid();
  //print error message if fork fails
  if ((curr_pid = fork()) < 0) {
    perror(" fork failed/n");
    exit(1);
  }

  //if curr_pid is zero then current process is child process
  if (curr_pid == 0) {
    //execuate the program pointed to by argv, replace the current process with a new process
    std::vector<std::string> arguments = parseArgument();
    size_t size = arguments.size();
    char ** argument_list = new char *[size + 2];
    argument_list[0] = argv;
    argument_list[size + 1] = NULL;
    char * oneArgument;
    for (size_t i = 1; i < size + 1; i++) {
      std::string one_argument = arguments[i - 1];
      oneArgument = strdup(one_argument.c_str());
      argument_list[i] = oneArgument;
    }
    free(oneArgument);
    char ** env = environ;

    execve(argv, argument_list, env);
    //print the error message and exit if execve() function fails
    delete[] argument_list;
    perror(*argument_list);
    exit(1);
  }
  else {
    //when current process is parent process
    int status = 0;
    int option = 0;
    pid_t error_pid = waitpid(curr_pid, &status, option);
    //print the error message and exit if the parent process cannot wait for child process to execuate
    if (error_pid != -1 && WIFSIGNALED(status)) {
      int exit_signal = WTERMSIG(status);
      printf("Program was killed by signal %d\n", exit_signal);
    }
    if (error_pid != -1 && WIFEXITED(status)) {
      int exit_code = WEXITSTATUS(status);
      printf(" Program exited with status %d\n", exit_code);
    }
  }
}
void myShell::getAllPossiblePath() {
  //call getenv to get all the possible path string
  char * allpossiblepath = getenv("PATH");
  //backup the return value of getenv
  std::string bp(allpossiblepath);
  //split the path string with : and stroe them into vector path
  char * onepossiblepath = strtok((char *)bp.c_str(), ":");

  while (onepossiblepath != NULL) {
    std::string onepossible = onepossiblepath;
    path.push_back(onepossible);
    onepossiblepath = strtok(NULL, ":");
  }
}
std::string myShell::getPath(std::string argv) {
  //convert the user command from char * to string
  //command is the absolute path to return
  std::string command = argv;

  //if the std::cin starts with / or starts with ./

  if (argv[0] == '/' || (argv[0] == '.' && argv[1] == '/')) {
    return command;
  }
  else {
    //check whether the path exists
    struct stat sb;
    //use iterator to scan all value in vector path
    std::vector<std::string>::iterator it = path.begin();
    while (it != path.end()) {
      std::string real_path = *it + "/" + command;
      //convert std::string to const char * to use stat() function
      const char * path_format = real_path.c_str();
      //if the absolute exits and can execuate, then return the path
      if (stat(path_format, &sb) == 0 && sb.st_mode & S_IXUSR) {
        return real_path;
      }
      ++it;
    }
    //if the path does not exists
    if (it == path.end()) {
      std::cout << "Command " << argv << " not found";
    }
  }
  return command;
}

void myShell::cdCommand(std::string directory) {
  chdir(directory.c_str());
  printf("current working directory: %s\n", getcwd(NULL, 0));
}
//access the firstCommand attribute of class myShell
std::string myShell::getFirstCommand() {
  return first_command;
}
std::string myShell::getArgumentStr() {
  return argumentStr;
}

bool myShell::checkVarName(std::string name) {
  //check the variable name is valid or not
  for (size_t i = 0; i < name.size(); i++) {
    if (!(isalpha(name[i]) || isdigit(name[i]) || name[i] == '_')) {
      std::cout << "invalid variable name" << std::endl;
      return false;
    }
  }

  return true;
}
void myShell::setVar(std::string argument) {
  //split the argument into key and value
  size_t blankInd = argument.find(" ");
  std::string key = argument.substr(0, blankInd);
  std::string value = argument.substr(blankInd + 1);
  //check the variable name legal or not
  bool valid = checkVarName(key);
  if (valid == false) {
    std::cout << "invalid variable name " << key << std::endl;
    return;
  }
  //update the map for key value pair
  std::map<std::string, std::string>::iterator it;
  it = varMap.find(key);
  if (it != varMap.end()) {
    varMap[key] = value;
  }
  else {
    varMap.insert(std::pair<std::string, std::string>(key, value));
  }
  std::cout << "set variable " << key << " to value " << value << std::endl;
}

//export command
void myShell::exportVar(std::string varName) {
  std::map<std::string, std::string>::iterator it;
  it = varMap.find(varName);
  if (it != varMap.end()) {
    std::cout << "it value is " << it->second << std::endl;
    if (setenv(it->first.c_str(), it->second.c_str(), 1) == -1) {
      perror("setenv error in export command\n");
    }
  }
  else {
    std::cout << "variable " << varName << "does not exit" << std::endl;
    return;
  }
}
int myShell::checkDigit(std::string value) {
  int valueDigit = 0;
  bool digit = true;
  for (size_t i = 0; i < value.size(); i++) {
    if (!isdigit(value[i])) {
      digit = false;
    }
  }
  if (digit == false) {
    std::cout << "the value is not digit , value be set as " << valueDigit << std::endl;
    return valueDigit;
  }
  else {
    std::cout << "value is digit" << std::endl;
    valueDigit = atoi(value.c_str());
  }

  return valueDigit;
}

void myShell::incInterpret(std::string argument) {
  //check the variable name legal or not
  bool valid = checkVarName(argument);
  if (valid == false) {
    return;
  }
  //check whether varMap has variable or not
  std::map<std::string, std::string>::iterator it;
  it = varMap.find(argument);
  if (it != varMap.end()) {
    std::ostringstream os;

    os << (checkDigit(it->second) + 1);
    varMap[argument] = os.str();
    std::cout << "inc the var " << argument << " to value " << varMap[argument] << std::endl;
  }
  else {
    varMap.insert(std::pair<std::string, std::string>(argument, "1"));
    std::cout << "variable " << argument << " does not exist" << std::endl;
    std::cout << "insert new variable to varMap "
              << "key is " << argument << " value is "
              << "1" << std::endl;
  }
}

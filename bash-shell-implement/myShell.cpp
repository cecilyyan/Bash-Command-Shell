#include "myShell.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
int main() {
  //execuate the shell program when true

  myShell shell;
  shell.getAllPossiblePath();
  while (true) {
    //show the prompt
    char * currentDir = getcwd(NULL, 0);
    std::cout << "myShell:" << currentDir << " $";
    //buffer to store the user command
    char commandBuf[1024];
    //read the strig from the stdin and store it into commandBuf
    std::cin.getline(commandBuf, 1024);

    std::string check_command = commandBuf;
    //if user enter "exit" or read EOF then exit the shell
    if (check_command.compare("exit") == 0 || std::cin.eof()) {
      free(currentDir);
      break;
    }
    else if (check_command.size() == 0) {
      free(currentDir);
      continue;
    }
    else if (check_command.find_first_not_of(" ") == std::string::npos ||
             check_command.compare(".") == 0 || check_command.compare("..") == 0) {
      std::cout << "invalid command shell input" << std::endl;
      free(currentDir);
      continue;
    }
    else {
      std::string userInput = commandBuf;

      //if the first character of userinput is dollar sign then print the variable value if exits
      if (userInput[0] == '$') {
        shell.printVar(userInput);
      }
      else {
        //replace the dollar sign with its value in userinput
        std::string replaceInput = shell.replaceVar(userInput);

        shell.parse(replaceInput);
        std::string firstCommand = shell.getFirstCommand();
        //cd command
        if (firstCommand.compare("cd") == 0) {
          shell.cdCommand(shell.getArgumentStr());
        }
        else if (firstCommand.compare("set") == 0) {
          if (check_command.compare("set") == 0) {
            std::cout << "please follow the format set varName value" << std::endl;
            free(currentDir);
            continue;
          }
          shell.setVar(shell.getArgumentStr());
        }
        else if (firstCommand.compare("export") == 0) {
          shell.exportVar(shell.getArgumentStr());
        }
        else if (firstCommand.compare("inc") == 0) {
          shell.incInterpret(shell.getArgumentStr());
        }
        else {
          std::string command = shell.getPath(firstCommand);
          if (command == "") {
            free(currentDir);

            continue;
          }
          char command_format[256];
          strcpy(command_format, command.c_str());
          shell.execuate(command_format);
        }
      }
    }
    free(currentDir);
  }

  return 0;
}

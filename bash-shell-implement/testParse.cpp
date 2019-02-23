#include <iostream>

int main(int argc, char ** argv) {
  std::cout << "it has " << argc << " elements." << std::endl;
  for (int i = 0; i < argc; i++) {
    std::cout << "argv[" << i << "] is " << argv[i] << std::endl;
  }

  return 0;
}

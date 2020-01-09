#include <stdlib.h>

#include <iostream>
#include <string>
using std::cout;
using std::string;

int main() {
  cout << "please input"
       << "\n";
  string s;
  while (std::getline(std::cin, s)) {
    cout << "your input is:" << s << "\n";
  }
  exit(EXIT_SUCCESS);
}

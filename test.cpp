#include <stdlib.h>

#include <iostream>
#include <string>
using std::cout;
using std::string;

int main(int argc, char * argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    cout << "argv[" << i << "]"
         << ":" << argv[i] << "\n";
  }
  exit(EXIT_SUCCESS);
}
// int main() {
//   cout << "please input"
//        << "\n";
//   string s;
//   while (std::getline(std::cin, s)) {
//     cout << "your input is:" << s << "\n";
//   }
//   exit(EXIT_SUCCESS);
// }

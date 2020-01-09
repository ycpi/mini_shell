//#include <stdlib.h>

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

class shell {
 private:
  string path_551;
  string cmd;
  unordered_map<string, string> var_tb;
  vector<string> argv;
  vector<string> path_vec;
  char ** envp;

  // get cmd, parse it, find
  void get_cmd(string & new_cmd);
  int parse_cmd(void);
  void search_and_replace();
  string replace_with_val(string arg);

  // built-in function
  int execute_build_in_cmd(void);
  void execute_cd();
  void execute_set();
  void execute_export();
  void execute_rev();
  string find_var_in_tb(string var_name);

  // execute file
  int run_cmd();
  void run_cmd_child();
  int execute_file(const char ** arr_argv, const char ** arr_envp);

 public:
  shell() {  // initialize ECE551PATH
    setenv("ECE551PATH", std::getenv("PATH"), 1);
  }
  int run_shell();
};

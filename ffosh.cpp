#include "ffosh.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using std::cout;
using std::pair;
using std::string;
using std::stringstream;

void print_exit_status(int wstatus) {
  if (WIFEXITED(wstatus)) {
    if (WEXITSTATUS(wstatus) == 0) {
      cout << "Program was successful\n";
    }
    else if (WEXITSTATUS(wstatus) != 0) {
      cout << "Program failed with code " << WEXITSTATUS(wstatus) << "\n";
    }
  }
  else if (WIFSIGNALED(wstatus)) {
    cout << "Terminated by signal " << WTERMSIG(wstatus) << "\n";
  }
}

const char ** convert_type(vector<string> & in) {
  // convert vector<string> to const char **
  const char ** out = new const char *[in.size() + 1];
  for (size_t i = 0; i < in.size(); i++) {
    out[i] = in[i].c_str();
  }
  out[in.size()] = NULL;
  return out;
}

void my_split(const string & str, vector<string> & container, char delim = ':') {
  // split a str into a vector<string> according to different delim
  stringstream ss(str);
  string tmp_part;
  while (std::getline(ss, tmp_part, delim)) {
    container.push_back(tmp_part);
  }
}

//void update_argv(size_t & i, string & tmp_cmd, string & args, vector<string> & argv)
void update_argv(string & args, vector<string> & argv) {
  // update argv
  if (args != "") {
    argv.push_back(args);
    args.clear();
  }
}

int shell::parse_cmd() {
  // parse cmd to argv
  // size_t i = 0;
  string args = "";
  string tmp_cmd = cmd;
  bool q_state = false;  // indicate if in quotation area
  char pre_char = ' ';
  string::iterator it = tmp_cmd.begin();
  while (it != tmp_cmd.end()) {
    char cur_char = *it;
    if (cur_char == '"') {
      if (pre_char == '\\') {  // add as usual
        args.push_back(cur_char);
        ++it;
      }
      else {  // according to current state, start or end quotation
        q_state = !q_state;
        it = tmp_cmd.erase(it);  // delete this " from str
      }
    }
    else if (cur_char == ' ') {
      if (q_state == false) {   // outside quotation
        if (pre_char != ' ') {  // deal with consective space
          //  update_argv(i, tmp_cmd, args, argv);
          update_argv(args, argv);
        }
        ++it;
      }
      else {  // add as usual
        args.push_back(cur_char);
        ++it;
      }
    }
    else if (cur_char == '\\') {
      if (pre_char == '\\') {  // add as usual
        args.push_back(cur_char);
        cur_char = '?';  // this \\ have been used
        ++it;
      }
      else {
        it = tmp_cmd.erase(it);  // delete this \ from str
      }
    }
    else {
      args.push_back(cur_char);
      ++it;
    }
    pre_char = cur_char;
  }
  if (q_state != false) {  // find unclosed quotation
    std::cerr << "find unclosed quotation\n";
    // exit(EXIT_FAILURE);
    return 0;
  }
  if (argv.size() == 0) {  // all args are white spaces
    return 0;
  }
  cmd = tmp_cmd;
  return 1;
}

int execve_cpp(const char ** arr_argv, const char ** arr_envp) {
  return execve(arr_argv[0],
                (char * const *)arr_argv,  // convert to required type first
                (char * const *)arr_envp);
}
int execvpe_cpp(const char ** arr_argv, const char ** arr_envp) {
  return execvpe(arr_argv[0],
                 (char * const *)arr_argv,  // convert to required type first
                 (char * const *)arr_envp);
}

bool check_exist(const char * file) {  // check if file exists
  if (FILE * f = fopen(file, "r")) {
    fclose(f);
    return true;
  }
  else {
    return false;
  }
}

string shell::find_var_in_tb(string var_name) {
  string val;
  // for (size_t v_i = var_name.size(); v_i > 0; v_i--) {
  //   if (var_tb.find(var_name) != var_tb.end()) {
  // std::reverse(val.begin(), val.end());
  //  val.insert(0, var_tb[var_name]);
  //   break;
  // }
  // val = val + var_name.back();  // char don't actually belong to variable
  // var_name.pop_back();
  //}
  if (var_tb.find(var_name) != var_tb.end()) {
    val = var_tb[var_name];
  }
  return val;
}

string shell::replace_with_val(string arg) {
  //replace value in the arg with its value
  string res, val;
  bool in_var = false;  // flag to show if all var have been converted to value
  string var_name;
  for (size_t j = 0; j < arg.size(); j++) {  // legal char
    bool if_legal_char = (arg[j] <= 57 && arg[j] >= 48) ||
                         (arg[j] >= 65 && arg[j] <= 90) || (arg[j] == '_') ||
                         (arg[j] >= 97 && arg[j] <= 122);
    if (in_var == true && if_legal_char) {  // in a varible name
      var_name.push_back(arg[j]);           // update var_name
    }
    else {
      if (in_var == true) {         // exist possible variable
        if (var_name.size() > 0) {  // search varible in table and add to arg
          val = find_var_in_tb(var_name);
          res = res + val;
          var_name.clear();  // clear old var_name
        }
        else {  // previous $ is not used for variable
          res.push_back('$');
        }
        if (arg[j] != '$') {  // char is not $
          in_var = false;     // change in_var state
          res.push_back(arg[j]);
        }
      }
      else {                  // don't exist possible variable at this time
        if (arg[j] == '$') {  //  possible new variable, change in_var state
          in_var = true;
          continue;
        }
        else {  // char will not be part of var_name, just add to arg
          res.push_back(arg[j]);
        }
      }
    }
  }
  return res;
}

void shell::search_and_replace() {
  // find possible variable in arg and replace it with value
  for (size_t i = 0; i < argv.size(); i++) {
    size_t pos = cmd.find(argv[i]);
    size_t original_len = argv[i].size();
    string arg = argv[i] + '$';  // make sure the last var is replaced

    argv[i] = replace_with_val(arg);
    cmd.replace(pos, original_len, argv[i]);  // modify cmd
  }
  vector<string>::iterator it = argv.begin();  // remove all empty args
  while (it != argv.end()) {
    if (*it == "") {
      it = argv.erase(it);
    }
    else {
      ++it;
    }
  }
}

int shell::execute_file(const char ** arr_argv, const char ** arr_envp) {
  const char * name = arr_argv[0];
  const char * find_c = strchr(name, '/');  // check if need to search in envp
  if (find_c != NULL) {                     // search in give directory
    if (check_exist(name)) {                // file exists, excute it
      return execve_cpp(arr_argv, arr_envp);
    }
    else {  // doesn't exist
      return 2222;
    }
  }
  else {  // search in path_551
    string name_str(name);
    if (name_str.size() == 0) {  // is not a file
      return 2222;
    }
    for (size_t i = 0; i < path_vec.size(); i++) {
      string path = path_vec[i];
      path.append("/");
      path.append(name_str);  // get path
      const char * path_c = path.c_str();
      if (check_exist(path_c)) {
        arr_argv[0] = path_c;  // replace the first arg
        return execve_cpp(arr_argv, arr_envp);
      }
    }
    // doesn't exist
    return 2222;
  }
}

void change_std_out(const vector<string> & redirect_args) {
  int file;
  if (redirect_args.size() >= 2) {
    if ((file = open(redirect_args[1].c_str(),
                     O_CREAT | O_WRONLY | O_TRUNC,
                     S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) != -1) {
      if (dup2(file, 1) != -1) {  // replace with stdout
        if (close(file) == -1) {
          perror("close");
        }
      }
      else {
        perror("dup2");
        exit(EXIT_FAILURE);
      }
    }
    else {
      perror("open");
      exit(EXIT_FAILURE);
    }
  }
  else if (redirect_args.size() == 1) {
    std::cerr << "redirection argument's number is incorrect";
    exit(EXIT_FAILURE);
  }
}

void change_std_in(const vector<string> & redirect_args) {
  int file;
  if (redirect_args.size() >= 2) {
    if ((file = open(redirect_args[1].c_str(), O_RDONLY)) != -1) {
      if (dup2(file, 0) != -1) {  // replace with stdin
        if (close(file) == -1) {
          perror("close");
        }
      }
      else {
        perror("dup2");
        exit(EXIT_FAILURE);
      }
    }
    else {
      perror("open");
      exit(EXIT_FAILURE);
    }
  }
  else if (redirect_args.size() == 1) {
    std::cerr << "redirection argument's number is incorrect";
    exit(EXIT_FAILURE);
  }
}

void change_std_error(const vector<string> & redirect_args) {
  int file;
  if (redirect_args.size() >= 2) {
    if ((file = open(redirect_args[1].c_str(),
                     O_CREAT | O_WRONLY | O_TRUNC,
                     S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) != -1) {
      if (dup2(file, 2) != -1) {  // replace with stderror
        if (close(file) == -1) {
          perror("close");
        }
      }
      else {
        perror("dup2");
        exit(EXIT_FAILURE);
      }
    }
    else {
      perror("open");
      exit(EXIT_FAILURE);
    }
  }
  else if (redirect_args.size() == 1) {
    std::cerr << "redirection argument's number is incorrect";
    exit(EXIT_FAILURE);
  }
}

void check_redirect(vector<string> & argv) {
  vector<string> redirect_args;
  vector<string>::iterator it;
  for (it = argv.begin(); it != argv.end(); ++it) {
    if (*it == ">" || *it == "<" || *it == "2>") {  // try to find >
      break;
    }
  }
  while (it != argv.end()) {
    redirect_args.push_back(*it);  // push > and following arguments into vector
    it = argv.erase(it);           // delete these arguments in argv
  }
  if (redirect_args.size() > 0) {
    // replace different std according to the first args
    if (redirect_args[0] == ">") {
      change_std_out(redirect_args);
    }
    else if (redirect_args[0] == "<") {
      change_std_in(redirect_args);
    }
    else {
      change_std_error(redirect_args);
    }
  }
}

void shell::run_cmd_child() {
  check_redirect(argv);

  const char ** arr_argv = convert_type(argv);

  int status = execute_file(arr_argv, (const char **)envp);
  // if fail
  delete[] arr_argv;
  //    delete[] arr_envp;
  if (status == 2222) {
    cout << "Command " << argv[0] << " not found\n";
  }
}

int shell::run_cmd() {
  pid_t pid_c, pid_w;
  int wstatus;

  pid_c = fork();  // duplicate
  if (pid_c == -1) {
    perror("fork error");
    exit(EXIT_FAILURE);
  }
  else if (pid_c == 0) {  // child process
    run_cmd_child();
    exit(EXIT_FAILURE);
  }
  else {  // parent process, pid_c is the pid of child process
    do {
      pid_w = waitpid(pid_c, &wstatus, WUNTRACED | WCONTINUED);
      if (pid_w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    return wstatus;  // return the status of child process
  }
}

void shell::get_cmd(string & new_cmd) {
  // get cmd
  cmd = new_cmd;
  // skip blank spaces in the begining of str
  size_t char_ind = cmd.find_first_not_of(" ", 0);
  if (char_ind != string::npos) {
    cmd = cmd.substr(char_ind);
    cmd.push_back(' ');  // ensure last args can be added to argv
  }
}

void print_arg_error(size_t size, size_t rule) {
  if (size < rule) {
    std::cerr << "please input enough arguments\n";
  }
  if (size > rule) {
    std::cerr << "too many arguments\n";
  }
}

void shell::execute_cd() {
  if (argv.size() == 2) {  // excute cd
    if (chdir(argv[1].c_str()) == 0) {
    }
    else {
      perror("cd");
    }
  }
  else {  // wrong number of arguments
    print_arg_error(argv.size(), 2);
  }
}

int check_var_name(string var) {
  // check if variable name is legal
  if (var.size() == 0) {
    return 0;
  }
  for (size_t j = 0; j < var.size(); j++) {
    bool if_legal_char = (var[j] <= 57 && var[j] >= 48) ||
                         (var[j] >= 65 && var[j] <= 90) || (var[j] == '_') ||
                         (var[j] >= 97 && var[j] <= 122);
    if (!if_legal_char) {
      return 0;
    }
  }
  return 1;
}

void shell::execute_set() {
  if (argv.size() < 2) {  // wrong number of arguments
    std::cerr << "please input enough arguments\n";
  }
  else {                                 // excute set
    if (check_var_name(argv[1]) == 0) {  // check if variable name is legal
      std::cerr << "variable name is not legal\n";
    }
    else {
      if (argv.size() == 2) {  // don't have value
        var_tb[argv[1]] = "";
      }
      else {
        size_t pos = cmd.find(argv[1]);
        size_t val_pos = cmd.find_first_not_of(" ", pos + argv[1].size());
        string val = cmd.substr(val_pos);  // val is the rest of cmd, skip spaces
        val.pop_back();                    // delete the last space that we add before
        var_tb[argv[1]] = val;             // create the new one or replace the old one
      }
    }
  }
}

void shell::execute_export() {
  if (argv.size() == 2) {                        // excute export
    if (var_tb.find(argv[1]) != var_tb.end()) {  // set env
      setenv(argv[1].c_str(), var_tb[argv[1]].c_str(), 1);
    }
    else {
      std::cerr << "cannot find variable\n";
    }
  }
  else {  // wrong number of arguments
    print_arg_error(argv.size(), 2);
  }
}

void shell::execute_rev() {
  if (argv.size() == 2) {                        // excute rev
    if (var_tb.find(argv[1]) != var_tb.end()) {  // set env
      std::reverse(var_tb[argv[1]].begin(), var_tb[argv[1]].end());
    }
    else {
      std::cerr << "cannot find variable\n";
    }
  }
  else {  // wrong number of arguments
    print_arg_error(argv.size(), 2);
  }
}

int shell::execute_build_in_cmd() {
  // check if user is executing build-in cmd, return different int to indicate status
  // 0--execute cmd and should quit shell
  // 1--execute cmd and wait for next input cmd
  // 2--it is not build-in cmd
  string exit_cmd = "exit";
  string cd_cmd = "cd";
  string set_cmd = "set";
  string export_cmd = "export";
  string rev_cmd = "rev";

  if (exit_cmd == argv[0]) {  // excute exit
    return 0;
  }

  if (cd_cmd == argv[0]) {  // excute cd
    execute_cd();
    return 1;
  }

  if (set_cmd == argv[0]) {  // excute set
    // execute_cd();
    execute_set();
    return 1;
  }

  if (export_cmd == argv[0]) {  // excute export
    execute_export();
    return 1;
  }

  if (rev_cmd == argv[0]) {  // excute cd
    execute_rev();
    return 1;
  }
  return 2;
}

int shell::run_shell() {
  int child_status;
  int build_in_cmd_status;
  char * cur_dir = NULL;

  do {
    cur_dir = get_current_dir_name();
    cout << "ffosh:" << cur_dir << "$ ";
    free(cur_dir);
    string tmp_cmd;
    if (std::getline(std::cin, tmp_cmd)) {
      argv.clear();      // clear previous cmd's argv
      path_vec.clear();  // clear previous path_vec
      envp = environ;    // update envp

      get_cmd(tmp_cmd);

      path_551 = std::getenv("ECE551PATH");  // update path
      my_split(path_551, path_vec, ':');     // split path_551 to vector

      if (parse_cmd() == 0) {  // get argv
        continue;              // something wrong, skip this cmd
      }

      search_and_replace();  // find variable
      if (argv.size() == 0) {
        continue;
      }

      build_in_cmd_status = execute_build_in_cmd();
      if (build_in_cmd_status == 0) {  // get next cmd or exit
        break;
      }
      else if (build_in_cmd_status == 1) {
        continue;
      }

      child_status = run_cmd();

      print_exit_status(child_status);
    }
    else {  // eof
      break;
    }
  } while (true);

  exit(EXIT_SUCCESS);
}

extern char ** environ;  // shell environment

int main() {
  shell my_sh;
  my_sh.run_shell();
}

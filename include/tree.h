#include <string>
#include <iostream>
using namespace std;
extern int tempFile, emptyFolder;
extern int max_length;
extern string longestPath;
extern int dcnt, fcnt; // 计算文件和文件夹的数量
extern int max_binary_tree_depth;
extern int max_depth;
void traversal(int traversal_times);
void get_files_info(string file_name);
void modify_files();
void modify_dir();
void scan();
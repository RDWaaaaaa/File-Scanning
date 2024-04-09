#include<map>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <fstream> // 文件流
#include <chrono>
#include <stack>
#include <format>
#include <string>
#include <ranges> 
#include <cctype> 
using namespace std;

struct node{
    filesystem::path path;//路径
    node * next;//孩子兄弟树中的兄弟指针
    node * son;//孩子兄弟书中的孩子指针
    int id;//文件或文件夹的编号
    filesystem::file_time_type lastwritetime;//记录节点上次修改的时间
    int depth;//节点在目录树中的层数
    long long filesize;//如果节点是文件，记录文件大小
    char type;//节点类型，'D'表示目录，'F'表示文件
    int binary_tree_depth;//节点在孩子兄弟树中的深度
};
extern node *head;
extern map<string,node*>mp;

node * get_new_node(filesystem::path path,node * next,node * son,
    int id,filesystem::file_time_type lastwritetime,int depth,
    long long filesize,char type,int binary_tree_depth);
void delete_node(node * p);
void initSqlFile(ofstream& sqlFile);
string to_lowercase(const string& str);
string convert(const string& path);
bool isTemporaryFile(const filesystem::directory_entry& entry);
bool isEmptyFolder(const filesystem::directory_entry& entry);
void compare_result(string path_str1,string path_str2);
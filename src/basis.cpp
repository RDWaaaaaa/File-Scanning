#include "basis.h"
#include<string>
using namespace std;
map<string,node*>mp;
node *head = NULL;

node * get_new_node(filesystem::path path,node * next,node * son,int id,filesystem::file_time_type lastwritetime,int depth,long long filesize,char type,int binary_tree_depth){//新建节点
    node *p = new node;
    p -> path = path;
    p -> next = next;
    p -> son = son;
    p -> id = id;
    p ->lastwritetime = lastwritetime;
    p -> depth = depth;
    p -> filesize = filesize;
    p -> type = type;
    p -> binary_tree_depth = binary_tree_depth;
    return p;
}

string to_lowercase(const string& str) {// 把一个字符串转换为全部小写
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return tolower(c); });
    return result;
}

void delete_node(node * p){ // 删掉p为根节点的子树
    stack<node*>stk;
    stk.push(p);
    while(!stk.empty()){//bfs
        auto t = stk.top();
        stk.pop();
        if(t->next){
            stk.push(t->next);//两个子节点入栈
        }
        if(t->son){
            stk.push(t->son);//两个子节点入栈
        }
        mp.erase(to_lowercase(t -> path.string()));//map中删除当前节点
        delete t;//释放当前节点空间

    }
}

string convert(const string& path) { // 调整转义符个数
    string result;
    for(char c : path) {
        if(c == '\\') {
            result += "\\\\";//将一个转义符变成两个
        } else {
            result += c;
        }
    }
    return result;
}

void initSqlFile(ofstream& sqlFile) {// 初始化SQL文件，创建文件、文件夹的表
    
    sqlFile << "DROP DATABASE IF EXISTS test;"<<endl
            << "CREATE DATABASE test;"<<endl
            << "USE test;"<<endl;
    // 创建文件夹表
    sqlFile << "CREATE TABLE Folders ("
            << "FolderID INT PRIMARY KEY NOT NULL,"//编号，主码
            << "FolderName VARCHAR(255) NOT NULL,"//文件夹名称
            << "FolderPath VARCHAR(255) NOT NULL,"//路径
            << "LastWriteTime TIMESTAMP,"//最新修改时间
            << "PreDirID INT,"//父目录编号
            << "FOREIGN KEY (PreDirID) REFERENCES Folders(FolderID)"//父目录编号参考目录编号
            << ");" << endl;
    // 创建文件表
    sqlFile << "CREATE TABLE Files ("
            << "FileID INT PRIMARY KEY NOT NULL,"//编号，主码
            << "FileName VARCHAR(255) NOT NULL,"//文件名称
            << "FilePath VARCHAR(255) NOT NULL,"//路径
            << "FileSize BIGINT NOT NULL,"//文件大小
            << "FileType VARCHAR(50),"//文件类型
            << "LastWriteTime TIMESTAMP,"//最新修改时间
            << "FolderID INT,"//父目录编号
            << "FOREIGN KEY (FolderID) REFERENCES Folders(FolderID)"//父目录编号参考目录编号
            << ");" << endl;
}

bool isTemporaryFile(const filesystem::directory_entry& entry) {// 判断是否为临时文件
    std::string extension = entry.path().extension().string();
    std::initializer_list<std::string> tempExtensions = {".tmp", ".temp", ".bak", ".tempfile",".bak"};
    for (const auto& ext : tempExtensions) {// 检查文件扩展名是否在临时文件列表中
        if (extension == ext) {
            return true;
        }
    }
    return false;
}

bool isEmptyFolder(const filesystem::directory_entry& entry) {//判断是否为空文件夹
    return filesystem::is_empty(entry.path());
}

void compare_result(string path_str1,string path_str2){//逐行比较两个文件
    ifstream file1(path_str1);
    ifstream file2(path_str2);
    ofstream result("output\\compare_result.csv", ios::app);
    result<<"编号,"<<"路径,"<<"状态,"<<"文件数量,"<<"总文件大小,"<<"最早文件名,"<<"最早文件大小,"<<"最早文件时间,"<<"最晚文件名,"<<"最晚文件大小,"<<"最晚文件时间"<<endl;
    string line1,line2;
    while (getline(file1, line1)&&getline(file2,line2)){
        if(line1 == line2)continue;
        else{
            result<<line1<<endl<<line2<<endl;//两行不一样则输出到compare_result.csv中
        }
    }
    cout<<"对比完成,结果在"<<"output\\compare_result.csv"<<endl;
}
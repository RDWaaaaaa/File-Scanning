#include "basis.h"
#include "tree.h"
using namespace std;

int tempFile = 0, emptyFolder = 0;
int max_length = 0;
string longestPath;
int dcnt = 0, fcnt = 0; // 计算文件和文件夹的数量
int max_binary_tree_depth = 0;
int max_depth = 0;

void scan(){// 扫描并生成目录树，生成dir_list.csv存储每个文件夹的详细信息,file_list.csv存储每个文件的详细信息，并生成insert_data.sql
    cout<<"每输出一个D或F表示已扫描10,000个文件夹或文件"<<endl;
    string path = "C:\\Windows"; // 目标路径

    ofstream dir_list("output\\dir_list.csv"); //存储每个文件夹的详细信息
    ofstream file_list("output\\file_list.csv");//存储每个文件的详细信息
    dir_list<<"No.,path,last_write_time"<<endl;//表头
    file_list<<"No.,path,last_write_time"<<endl;//表头
    ofstream sqlFile("output\\insert_data.sql");//生成insert_data.sql
    initSqlFile(sqlFile);//初始化insert_data.sql
    stack<node *> dirs; // bfs中栈用于存储待遍历的目录

    filesystem::directory_entry entry(path);
    head = get_new_node(path,NULL,NULL,0,filesystem::last_write_time(entry),0,0,'D',0);//头节点赋值
    mp[to_lowercase(entry.path().string())] = head;//存储路径，方便查找
    sqlFile << "INSERT INTO Folders VALUES ("
        << 0 << ", "
        << "'" << convert(entry.path().filename().string()) << "', "
        << "'" << convert(entry.path().string()) << "', "
        << "'" << format("{:%Y-%m-%d %H:%M:%S}", entry.last_write_time()) << "', "
        << "NULL"
        << ");" << std::endl;//头节点记录
    dirs.push(head);
    while (!dirs.empty()) {//bfs
        auto last = dirs.top();
        auto father_p = dirs.top();
        dirs.pop();
        try {
            max_depth = max(max_depth, last -> depth);//维护最大目录层数
            //遍历当前目录下的子目录及文件
            for (const auto& entry : filesystem::directory_iterator(father_p->path, filesystem::directory_options::skip_permission_denied | filesystem::directory_options::follow_directory_symlink)) {
                string entry_path_str = entry.path().string();
                int length = entry_path_str.size();
                if(length > max_length){
                    max_length = length;
                    longestPath = entry_path_str;
                }
                auto ftime = filesystem::last_write_time(entry);
                if(ftime == std::filesystem::file_time_type::min())continue;
                node * now =  get_new_node(entry.path(),NULL,NULL,-1,ftime,(father_p -> depth) + 1,0,'0',(last -> binary_tree_depth) +1);
                if(last == father_p){//连接孩子兄弟树
                    father_p ->son = now;
                }
                else{
                    last -> next = now;
                }
                // 如果是目录
                if (entry.is_directory()) {
                    if(isEmptyFolder(entry)){
                        emptyFolder++;
                        // continue;
                    }
                    dcnt++;
                    dirs.push(now); 
                    if (dcnt % 10000 == 0) {
                        cout << "D";
                    }
                    now -> id = dcnt;
                    now -> type = 'D';
                    dir_list<< dcnt <<","  << entry_path_str  << "," << format("{:%Y-%m-%d %H:%M:%S}", ftime)<<endl;
                    sqlFile << "INSERT INTO Folders VALUES ("
                            << dcnt << ","
                            << "'" << convert(entry.path().filename().string()) << "', "
                            << "'" << convert(entry.path().string()) << "', "
                            << "'" << format("{:%Y-%m-%d %H:%M:%S}", ftime) << "',"
                            << father_p -> id<< ");" << endl;
                } 
                else {//如果是文件
                    
                    if (fcnt % 10000 == 0) {
                        cout << "F";
                    }
                    if(isTemporaryFile(entry)){
                        tempFile++;
                        // continue;
                    }
                    fcnt++;
                    now -> id = fcnt;
                    now -> type = 'F';
                    now -> filesize = (long long)entry.file_size();
                    file_list << fcnt <<"," << entry_path_str << "," << format("{:%Y-%m-%d %H:%M:%S}", ftime) <<endl;
                    sqlFile << "INSERT INTO Files VALUES ("
                            << fcnt << ","
                            << "'" << convert(entry.path().filename().string()) << "', "
                            << "'" << convert(entry.path().string()) << "', "
                            << entry.file_size() << ", "
                            << "'" << convert(entry.path().extension().string()) << "', "
                            << "'" << format("{:%Y-%m-%d %H:%M:%S}", ftime) << "',"
                            << father_p -> id << ");" << endl;
                }
                mp[to_lowercase(entry.path().string())] = now;//mp中添加当前路径，方便查找
                last = now;
            }
        } 
        catch (const exception& e) {
            // 输出错误信息到终端
            // cerr << "\n遍历目录时发生错误: " << e.what() << "\n";
        }
    }
    
    dir_list.close();
    file_list.close();
    sqlFile.close();

    //输出统计结果
    cout << endl << "扫描完成，统计结果如下" << endl;
    cout << endl << "文件树的最大深度为：" << max_depth << endl;
    cout << "总文件数量：" << fcnt << endl;
    cout << "总文件夹数量：" << dcnt << endl;
    cout << "其中空文件夹" << emptyFolder << "个" << endl;
    cout << "临时文件" << tempFile << "个" << endl;
    cout << "最长路径" << max_length << endl;
    cout << "（可能不唯一）最长路径是" << longestPath << endl<< endl;
}

void traversal(int traversal_times){// 遍历目录树，将结果存到traversal_log.csv
    
    cout<<endl<<"遍历"<<endl;
    ofstream log("output\\traversal_log.csv", ios::app);
    if(traversal_times == 1){
        log<<"No.,孩子兄弟树最大深度,总文件数,总文件大小,最早文件名,最早文件大小,最早文件修改时间,最晚文件名,最晚文件大小,最晚文件修改时间"<<endl;
    }
    int sum_cnt = 0;
    long long sum_size = 0;
    node * earliest = NULL;
    node * latest = NULL;
    stack<node *> stk;
    stk.push(head);
    while(!stk.empty()){//bfs
        auto now = stk.top();
        stk.pop();
        if(now -> type == 'F'){
            sum_cnt++;
            sum_size += now -> filesize;
            if(earliest == NULL || earliest -> lastwritetime > now -> lastwritetime)earliest = now;
            if(latest == NULL || latest -> lastwritetime < now -> lastwritetime)latest = now;
        }
        max_binary_tree_depth = max(max_binary_tree_depth,now -> binary_tree_depth);
        if(now->son){
            if(now->son)stk.push(now->son);//子节点入栈
        }
        if(now->next){
            if(now->next)stk.push(now->next);
        }
    }
    //输出日志
    log << traversal_times<<","
        << max_binary_tree_depth<<","
        << sum_cnt<<","
        << sum_size<<","
        <<earliest->path.filename().string()<<","
        <<earliest->filesize<<","
        <<format("{:%Y-%m-%d %H:%M:%S}", earliest->lastwritetime)<<","
        <<latest->path.filename().string()<<","
        <<latest->filesize<<","
        <<format("{:%Y-%m-%d %H:%M:%S}", latest->lastwritetime)<<endl;
    cout<<"遍历完成"<<endl;
    cout<<"已在"<<" output\\traversal_log.csv "<<"中写入一条记录"<<endl;
} 

void get_files_info(string file_name){// 得到mystat指定目录对应信息,将结果存到files_infox.csv中，其中x表示第x次生成的文件
    cout<<"开始查找"<<endl;
    ifstream file("input\\mystat.txt");
    if (!file.is_open()) {// 检查文件是否成功打开
        cerr <<"Unable to open the file"<<endl;
        return;
    }
    ofstream info(file_name);
    string path_str;
    int cnt = 0;
    info<<"编号,"<<"路径,"<<"状态,"<<"文件数量,"<<"总文件大小,"<<"最早文件名,"<<"最早文件大小,"<<"最早文件时间,"<<"最晚文件名,"<<"最晚文件大小,"<<"最晚文件时间"<<endl;
    while (getline(file, path_str)) { // 逐行读取文件内容
        string str = to_lowercase(path_str);
        if(str[0]!='c')continue;
        if (!str.empty()) {
            str.erase(str.size() - 1);//将格式转换为map中存储的格式
        }
        else{
            cerr <<"error"<<endl;
        }
        node *p = mp[str];
        cnt++;
        info<<cnt<<",";
        info<<path_str<<",";
        if(!p){
            info<<"查找出错"<<endl;
        }
        else if(!p->son){
            info<<"空"<<endl;
        }
        else{
            info<<"正常,";
            node * earliest = NULL;
            node * latest = NULL;
            p = p->son;
            int sum_cnt = 0;
            long long sum_size =0;
            while(p){
                if(p->type == 'F'){
                    sum_cnt++;
                    sum_size += p->filesize;
                    if(earliest == NULL || earliest -> lastwritetime > p-> lastwritetime){
                        earliest = p;
                    }
                    if(latest == NULL || latest -> lastwritetime < p -> lastwritetime){
                        latest = p;
                    }
                }
                p = p -> next;
            }
            info << sum_cnt << ",";
            info << sum_size;
            if(sum_cnt){//如果有文件
                info <<","<<earliest->path.filename().string()<<","
                <<earliest->filesize<<","
                <<format("{:%Y-%m-%d %H:%M:%S}", earliest->lastwritetime)<<",";
                info <<latest->path.filename().string()<<","
                <<latest->filesize<<","
                <<format("{:%Y-%m-%d %H:%M:%S}", latest->lastwritetime)<<endl;
            }
            else{//文件夹下有文件夹，但是没有文件的情况
                info<<endl;
            }
                
        }
    }
    file.close(); // 关闭文件
    info.close();
    cout<<"查找完成，结果在"<<file_name<<endl<<endl;
}

void modify_files(){// 根据modify_files.txt进行文件操作，生成日志文件modify_files_log.txt
    ifstream file("input\\modify_files.txt");
    if (!file.is_open()) {
        cout<<"文件打开失败"<<endl;
        return;
    }
    ofstream log("output\\modify_files_log.txt", ios::app);
    string file_info_str;
    while (getline(file, file_info_str)){
        log<<endl<<"收到的字符串："<<endl;
        log<<file_info_str<<endl;
        stringstream ss(file_info_str);
        string token;
        vector<string> tokens;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        string path_str = tokens[0];
        string op = tokens[1];
        string time = tokens[2];
        string size = tokens[3];
        if(op=="M"){//修改节点
            long long new_size = stoll(size);
            long long new_time = stoll(time);
            node * p = mp[to_lowercase(path_str)];
            if(p){
                log<<"修改前："<<endl;
                log<<format("{:%Y-%m-%d %H:%M:%S}",p->lastwritetime)<<endl;
                log<<p->filesize<<endl;
                
                // 将 long long 类型转换为 time_t 类型
                time_t timestamp = static_cast<time_t>(new_time);
                struct tm *local_time = std::localtime(&timestamp);
                // 将 time_t 转换为 std::chrono::system_clock::time_point
                chrono::system_clock::time_point sys_time = chrono::system_clock::from_time_t(timestamp);
                // 将 system_clock::time_point 转换为 std::filesystem::file_time_type
                filesystem::file_time_type file_time = filesystem::file_time_type::clock::from_sys(sys_time);
                p -> filesize = new_size;
                p -> lastwritetime = file_time;
                log<<"修改后："<<endl;
                log<<format("{:%Y-%m-%d %H:%M:%S}",p->lastwritetime)<<endl;
                log<<p->filesize<<endl;
                p -> filesize = new_size;
            }
            else{
                log<<"查找出错"<<endl;
            }
        }
        else if(op=="A"){//增加节点
            long long new_size = stoll(size);
            long long new_time = stoll(time);
            filesystem::path path = path_str;
            filesystem::path parent_path = path.parent_path();
            string parent_path_str = parent_path.string();
            node * p = mp[to_lowercase(parent_path_str)];
            time_t timestamp = static_cast<time_t>(new_time);
            struct tm *local_time = std::localtime(&timestamp);
            // 将 time_t 转换为 std::chrono::system_clock::time_point
            chrono::system_clock::time_point sys_time = chrono::system_clock::from_time_t(timestamp);
            // 将 system_clock::time_point 转换为 std::filesystem::file_time_type
            filesystem::file_time_type file_time = filesystem::file_time_type::clock::from_sys(sys_time);
            if(p){
                fcnt++;
                node * now = get_new_node(path,NULL,NULL,fcnt,file_time,p -> depth,new_size,'F',p -> binary_tree_depth + 1);
                if(p->son){
                    p=p->son;
                    while(p->next){
                        p = p -> next;
                    }
                    p->next = now;
                    
                }
                else{
                    p->son = now;
                }
                log<<"添加完成！"<<endl;
            }
            else{
                log<<"查找出错"<<endl;
            }
        }
        else if(op=="D"){//删除节点
            filesystem::path path = path_str;
            filesystem::path parent_path = path.parent_path();
            string parent_path_str = parent_path.string();
            node * p = mp[to_lowercase(parent_path_str)];
            if(p){
                if(p->son){
                    node * p1 = p -> son;
                    if(to_lowercase(p1 -> path.string()) == to_lowercase(path_str)){
                        p->son = p1 -> next;
                        mp.erase(to_lowercase(p1 -> path.string()));
                        delete p1;
                    }
                    else{
                        while(p1->next){
                            if(to_lowercase(p1 -> next -> path.string()) == to_lowercase(path_str)){
                                break;
                            }
                            p1 = p1 -> next;
                        }
                        if(p1->next){
                            p = p1 -> next;
                            if(to_lowercase(p1 ->next -> path.string()) == to_lowercase(path_str)){
                                p1 -> next = p -> next;
                                mp.erase(to_lowercase(p -> path.string()));
                                delete p;
                                log<<"删除完成"<<endl;
                            }
                            else{
                                log<<"查找失败4"<<endl;
                            }
                        }
                        else{
                            log<<"查找失败3"<<endl;
                        }
                    }
                }
                else{
                    log<<"查找出错2"<<endl;
                }
            }
            else{
                log<<"查找出错"<<endl;
            }

        }
    }
    cout<<"文件已按照"<<"input\\modify_files.txt"<<"要求修改完成,修改日志在"<<"output\\modify_files_log.txt"<<endl;
}

void modify_dir(){// 根据modify_dir.txt进行文件操作，生成日志文件modify_dir_log.txt
    ifstream file("input\\modify_dir.txt");
    if (!file.is_open()) {
        cout<<"文件打开失败"<<endl;
        return;
    }
    ofstream log("output\\modify_dir_log.txt", ios::app);
    string dir_info_str;
    while(getline(file, dir_info_str)){
        log<<endl<<"收到的字符串："<<endl;
        log<<dir_info_str<<endl;
        stringstream ss(dir_info_str);
        string token;
        vector<string> tokens;
        while (std::getline(ss, token, ',')) {//将输入的信息拆分成几个字符串
            tokens.push_back(token);
        }
        string path_str = tokens[0];
        string op = tokens[1];
        string time = tokens[2];
        string size = tokens[3];
        if (!path_str.empty()) {
                path_str.erase(path_str.size() - 1);
        }
        filesystem::path path = path_str;
        filesystem::path parent_path = path.parent_path();
        string parent_path_str = parent_path.string();
        node * p = mp[to_lowercase(parent_path_str)];
        if(p){
            if(p->son){
                node * p1 = p -> son;
                if(to_lowercase(p1 -> path.string()) == to_lowercase(path_str)){
                    p->son = p1 -> next;
                    if(p1->son){
                        delete_node(p1->son);
                    }
                    mp.erase(to_lowercase(p1 -> path.string()));
                    delete p1;
                    log<<"删除完成"<<endl;
                }
                else{
                    while(p1->next){
                        if(to_lowercase(p1 -> next -> path.string()) == to_lowercase(path_str)){
                            break;
                        }
                        p1 = p1 -> next;
                    }
                    if(to_lowercase(p1 ->next -> path.string()) != to_lowercase(path_str)){
                        log<<"查找出错3"<<endl;
                    }
                    p = p1 -> next;
                    p1 -> next = p -> next;
                    if(p -> son){
                        delete_node(p -> son);
                    }
                    mp.erase(to_lowercase(p -> path.string()));
                    delete p;
                    log<<"删除完成"<<endl;
                }
            }
            else{
                log<<"查找出错2"<<endl;
            }
        }
        else{
            log<<"查找出错1"<<endl;
        }
    }
    cout<<"文件夹已按照"<<"input\\modify_dir.txt"<<"要求修改完成,修改日志在"<<"output\\modify_dir_log.txt"<<endl;
    return ;
}

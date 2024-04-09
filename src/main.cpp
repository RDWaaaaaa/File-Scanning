#include "basis.h"
#include "tree.h"
using namespace std;

int main() {
    int op,cnt = 0;
    head = NULL;
    int traversal_times = 0;
    int flag = 0;
    int opcnt = 1;
    while(1){
        cout<<"-------------------------------------------------------------------"<<opcnt<<endl;
        opcnt++;
        cout<<"0 结束"<<endl
            <<"1 扫描并生成目录树"<<endl
            <<"2 遍历目录树"<<endl
            <<"3 得到mystat指定目录对应信息"<<endl
            <<"4 将两次mystat对应信息进行对比"<<endl
            <<"5 根据modify_files.txt进行文件操作"<<endl
            <<"6 根据modify_dir.txt进行文件操作"<<endl;
        cout<<endl;
        cout<<"建议操作顺序："<<endl
            <<"   1 -> 2 -> 3 "<<endl<<"-> 5 -> 2 -> 3 "<<endl<<"-> 6 -> 2 -> 3"<<endl
            <<"而后输入命令 4 1 2 、4 2 3  进行文件 1、2 对比、文件 2、3 对比"<<endl
            <<"最后以0结束"<<endl<<endl;
        cin>>op;
        auto start_time = chrono::steady_clock::now(); // 获取开始时间
        if(op == 0){// 结束
            if(head)delete_node(head);
            break;
        }
        else if (op == 1){// 扫描并生成目录树，生成dir_list.csv存储每个文件夹的详细信息,
                        //file_list.csv存储每个文件的详细信息，并生成insert_data.sql
            if(flag){
                cout<<"已经扫描完成，无需再次扫描"<<endl;
            }
            else{
                scan();
                flag = 1;
            }    
            
        }
        else if(op == 2){// 遍历目录树，将结果存到traversal_log.csv
            traversal_times++;
            traversal(traversal_times);
        }
        else if(op == 3){// 得到mystat指定目录对应信息,将结果存到files_infox.csv中，其中x表示第x次生成的文件
            cnt++;
            string output_path = "output\\files_info" + to_string(cnt) + ".csv";
            get_files_info(output_path);
        }
        else if(op == 4){// 将两次mystat对应信息，即两个files_info.txt文件进行对比,结果存到compare_result.csv中
            cout<<"请输入两个数字表示哪两次结果进行对比"<<endl;
            int f1,f2;
            cin>>f1>>f2;
            string str1 = "output\\files_info" + to_string(f1) + ".csv";
            string str2 = "output\\files_info" + to_string(f2) + ".csv";
            compare_result(str1,str2);
        }
        else if(op == 5){// 根据modify_files.txt进行文件操作，生成日志文件modify_files_log.txt
            modify_files();
        }
        else if(op == 6){// 根据modify_dir.txt进行文件操作，生成日志文件modify_dir_log.txt
            modify_dir();
        }
        auto end_time = chrono::steady_clock::now(); // 获取结束时间
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        cout << "用时：" << duration.count() << " 毫秒" << endl<<endl; // 输出总共用时到终端
    }  
    
    return 0;
}
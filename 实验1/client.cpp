#include <iostream>
#include <winsock2.h>
#include <thread>
#include <functional>
#include <string>
#include <windows.h>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define BUF_SIZE 4000

string username;
bool is_connected = false; 
SOCKET u_socket;
//接收
void receive(SOCKET u_socket) {
    char buffer[BUF_SIZE];
    while (is_connected) {
        int receive = recv(u_socket, buffer, BUF_SIZE, 0);
        if (receive > 0) {
            buffer[receive] = '\0';
            cout << buffer;
        }
        else break;
    }
}
//连接
void connect(string &ip) {
    struct sockaddr_in addr;
    u_socket = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4444);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (!connect(u_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        cout << "\033[34m 服务器" << ip << "连接成功！\033[0m \n";
        cout <<"在连接状态下，输入\\d即可断开连接。\n";
        send(u_socket, username.c_str(), username.size(), 0);
        is_connected = true;

        thread recv_thread(receive, u_socket);
        recv_thread.detach();
    } 
    else cout << "\033[31m 无法连接服务器 " << ip << " ! \033[0m \n";
}

int main(){
    //创建wsa存储socket数据
    WSADATA wsa;
    string input;
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA创建失败！ \033[0m \n";
        return 0;
    }
    
    cout<< "[欢迎使用Socket多人聊天程序--用户端 by 2211290姚知言] \n";
    cout<< "输入你的用户名: ";
    getline(cin, username); 
    cout<<"欢迎"<<username<<"!\n";
    cout << "命令列表(在未连接状态下):\n"<< "\\c : 输入服务器IP地址并连接\n"<< "\\r : 更改用户名\n"<< "\\q : 退出程序\n";
    while (true) {
        getline(cin, input);
        if (!is_connected) {
            if (input=="\\c") {
                cout << "请输入服务器IP地址(默认为127.0.0.1，若无需更改，可直接回车): ";
                getline(cin, input);
                if(input=="") input="127.0.0.1";
                connect(input);
            } 
            else if (input == "\\r") {
                cout << "请输入新的用户名: ";
                getline(cin, username); 
                cout<<"欢迎"<<username<<"!\n";
            }
            else if (input == "\\q") {
                std::cout << "程序将在5s后退出，感谢你的使用！\n";
                Sleep(5000);
                break;
            }
            else cout << "命令列表(在未连接状态下):\n"<< "\\c : 输入服务器IP地址并连接\n"<< "\\r : 更改用户名\n"<< "\\q : 退出程序\n";
        } 
        else {
            if (input == "\\d") {
                string message= "\\d";
                send(u_socket, message.c_str(), message.size(), 0);
                is_connected = false;
                cout << "已退出聊天！\n";
                cout << "命令列表(在未连接状态下):\n"<< "\\c : 输入服务器IP地址并连接\n"<< "\\r : 更改用户名\n"<< "\\q : 退出程序\n";
            }
            else{
                string message = username + ": " + input + "\n";
                send(u_socket, message.c_str(), message.size(), 0);
            }
        }
    }
    WSACleanup();
    return 0;
}
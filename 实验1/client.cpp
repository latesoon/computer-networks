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
//����
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
//����
void connect(string &ip) {
    struct sockaddr_in addr;
    u_socket = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4444);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (!connect(u_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        cout << "\033[34m ������" << ip << "���ӳɹ���\033[0m \n";
        cout <<"������״̬�£�����\\d���ɶϿ����ӡ�\n";
        send(u_socket, username.c_str(), username.size(), 0);
        is_connected = true;

        thread recv_thread(receive, u_socket);
        recv_thread.detach();
    } 
    else cout << "\033[31m �޷����ӷ����� " << ip << " ! \033[0m \n";
}

int main(){
    //����wsa�洢socket����
    WSADATA wsa;
    string input;
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA����ʧ�ܣ� \033[0m \n";
        return 0;
    }
    
    cout<< "[��ӭʹ��Socket�����������--�û��� by 2211290Ҧ֪��] \n";
    cout<< "��������û���: ";
    getline(cin, username); 
    cout<<"��ӭ"<<username<<"!\n";
    cout << "�����б�(��δ����״̬��):\n"<< "\\c : ���������IP��ַ������\n"<< "\\r : �����û���\n"<< "\\q : �˳�����\n";
    while (true) {
        getline(cin, input);
        if (!is_connected) {
            if (input=="\\c") {
                cout << "�����������IP��ַ(Ĭ��Ϊ127.0.0.1����������ģ���ֱ�ӻس�): ";
                getline(cin, input);
                if(input=="") input="127.0.0.1";
                connect(input);
            } 
            else if (input == "\\r") {
                cout << "�������µ��û���: ";
                getline(cin, username); 
                cout<<"��ӭ"<<username<<"!\n";
            }
            else if (input == "\\q") {
                std::cout << "������5s���˳�����л���ʹ�ã�\n";
                Sleep(5000);
                break;
            }
            else cout << "�����б�(��δ����״̬��):\n"<< "\\c : ���������IP��ַ������\n"<< "\\r : �����û���\n"<< "\\q : �˳�����\n";
        } 
        else {
            if (input == "\\d") {
                string message= "\\d";
                send(u_socket, message.c_str(), message.size(), 0);
                is_connected = false;
                cout << "���˳����죡\n";
                cout << "�����б�(��δ����״̬��):\n"<< "\\c : ���������IP��ַ������\n"<< "\\r : �����û���\n"<< "\\q : �˳�����\n";
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
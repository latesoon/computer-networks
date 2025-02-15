#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define BUF_SIZE 4000
#define NAME_SIZE 50

vector<SOCKET> client_sockets;
mutex clients_mutex;//������
int num_client;
//����
void add_client(SOCKET client_socket) {
    lock_guard<mutex> lock(clients_mutex);
    client_sockets.push_back(client_socket);
}
//ɾ��
void remove_client(SOCKET client_socket) {
    lock_guard<mutex> lock(clients_mutex);
    client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
}
//�㲥��Ϣ
void send_message_to_all(const string &message, SOCKET sender_socket) {
    lock_guard<mutex> lock(clients_mutex);
    char time_buf[80];
    time_t now = time(nullptr);
    strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    string msg=string(time_buf) + " " + message;
    cout<<msg;
    for (auto &socket : client_sockets) {
        send(socket, msg.c_str(), msg.size(), 0);
    }
}

void op_client(SOCKET client_socket) {
    char buffer[BUF_SIZE];
    char username[NAME_SIZE];

    recv(client_socket, username, NAME_SIZE, 0);
    num_client++;
    string join_message=string(username) + "���������죬��ǰ����"+to_string(num_client)+"�����졣\n";
    send_message_to_all(join_message, client_socket);

    while (true) {
        int receive=recv(client_socket, buffer, BUF_SIZE, 0);
        if (receive> 0) {
            buffer[receive]='\0';
            string message = string(buffer);
            if (message == "\\d") {
                num_client--;
                string exit_message=string(username)+ "�˳������죬��ǰ����"+to_string(num_client)+"�����졣\n";
                send_message_to_all(exit_message, client_socket);
                remove_client(client_socket);
                closesocket(client_socket);
                break;
            }
            else{
                send_message_to_all(message, client_socket);
            }
        } 
        else {
            num_client--;
            string exit_message=string(username)+ "�Ͽ������ӣ���ǰ����"+to_string(num_client)+"�����졣\n";
            send_message_to_all(exit_message, client_socket);
            remove_client(client_socket);
            closesocket(client_socket);
            break;
        }
    }
}

int main(){
    cout<< "[Socket�����������--�������� by 2211290Ҧ֪��] \n";
    //����wsa�洢socket����
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA����ʧ�ܣ� \033[0m\n";
        return 0;
    }
    //����socket
    SOCKET ser_socket;
    struct sockaddr_in addr,u_addr;
    ser_socket = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4444);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(ser_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(!listen(ser_socket, 10))
        cout<<"\033[34m �����������ɹ��� \033[0m \n";
    else{
        cout<<"\033[31m ����������ʧ�ܡ� \033[0m \n";   
    }
    //����client
    while(true){
        int u_len = sizeof(u_addr);
        SOCKET u_socket = accept(ser_socket, (struct sockaddr*)&u_addr, &u_len);
        add_client(u_socket);
        thread client_thread(op_client, u_socket);
        client_thread.detach();
    }
    closesocket(ser_socket);
    WSACleanup();
    return 0;
}
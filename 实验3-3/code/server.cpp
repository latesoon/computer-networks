//run code:
//g++ code/server.cpp code/message.cpp -o exe_and_testcase/server/server -lws2_32
#include <iostream>
#include <winsock2.h>
#include "message.h"
#include <time.h>
#include <fstream>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
const u_short ser_port=2222;
const u_short r_port=3333;
message smsg,rmsg;//存储发送的报文和接收的报文

bool server_init(WSADATA &wsa,SOCKET &ser_socket,sockaddr_in &addr,sockaddr_in &r_addr){
    cout<< "[UDP可靠传输协议part3--服务器端 by 2211290姚知言] \n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA创建失败！ \033[0m\n";
        return false;
    }
    ser_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);//使用UDP协议

    u_long mode=1;
    if (ioctlsocket(ser_socket, FIONBIO, &mode)){
        cout<<"\033[31m 设置套接字非阻塞失败。 \033[0m \n";
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ser_port);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

	r_addr.sin_family = AF_INET; 
    r_addr.sin_port = htons(r_port);
	r_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if(bind(ser_socket, (struct sockaddr *)&addr, sizeof(addr))!= SOCKET_ERROR){
        cout<<"\033[34m 服务器启动成功！ \033[0m \n";
        return true;
    }
    else{
        cout<<"\033[31m 服务器启动失败。 \033[0m \n";
        return false;
    }
}
bool server_shake(SOCKET ser_socket,sockaddr_in r_addr){
    memset(&smsg, 0, sizeof(smsg));
    srand(time(NULL));
    int r_addrsize=sizeof(r_addr);
    //第一次握手
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 2) && rmsg.verifychecksum() && rmsg.ack == 0){
                cout<<"\033[34m 第一次握手接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
    }
    //第二次握手
    smsg.srcport = ser_port;
	smsg.dstport = r_port;
    smsg.seq = rand() % 500;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 3;//SYN ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第二次握手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第二次握手发送失败。 \033[0m \n";
        return false;
    }

    //第三次握手（包括第二次握手重传逻辑）
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 第三次握手接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
        if(clock()-lastsend >= maxwait){
            if(sendcnt++ >= maxsend){
                cout<<"\033[31m 超时重传已达到最大次数，握手失败。\033[0m \n";
                return false;
            }
            else{
                smsg.printdetails(true);
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"接收超时，第二次握手重传成功。\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m 接收超时，且第一次握手重传失败。\033[0m \n";
                    return false;
                }
            }
        }
    }
    cout<<"\033[34m 连接成功！\033[0m \n";
    return true;
}

bool server_recvfile(SOCKET ser_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    char filename[50]={'\0'};
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 0) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 文件信息报文接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
    }

    u_long nowlen=rmsg.len;
    u_long size=0;
    while(rmsg.data[--nowlen]!='\0'){
        size*=10;
        size+=(rmsg.data[nowlen]-48);
    }
    for(int i=0;i<=nowlen;i++)
        filename[i]=rmsg.data[i];
    smsg.seq +=1;
    //后续smsg.seq 因为在传输阶段长度一直为0，不再更新
    smsg.ack = rmsg.seq+rmsg.len;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 文件信息接收报文发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 文件信息接收报文发送失败。\033[0m \n";
        return false;
    }
    BYTE* buffer = new BYTE[size];
    u_long ptr=0;
    auto lastsend = clock();
    int sendcnt=1;
    u_long totseq=smsg.ack+size,nowseq=smsg.ack;
    while(true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 0) && rmsg.verifychecksum() && rmsg.ack == smsg.seq && rmsg.seq==nowseq){//想要的就是ta
                cout<<"\033[34m 收到了新的文件报文，发送确认报文！\033[0m \n";
                smsg.ack = rmsg.seq+rmsg.len;
                smsg.prepare();
                nowseq=smsg.ack;
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m 确认报文发送成功！\033[0m \n";
                else{
                    cout<<"\033[31m 确认报文发送失败。\033[0m \n";
                    return false;
                }
                //store
                for(int i=0;i<rmsg.len;i++)
                    buffer[ptr++]=rmsg.data[i];
                if(rmsg.seq+rmsg.len==totseq)break;//最后一个ack
                sendcnt=1;//重置发送次数
            }
            else{//想要的不是ta
                cout<<"接收到了信息，但未能通过验证，发送重传报文。\n";
                smsg.printdetails(true);
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m 重传报文发送成功！\033[0m \n";
                else{
                    cout<<"\033[31m 重传报文发送失败。\033[0m \n";
                    return false;
                }
            }
            lastsend=clock();//无论是哪一种，刷新时钟
        }
        if(clock()-lastsend >= maxwait){
            smsg.printdetails(true);
            if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                cout<<"接收超时，重传报文发送成功。\n";
                lastsend=clock();
            }
            else{
                cout<<"\033[31m 接收超时，重传报文发送失败。\033[0m \n";
                return false;
            }
        }
    }
    cout<<"\033[34m 文件已成功接收完毕，开始写入！\033[0m \n";
    
    ofstream file(filename, ios::binary);

    if (!file.is_open()) {
        cout<<"\033[31m 无法创建文件"<<filename<<"。\033[0m \n";
        return false;
    }
    // 将 buffer 的内容写入文件
    file.write(reinterpret_cast<char*>(buffer), size);
    // 关闭文件
    file.close();
    cout<<"\033[34m 文件已成功写入完毕！\033[0m \n";
    return true;
}

bool server_wave(SOCKET ser_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    //第一次挥手
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 5) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+smsg.len){
                cout<<"\033[34m 第一次挥手接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
    }
    //第二次挥手
    memset(&smsg.data,0,sizeof(smsg.data));//清空data
    smsg.seq = rmsg.ack;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第二次挥手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第二次挥手发送失败。 \033[0m \n";
        return false;
    }

    //第三次挥手
    smsg.flags = 5;//FIN ACK
    smsg.prepare();
    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第三次挥手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第三次挥手发送失败。 \033[0m \n";
        return false;
    }

    //第四次挥手（包括第三次挥手重传逻辑）
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 第四次挥手接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
        if(clock()-lastsend >= maxwait){
            if(sendcnt++ >= maxsend){
                cout<<"\033[31m 超时重传已达到最大次数，挥手失败。\033[0m \n";
                return false;
            }
            else{
                smsg.printdetails(true);
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"接收超时，第三次挥手重传成功。\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m 接收超时，且第三次挥手重传失败。\033[0m \n";
                    return false;
                }
            }
        }
    }
    return true;
}

int main(){
    //创建wsa存储socket数据
    WSADATA wsa;
    //创建socket
    SOCKET ser_socket;
    struct sockaddr_in addr,r_addr;
    if(!server_init(wsa,ser_socket,addr,r_addr) || !server_shake(ser_socket,r_addr) || !server_recvfile(ser_socket,r_addr) || !server_wave(ser_socket,r_addr)){
        cout<<"请输入任意字符，以退出程序。\n";
        char end;
        cin>>end;
        return 0;
    }
    closesocket(ser_socket);
	WSACleanup();
    cout<<"程序执行完成，请输入任意字符以退出程序。\n";
    char end;
    cin>>end;
    return 0;
}
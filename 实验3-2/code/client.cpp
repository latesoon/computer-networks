//run code:
//g++ code/client.cpp code/message.cpp -o exe_and_testcase/client/client -lws2_32
#include <iostream>
#include <winsock2.h>
#include "message.h"
#include <time.h>
#include <fstream>
#include <cstring>
#include <windows.h>
#include <deque>
#include <thread>
#include <future>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
const u_short u_port=4444;
const u_short r_port=3333;
message smsg,rmsg;//存储发送的报文和接收的报文

//窗口大小
#define windowsize 30
//连续收到多少个相同报文才会重发
#define CNTERR 3

int winend=-1,winstart=0;

bool client_init(WSADATA &wsa,SOCKET &u_socket,sockaddr_in &u_addr,sockaddr_in &r_addr){
    cout<< "[UDP可靠传输协议part2--用户端 by 2211290姚知言] \n";
    cout<< "使用滑动窗口机制，窗口大小为："<<windowsize<<'\n';
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA创建失败！ \033[0m \n";
        return false;
    }
    
    u_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);//使用UDP协议

    if (u_socket == INVALID_SOCKET) {
        cout << "\033[31m 套接字创建失败，错误码: " << WSAGetLastError() << " \033[0m \n";
        return false;
    }

    u_long mode=1;
    if (ioctlsocket(u_socket, FIONBIO, &mode)){
        cout << "\033[31m 设置套接字非阻塞失败，错误码: " << WSAGetLastError() << " \033[0m \n";
        return false;
    }

    //客户端
    u_addr.sin_family = AF_INET;
    u_addr.sin_port = htons(u_port);
    u_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //路由器端
    r_addr.sin_family = AF_INET;
    r_addr.sin_port = htons(r_port);
    r_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if(::bind(u_socket, (struct sockaddr *)&u_addr, sizeof(u_addr))== SOCKET_ERROR){
        cout<<"\033[31m 客户端初始化失败。 \033[0m \n";
        return false;
    }
    return true;
}

bool client_shake(SOCKET u_socket,sockaddr_in r_addr){
    //第一次握手
    memset(&smsg, 0, sizeof(smsg));
    srand(time(NULL));
    smsg.srcport = u_port;
	smsg.dstport = r_port;
    smsg.seq = rand() % 500;
    smsg.flags = 2;//SYN
    smsg.prepare();
    int r_addrsize=sizeof(r_addr);
    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第一次握手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第一次握手发送失败。 \033[0m \n";
        return false;
    }

    //第二次握手（包括第一次握手重传逻辑）
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 3) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 第二次握手接收成功！\033[0m \n";
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
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"接收超时，第一次握手重传成功。\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m 接收超时，且第一次握手重传失败。\033[0m \n";
                    return false;
                }
            }
        }
    }

    //第三次握手
    memset(&smsg, 0, sizeof(smsg));
    smsg.srcport = u_port;
	smsg.dstport = r_port;
    smsg.seq = rmsg.ack;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;
    smsg.prepare();

    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, sizeof(r_addr))>0)
        cout<<"\033[34m 第三次握手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第三次握手发送失败。\033[0m \n";
        return false;
    }
    cout<<"\033[34m 连接成功！\033[0m \n";
    return true;
}

bool useresend=false,finfile=false;

bool client_resend(SOCKET u_socket,sockaddr_in r_addr,message* msgbuf,u_long batch){
    auto lastsend = clock();
    int r_addrsize=sizeof(r_addr);
    int lastack = 0, cntlast = 1;
    while (true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack >= msgbuf[winstart].seq + msgbuf[winstart].len){
                lastack = rmsg.ack;
                while(true){
                    if(winstart > batch){
                        finfile = true;
                        return true;
                    }
                    else if(rmsg.ack >= msgbuf[winstart].seq + msgbuf[winstart].len) winstart++;
                    else break;
                }
                cntlast = 1;
                lastsend=clock();//会发送新的报文
            }
            else if ((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == lastack){
                cntlast++;
                if(cntlast == CNTERR){
                    cout<<"收到错误报文达到次数，重传已发送报文。\n";
                    useresend=true;
                    cntlast = 0;
                    lastsend=clock();    
                }
            }
            else{
                //一般是不会到这里的，说明有先发的报文后收到，或者错误报文接收，忽略即可
                cout<<"接收到了一个错误或过期的报文。\n";
            }
        }
        if(clock()-lastsend >= maxwait){
            cout<<"接收超时，重传已发送报文。\n";
            useresend=true;
            cntlast = 0;
            lastsend=clock();
        }
    }
}

bool client_sendfile(SOCKET u_socket,sockaddr_in r_addr){
    int mode;
    string file;
    cout << "可靠数据传输测试开始:\n";
    cout << "请输入一个数字，表示测试文件:\n1:1.jpg\n2:2.jpg\n3:3.jpg\n4:helloworld.txt\n5:自定义输入文件\n";
    cin >> mode;
	switch(mode){
        case 1:file="1.jpg";break;
        case 2:file="2.jpg";break;
        case 3:file="3.jpg";break;
        case 4:file="helloworld.txt";break;
        default:cout<<"请输入文件名:";cin>>file;break;
    }
    cout<<"开始对文件"<<file<<"的传输测试。\n";
    auto timestart = clock();
    ifstream textfile(file.c_str(), ifstream::binary);
    if(!textfile){//如果文件不能打开
        cout<<"\033[31m 文件读取失败。\033[0m \n";
        return false;
    }
    
    u_long size=0;
    BYTE* buffer = new BYTE[15000000];
    BYTE byte = textfile.get();
	while (textfile) {
		buffer[size++] = byte;
		byte = textfile.get();
	}
    textfile.close();

    int nowbatch=0,totbatch=(size-1)/maxlen+1,lastbatch=size%maxlen;
    if(!lastbatch)lastbatch=maxlen;

    message* msgbuf = new message[totbatch+1];


    //port seq ack无需更新
    smsg.flags=0;
    int nowlen=0;
    for(;nowlen<file.size();nowlen++)
        smsg.data[nowlen]=file[nowlen];
    smsg.data[nowlen++]='\0';
    u_long tempsize=size;
    while(tempsize){
        smsg.data[nowlen++]=tempsize%10+48;
        tempsize/=10;
    }
    smsg.len=nowlen;
    int r_addrsize=sizeof(r_addr);

    smsg.storechecksum();
    msgbuf[nowbatch] = smsg;

    while(true){
        nowbatch++;
        smsg.seq+=smsg.len;
        smsg.len=((nowbatch==totbatch)?lastbatch:maxlen);
        memset(&smsg.data,0,sizeof(smsg.data));
        for(int i=0;i<smsg.len;i++)
            smsg.data[i]=buffer[i+(nowbatch-1)*maxlen];
        smsg.storechecksum();
        msgbuf[nowbatch] = smsg;
        if(nowbatch==totbatch) break;
    }
    
    //建立接收thread
    future<bool> resend = async(launch::async, client_resend, u_socket, r_addr, msgbuf, totbatch);

    while(true){
        if(winend - winstart < windowsize - 1 && winend < totbatch && !useresend && !finfile){
            smsg=msgbuf[++winend];
            smsg.printdetails(true);
            cout<<"当前空闲窗口大小："<<windowsize -(winend - winstart + 1)<<'\n';
            if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                cout<<"\033[34m 新的报文发送成功！\033[0m \n";
            else{
                cout<<"\033[31m 新的报文发送失败。\033[0m \n";
                return false;
            }
        }
        if(useresend && !finfile){
            useresend = false;
            for(int i=winstart;i<=winend;i++){
                smsg=msgbuf[i];
                smsg.printdetails(true);
                cout<<"当前空闲窗口大小："<<windowsize -(winend - winstart+ 1)<<'\n';
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m 重传报文发送成功！\033[0m \n";
                else{
                    cout<<"\033[31m 重传报文发送失败。\033[0m \n";
                    return false;
                }
            }
        }
        if(finfile) break;
    }
    
    //在此处等待线程结束，并读返回值
    if (!resend.get()) return false;


    cout<<"\033[34m 文件已成功发送完毕！\033[0m \n";
    auto timeconnect=clock()-timestart;
    cout<<"传输时间:"<<double(timeconnect)/CLOCKS_PER_SEC<<"s\n";
    cout<<"吞吐量:"<<double(size)/double(timeconnect)*CLOCKS_PER_SEC<<"Byte/s\n";
    return true;
}

bool client_wave(SOCKET u_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    //第一次挥手
    memset(&smsg.data,0,sizeof(smsg.data));//清空data
    //port seq ack不更新
    smsg.flags = 5;//FIN,ACK
    smsg.len = 0;
    smsg.prepare();
    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第一次挥手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第一次挥手发送失败。 \033[0m \n";
        return false;
    }
    
    //第二次挥手（包括第一次挥手重传逻辑）
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 第二次挥手接收成功！\033[0m \n";
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
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"接收超时，第一次挥手重传成功。\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m 接收超时，且第一次挥手重传失败。\033[0m \n";
                    return false;
                }
            }
        }
    }

    //第三次挥手
    while (true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 5) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m 第三次挥手接收成功！\033[0m \n";
                break;
            }
            else
                cout<<"接收到了信息，但未能通过验证。\n";
        }
    }

    //第四次挥手
    smsg.seq +=1;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m 第四次挥手发送成功！\033[0m \n";
    else{
        cout<<"\033[31m 第四次挥手发送失败。 \033[0m \n";
        return false;
    }
    Sleep(maxwait);
    return true;
}

int main(){
    //创建wsa存储socket数据
    WSADATA wsa;
    //创建socket
    SOCKET u_socket;
    struct sockaddr_in u_addr,r_addr;
    if(!client_init(wsa,u_socket,u_addr,r_addr) || !client_shake(u_socket,r_addr)|| !client_sendfile(u_socket,r_addr) || !client_wave(u_socket,r_addr)){
        cout<<"请输入任意字符，以退出程序。\n";
        char end;
        cin>>end;
        return 0;
    }
    closesocket(u_socket);
	WSACleanup();
    cout<<"程序执行完成，请输入任意字符以退出程序。\n";
    char end;
    cin>>end;
    return 0;
}
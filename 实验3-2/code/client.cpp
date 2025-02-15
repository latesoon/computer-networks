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
message smsg,rmsg;//�洢���͵ı��ĺͽ��յı���

//���ڴ�С
#define windowsize 30
//�����յ����ٸ���ͬ���ĲŻ��ط�
#define CNTERR 3

int winend=-1,winstart=0;

bool client_init(WSADATA &wsa,SOCKET &u_socket,sockaddr_in &u_addr,sockaddr_in &r_addr){
    cout<< "[UDP�ɿ�����Э��part2--�û��� by 2211290Ҧ֪��] \n";
    cout<< "ʹ�û������ڻ��ƣ����ڴ�СΪ��"<<windowsize<<'\n';
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA����ʧ�ܣ� \033[0m \n";
        return false;
    }
    
    u_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);//ʹ��UDPЭ��

    if (u_socket == INVALID_SOCKET) {
        cout << "\033[31m �׽��ִ���ʧ�ܣ�������: " << WSAGetLastError() << " \033[0m \n";
        return false;
    }

    u_long mode=1;
    if (ioctlsocket(u_socket, FIONBIO, &mode)){
        cout << "\033[31m �����׽��ַ�����ʧ�ܣ�������: " << WSAGetLastError() << " \033[0m \n";
        return false;
    }

    //�ͻ���
    u_addr.sin_family = AF_INET;
    u_addr.sin_port = htons(u_port);
    u_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //·������
    r_addr.sin_family = AF_INET;
    r_addr.sin_port = htons(r_port);
    r_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if(::bind(u_socket, (struct sockaddr *)&u_addr, sizeof(u_addr))== SOCKET_ERROR){
        cout<<"\033[31m �ͻ��˳�ʼ��ʧ�ܡ� \033[0m \n";
        return false;
    }
    return true;
}

bool client_shake(SOCKET u_socket,sockaddr_in r_addr){
    //��һ������
    memset(&smsg, 0, sizeof(smsg));
    srand(time(NULL));
    smsg.srcport = u_port;
	smsg.dstport = r_port;
    smsg.seq = rand() % 500;
    smsg.flags = 2;//SYN
    smsg.prepare();
    int r_addrsize=sizeof(r_addr);
    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m ��һ�����ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m ��һ�����ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }

    //�ڶ������֣�������һ�������ش��߼���
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 3) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m �ڶ������ֽ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
        }
        if(clock()-lastsend >= maxwait){
            if(sendcnt++ >= maxsend){
                cout<<"\033[31m ��ʱ�ش��Ѵﵽ������������ʧ�ܡ�\033[0m \n";
                return false;
            }
            else{
                smsg.printdetails(true);
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"���ճ�ʱ����һ�������ش��ɹ���\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m ���ճ�ʱ���ҵ�һ�������ش�ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
        }
    }

    //����������
    memset(&smsg, 0, sizeof(smsg));
    smsg.srcport = u_port;
	smsg.dstport = r_port;
    smsg.seq = rmsg.ack;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;
    smsg.prepare();

    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, sizeof(r_addr))>0)
        cout<<"\033[34m ���������ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m ���������ַ���ʧ�ܡ�\033[0m \n";
        return false;
    }
    cout<<"\033[34m ���ӳɹ���\033[0m \n";
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
                lastsend=clock();//�ᷢ���µı���
            }
            else if ((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == lastack){
                cntlast++;
                if(cntlast == CNTERR){
                    cout<<"�յ������Ĵﵽ�������ش��ѷ��ͱ��ġ�\n";
                    useresend=true;
                    cntlast = 0;
                    lastsend=clock();    
                }
            }
            else{
                //һ���ǲ��ᵽ����ģ�˵�����ȷ��ı��ĺ��յ������ߴ����Ľ��գ����Լ���
                cout<<"���յ���һ���������ڵı��ġ�\n";
            }
        }
        if(clock()-lastsend >= maxwait){
            cout<<"���ճ�ʱ���ش��ѷ��ͱ��ġ�\n";
            useresend=true;
            cntlast = 0;
            lastsend=clock();
        }
    }
}

bool client_sendfile(SOCKET u_socket,sockaddr_in r_addr){
    int mode;
    string file;
    cout << "�ɿ����ݴ�����Կ�ʼ:\n";
    cout << "������һ�����֣���ʾ�����ļ�:\n1:1.jpg\n2:2.jpg\n3:3.jpg\n4:helloworld.txt\n5:�Զ��������ļ�\n";
    cin >> mode;
	switch(mode){
        case 1:file="1.jpg";break;
        case 2:file="2.jpg";break;
        case 3:file="3.jpg";break;
        case 4:file="helloworld.txt";break;
        default:cout<<"�������ļ���:";cin>>file;break;
    }
    cout<<"��ʼ���ļ�"<<file<<"�Ĵ�����ԡ�\n";
    auto timestart = clock();
    ifstream textfile(file.c_str(), ifstream::binary);
    if(!textfile){//����ļ����ܴ�
        cout<<"\033[31m �ļ���ȡʧ�ܡ�\033[0m \n";
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


    //port seq ack�������
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
    
    //��������thread
    future<bool> resend = async(launch::async, client_resend, u_socket, r_addr, msgbuf, totbatch);

    while(true){
        if(winend - winstart < windowsize - 1 && winend < totbatch && !useresend && !finfile){
            smsg=msgbuf[++winend];
            smsg.printdetails(true);
            cout<<"��ǰ���д��ڴ�С��"<<windowsize -(winend - winstart + 1)<<'\n';
            if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                cout<<"\033[34m �µı��ķ��ͳɹ���\033[0m \n";
            else{
                cout<<"\033[31m �µı��ķ���ʧ�ܡ�\033[0m \n";
                return false;
            }
        }
        if(useresend && !finfile){
            useresend = false;
            for(int i=winstart;i<=winend;i++){
                smsg=msgbuf[i];
                smsg.printdetails(true);
                cout<<"��ǰ���д��ڴ�С��"<<windowsize -(winend - winstart+ 1)<<'\n';
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m �ش����ķ��ͳɹ���\033[0m \n";
                else{
                    cout<<"\033[31m �ش����ķ���ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
        }
        if(finfile) break;
    }
    
    //�ڴ˴��ȴ��߳̽�������������ֵ
    if (!resend.get()) return false;


    cout<<"\033[34m �ļ��ѳɹ�������ϣ�\033[0m \n";
    auto timeconnect=clock()-timestart;
    cout<<"����ʱ��:"<<double(timeconnect)/CLOCKS_PER_SEC<<"s\n";
    cout<<"������:"<<double(size)/double(timeconnect)*CLOCKS_PER_SEC<<"Byte/s\n";
    return true;
}

bool client_wave(SOCKET u_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    //��һ�λ���
    memset(&smsg.data,0,sizeof(smsg.data));//���data
    //port seq ack������
    smsg.flags = 5;//FIN,ACK
    smsg.len = 0;
    smsg.prepare();
    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m ��һ�λ��ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m ��һ�λ��ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }
    
    //�ڶ��λ��֣�������һ�λ����ش��߼���
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m �ڶ��λ��ֽ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
        }
        if(clock()-lastsend >= maxwait){
            if(sendcnt++ >= maxsend){
                cout<<"\033[31m ��ʱ�ش��Ѵﵽ������������ʧ�ܡ�\033[0m \n";
                return false;
            }
            else{
                smsg.printdetails(true);
                if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"���ճ�ʱ����һ�λ����ش��ɹ���\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m ���ճ�ʱ���ҵ�һ�λ����ش�ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
        }
    }

    //�����λ���
    while (true){
        if(recvfrom(u_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 5) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m �����λ��ֽ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
        }
    }

    //���Ĵλ���
    smsg.seq +=1;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(u_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m ���Ĵλ��ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m ���Ĵλ��ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }
    Sleep(maxwait);
    return true;
}

int main(){
    //����wsa�洢socket����
    WSADATA wsa;
    //����socket
    SOCKET u_socket;
    struct sockaddr_in u_addr,r_addr;
    if(!client_init(wsa,u_socket,u_addr,r_addr) || !client_shake(u_socket,r_addr)|| !client_sendfile(u_socket,r_addr) || !client_wave(u_socket,r_addr)){
        cout<<"�����������ַ������˳�����\n";
        char end;
        cin>>end;
        return 0;
    }
    closesocket(u_socket);
	WSACleanup();
    cout<<"����ִ����ɣ������������ַ����˳�����\n";
    char end;
    cin>>end;
    return 0;
}
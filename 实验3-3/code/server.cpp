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
message smsg,rmsg;//�洢���͵ı��ĺͽ��յı���

bool server_init(WSADATA &wsa,SOCKET &ser_socket,sockaddr_in &addr,sockaddr_in &r_addr){
    cout<< "[UDP�ɿ�����Э��part3--�������� by 2211290Ҧ֪��] \n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        cout << "\033[31m WSA����ʧ�ܣ� \033[0m\n";
        return false;
    }
    ser_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);//ʹ��UDPЭ��

    u_long mode=1;
    if (ioctlsocket(ser_socket, FIONBIO, &mode)){
        cout<<"\033[31m �����׽��ַ�����ʧ�ܡ� \033[0m \n";
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ser_port);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

	r_addr.sin_family = AF_INET; 
    r_addr.sin_port = htons(r_port);
	r_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if(bind(ser_socket, (struct sockaddr *)&addr, sizeof(addr))!= SOCKET_ERROR){
        cout<<"\033[34m �����������ɹ��� \033[0m \n";
        return true;
    }
    else{
        cout<<"\033[31m ����������ʧ�ܡ� \033[0m \n";
        return false;
    }
}
bool server_shake(SOCKET ser_socket,sockaddr_in r_addr){
    memset(&smsg, 0, sizeof(smsg));
    srand(time(NULL));
    int r_addrsize=sizeof(r_addr);
    //��һ������
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 2) && rmsg.verifychecksum() && rmsg.ack == 0){
                cout<<"\033[34m ��һ�����ֽ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
        }
    }
    //�ڶ�������
    smsg.srcport = ser_port;
	smsg.dstport = r_port;
    smsg.seq = rand() % 500;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 3;//SYN ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m �ڶ������ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m �ڶ������ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }

    //���������֣������ڶ��������ش��߼���
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m ���������ֽ��ճɹ���\033[0m \n";
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
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"���ճ�ʱ���ڶ��������ش��ɹ���\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m ���ճ�ʱ���ҵ�һ�������ش�ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
        }
    }
    cout<<"\033[34m ���ӳɹ���\033[0m \n";
    return true;
}

bool server_recvfile(SOCKET ser_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    char filename[50]={'\0'};
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 0) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m �ļ���Ϣ���Ľ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
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
    //����smsg.seq ��Ϊ�ڴ���׶γ���һֱΪ0�����ٸ���
    smsg.ack = rmsg.seq+rmsg.len;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m �ļ���Ϣ���ձ��ķ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m �ļ���Ϣ���ձ��ķ���ʧ�ܡ�\033[0m \n";
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
            if((rmsg.flags == 0) && rmsg.verifychecksum() && rmsg.ack == smsg.seq && rmsg.seq==nowseq){//��Ҫ�ľ���ta
                cout<<"\033[34m �յ����µ��ļ����ģ�����ȷ�ϱ��ģ�\033[0m \n";
                smsg.ack = rmsg.seq+rmsg.len;
                smsg.prepare();
                nowseq=smsg.ack;
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m ȷ�ϱ��ķ��ͳɹ���\033[0m \n";
                else{
                    cout<<"\033[31m ȷ�ϱ��ķ���ʧ�ܡ�\033[0m \n";
                    return false;
                }
                //store
                for(int i=0;i<rmsg.len;i++)
                    buffer[ptr++]=rmsg.data[i];
                if(rmsg.seq+rmsg.len==totseq)break;//���һ��ack
                sendcnt=1;//���÷��ʹ���
            }
            else{//��Ҫ�Ĳ���ta
                cout<<"���յ�����Ϣ����δ��ͨ����֤�������ش����ġ�\n";
                smsg.printdetails(true);
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
                    cout<<"\033[34m �ش����ķ��ͳɹ���\033[0m \n";
                else{
                    cout<<"\033[31m �ش����ķ���ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
            lastsend=clock();//��������һ�֣�ˢ��ʱ��
        }
        if(clock()-lastsend >= maxwait){
            smsg.printdetails(true);
            if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                cout<<"���ճ�ʱ���ش����ķ��ͳɹ���\n";
                lastsend=clock();
            }
            else{
                cout<<"\033[31m ���ճ�ʱ���ش����ķ���ʧ�ܡ�\033[0m \n";
                return false;
            }
        }
    }
    cout<<"\033[34m �ļ��ѳɹ�������ϣ���ʼд�룡\033[0m \n";
    
    ofstream file(filename, ios::binary);

    if (!file.is_open()) {
        cout<<"\033[31m �޷������ļ�"<<filename<<"��\033[0m \n";
        return false;
    }
    // �� buffer ������д���ļ�
    file.write(reinterpret_cast<char*>(buffer), size);
    // �ر��ļ�
    file.close();
    cout<<"\033[34m �ļ��ѳɹ�д����ϣ�\033[0m \n";
    return true;
}

bool server_wave(SOCKET ser_socket,sockaddr_in r_addr){
    int r_addrsize=sizeof(r_addr);
    //��һ�λ���
    while (true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 5) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+smsg.len){
                cout<<"\033[34m ��һ�λ��ֽ��ճɹ���\033[0m \n";
                break;
            }
            else
                cout<<"���յ�����Ϣ����δ��ͨ����֤��\n";
        }
    }
    //�ڶ��λ���
    memset(&smsg.data,0,sizeof(smsg.data));//���data
    smsg.seq = rmsg.ack;
    smsg.ack = rmsg.seq+1;
    smsg.flags = 1;//ACK
    smsg.prepare();

    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m �ڶ��λ��ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m �ڶ��λ��ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }

    //�����λ���
    smsg.flags = 5;//FIN ACK
    smsg.prepare();
    if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0)
        cout<<"\033[34m �����λ��ַ��ͳɹ���\033[0m \n";
    else{
        cout<<"\033[31m �����λ��ַ���ʧ�ܡ� \033[0m \n";
        return false;
    }

    //���Ĵλ��֣����������λ����ش��߼���
    auto lastsend = clock();
    int sendcnt=1;
    while(true){
        if(recvfrom(ser_socket, (char*)&rmsg, sizeof(rmsg), 0, (sockaddr*)&r_addr, &r_addrsize)>0){
            rmsg.printdetails(false);
            if((rmsg.flags == 1) && rmsg.verifychecksum() && rmsg.ack == smsg.seq+1){
                cout<<"\033[34m ���Ĵλ��ֽ��ճɹ���\033[0m \n";
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
                if(sendto(ser_socket, (char*)&smsg, sizeof(smsg), 0, (sockaddr*)&r_addr, r_addrsize)>0){
                    cout<<"���ճ�ʱ�������λ����ش��ɹ���\n";
                    lastsend=clock();
                }
                else{
                    cout<<"\033[31m ���ճ�ʱ���ҵ����λ����ش�ʧ�ܡ�\033[0m \n";
                    return false;
                }
            }
        }
    }
    return true;
}

int main(){
    //����wsa�洢socket����
    WSADATA wsa;
    //����socket
    SOCKET ser_socket;
    struct sockaddr_in addr,r_addr;
    if(!server_init(wsa,ser_socket,addr,r_addr) || !server_shake(ser_socket,r_addr) || !server_recvfile(ser_socket,r_addr) || !server_wave(ser_socket,r_addr)){
        cout<<"�����������ַ������˳�����\n";
        char end;
        cin>>end;
        return 0;
    }
    closesocket(ser_socket);
	WSACleanup();
    cout<<"����ִ����ɣ������������ַ����˳�����\n";
    char end;
    cin>>end;
    return 0;
}
#include <iostream>
#include "message.h"
using namespace std;

message::message(){
    srcport=0;
    dstport=0;
    seq=0;
    ack=0;
    flags=0;
    len=0;
    checksum=0;
    memset(&data, 0, sizeof(data));//clear all
}
u_short message::calchecksum(){
    u_long nowlen=len;//�洢����
    u_long nowsum=0;
    BYTE* ptr = data; 
    while(nowlen--){
        nowsum+=*(ptr++);
        if (nowsum & 0xffff0000) {
			nowsum &= 0xffff;
			nowsum++;
		}
    }
    return ~(nowsum & 0xffff);
}
void message::storechecksum(){
    checksum=calchecksum();
}
bool message::verifychecksum(){
    return (checksum==calchecksum());
}
void message::printdetails(){
    cout<<"������ϸ��Ϣ����:\n";
    cout<<"Դ�˿�:"<<srcport<<" Ŀ�Ķ˿�:"<<dstport<<'\n';
    cout<<"SEQ:"<<seq<<" ACK:"<<ack<<'\n';
    cout<<"FLAGS: [0]ACK:"<<((flags&0x1)?1:0)<<" [1]SYN:"<<((flags&0x2)?1:0)<<" [2]FIN:"<<((flags&0x4)?1:0)<<'\n';
    cout<<"���ĳ���:"<<len<<" У���:"<<checksum<<'\n';
}
void message::prepare(){
    storechecksum();
    printdetails();
}
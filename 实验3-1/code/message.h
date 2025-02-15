#include <iostream>
#include <windows.h>
using namespace std;

const unsigned int maxlen=14000;//不超过15000字节
const int maxwait=2000;//最大等待时间，2s
const int maxsend=10;//最多发送次数，10
#pragma pack(1)
class message{
    public:
    u_short srcport;//源端口
    u_short dstport;//目的端口
    u_long seq;
    u_long ack;
    u_short flags;//[0]:ACK,[1]:SYN,[2]:FIN
    u_long len;//报文长度
    u_short checksum;//校验和
    BYTE data[maxlen];
    message();
    u_short calchecksum();
    void storechecksum();
    void prepare();
    bool verifychecksum();
    void printdetails();
};
#pragma pack()
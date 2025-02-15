#include <iostream>
#include <windows.h>
using namespace std;

const unsigned int maxlen=14000;//������15000�ֽ�
const int maxwait=2000;//���ȴ�ʱ�䣬2s
const int maxsend=10;//��෢�ʹ�����10
#pragma pack(1)
class message{
    public:
    u_short srcport;//Դ�˿�
    u_short dstport;//Ŀ�Ķ˿�
    u_long seq;
    u_long ack;
    u_short flags;//[0]:ACK,[1]:SYN,[2]:FIN
    u_long len;//���ĳ���
    u_short checksum;//У���
    BYTE data[maxlen];
    message();
    u_short calchecksum();
    void storechecksum();
    void prepare();
    bool verifychecksum();
    void printdetails();
};
#pragma pack()
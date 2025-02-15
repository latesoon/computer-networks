#include<iostream>
using namespace std;

#define THRESHINIT 20//��ʼ������ֵ

#define WINDOWSIZE 30//���ڴ�С

//�����յ����ٸ���ͬ���ĲŻ��ط�
#define CNTERR 3

class renomanager{
public:
    int cwnd;//ӵ�����ڴ�С
    u_short mode;//0:������ 1:ӵ������ 2:���ٻָ�
    int ssthresh;//����������ֵ
    int ackcnt;//ӵ������׶����ڼ����Ƿ��������ڶ���ȷ����
    renomanager(){//��ʼ��
        cwnd=1;
        mode=0;
        ssthresh=THRESHINIT;
        ackcnt=0;
    }
    void printreno(int send){
        cout<<"��ǰ״̬��"<<((mode)?((mode==2)?"���ٻָ�":"ӵ������"):("������"))<<'\n';
        cout<<"��ǰӵ�����ڴ�С��"<<cwnd<<" �������ѷ��͵ı���������"<<send<<'\n';
    }
    void newack(){
        switch(mode){
            case 0: 
                if(++cwnd>=ssthresh){
                    ackcnt = 0;
                    mode = 1;
                }
                break;
            case 1:
                if(++ackcnt>=cwnd){
                    ackcnt = 0;
                    cwnd++;
                }
                break;
            case 2:
                cwnd = ssthresh;
			    mode = 1;
			    ackcnt = 0;
                break;
        }
    }
    void dupack(int dup){
        switch(mode){
            case 2:
                cwnd ++;
                break;
            default:
                if(dup >= CNTERR){
                    ssthresh = cwnd >> 1;
                    cwnd = ssthresh + CNTERR;
                    mode = 2;
                }
                break;
        }
    }
    void timeout(){
        ssthresh = cwnd >> 1;
        cwnd = 1;
        mode = 0;
    }
};
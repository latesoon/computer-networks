#include<iostream>
using namespace std;

#define THRESHINIT 20//初始化门限值

#define WINDOWSIZE 30//窗口大小

//连续收到多少个相同报文才会重发
#define CNTERR 3

class renomanager{
public:
    int cwnd;//拥塞窗口大小
    u_short mode;//0:慢启动 1:拥塞避免 2:快速恢复
    int ssthresh;//慢启动门限值
    int ackcnt;//拥塞避免阶段用于计算是否整个窗口都正确接收
    renomanager(){//初始化
        cwnd=1;
        mode=0;
        ssthresh=THRESHINIT;
        ackcnt=0;
    }
    void printreno(int send){
        cout<<"当前状态："<<((mode)?((mode==2)?"快速恢复":"拥塞避免"):("慢启动"))<<'\n';
        cout<<"当前拥塞窗口大小："<<cwnd<<" 窗口中已发送的报文数量："<<send<<'\n';
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
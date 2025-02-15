import socket
import time
import random

# 地址设置
SERVER_ADDRESS = ('127.0.0.1', 2222) 
ROUTER_ADDRESS = ('127.0.0.1', 3333)
CLIENT_ADDRESS = ('127.0.0.1', 4444) #直接给出了CLIENT地址，简化实现

DROP_RATE = 0  # 丢包率
DELAY = 0.1  # 时延(s)

def should_drop(): # 这里我使用random函数，比原有router更贴近现实场景
    return random.random() < DROP_RATE

def router():
    # 创建 UDP 套接字
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(ROUTER_ADDRESS)
        print(f"Router启动成功，地址： {ROUTER_ADDRESS}\nRouter的丢包率为{DROP_RATE}，传输时延为{DELAY}。")
        
        while True:
            # 接收数据
            data, addr = sock.recvfrom(100000)
            
            # 判断数据包来源
            if addr == SERVER_ADDRESS:
                # 来自 server 的包
                print(f"收到了来自server的包，大小 {len(data)} 字节。")
                sock.sendto(data, CLIENT_ADDRESS)
                
            elif addr == CLIENT_ADDRESS:
                # 来自 client 的包
                print(f"收到了来自client的包，大小 {len(data)} 字节。")
                
                if should_drop():
                    print("丢包。")
                    continue  # 丢包
                
                print("延迟发送。")
                time.sleep(DELAY)
                sock.sendto(data, SERVER_ADDRESS)
                continue

if __name__ == "__main__":
    try:
        router()
    except KeyboardInterrupt:
        print("\nRouter stopped.")
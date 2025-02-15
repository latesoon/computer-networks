"""
# group1
import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号
# 数据
loss_rate = [0, 2, 4, 6, 8, 10]  # 丢包率（百分比）

# 停等协议数据
stop_and_wait_time = [1.121, 7.117, 9.134, 15.118, 23.161, 25.122]  # 传输时间(s)
stop_and_wait_throughput = [1657, 261.0, 203.3, 122.9, 80.19, 73.93]  # 吞吐量(KB/s)

# 滑动窗口协议数据
sliding_window_time = [0.641, 0.729, 0.752, 0.856, 0.948, 1.265]  # 传输时间(s)
sliding_window_throughput = [2898, 2547, 2470, 2170, 1959, 1468]  # 吞吐量(KB/s)

# 绘制传输时间折线图
plt.figure(figsize=(12, 6))

plt.subplot(1, 2, 1)  # 第1个子图：传输时间
plt.plot(loss_rate, stop_and_wait_time, 'o--', label='停等机制', color='red')
plt.plot(loss_rate, sliding_window_time, 's-', label='滑动窗口机制', color='blue')
plt.title('不同丢包率下的传输时间')
plt.xlabel('丢包率 (%)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 绘制吞吐量折线图
plt.subplot(1, 2, 2)  # 第2个子图：吞吐量
plt.plot(loss_rate, stop_and_wait_throughput, 'o--', label='停等机制', color='red')
plt.plot(loss_rate, sliding_window_throughput, 's-', label='滑动窗口机制', color='blue')
plt.title('不同丢包率下的吞吐量')
plt.xlabel('丢包率 (%)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group1.png',dpi=500)


import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号

# 数据
delay = [0, 20, 40, 60, 80, 100]  # 传输延时 (ms)

# 停等协议数据
stop_and_wait_time = [1.121, 4.286, 8.430, 10.611, 11.474, 14.014]  # 传输时间 (s)
stop_and_wait_throughput = [1657, 433.4, 220.3, 175.0, 161.8, 132.5]  # 吞吐量 (KB/s)

# 滑动窗口协议数据
sliding_window_time = [0.641, 18.304, 21.234, 25.545, 31.368, 34.461]  # 传输时间 (s)
sliding_window_throughput = [2898, 101.5, 87.47, 72.70, 59.21, 53.89]  # 吞吐量 (KB/s)

# 绘制传输时间折线图
plt.figure(figsize=(12, 6))

plt.subplot(1, 2, 1)  # 第1个子图：传输时间
plt.plot(delay, stop_and_wait_time, 'o--', label='停等机制', color='red')
plt.plot(delay, sliding_window_time, 's-', label='滑动窗口机制', color='blue')
plt.title('不同延时下的传输时间')
plt.xlabel('传输延时 (ms)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 绘制吞吐量折线图
plt.subplot(1, 2, 2)  # 第2个子图：吞吐量
plt.plot(delay, stop_and_wait_throughput, 'o--', label='停等机制', color='red')
plt.plot(delay, sliding_window_throughput, 's-', label='滑动窗口机制', color='blue')
plt.title('不同延时下的吞吐量')
plt.xlabel('传输延时 (ms)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group2.png',dpi=500)


import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号
# 数据
loss_rate = [0, 2, 4, 6, 8, 10]  # 丢包率（%）

# 滑动窗口大小 = 5
size_5_time = [0.620, 0.684, 0.740, 0.800, 0.850, 1.010]
size_5_throughput = [2996, 2715, 2510, 2322, 2185, 1839]

# 滑动窗口大小 = 10
size_10_time = [0.610, 0.670, 0.760, 0.846, 0.850, 0.970]
size_10_throughput = [3045, 2772, 2444, 2195, 2185, 1915]

# 滑动窗口大小 = 20
size_20_time = [0.620, 0.680, 0.720, 0.760, 0.970, 1.001]
size_20_throughput = [2996, 2731, 2580, 2444, 1915, 1856]

# 滑动窗口大小 = 30
size_30_time = [0.641, 0.729, 0.752, 0.856, 0.948, 1.265]
size_30_throughput = [2898, 2547, 2470, 2170, 1959, 1468]

# 绘制传输时间折线图
plt.figure(figsize=(12, 6))

# 子图1：传输时间
plt.subplot(1, 2, 1)
plt.plot(loss_rate, size_5_time, 'o--', label='size=5', color='red')
plt.plot(loss_rate, size_10_time, 's--', label='size=10', color='blue')
plt.plot(loss_rate, size_20_time, 'd--', label='size=20', color='green')
plt.plot(loss_rate, size_30_time, 'x--', label='size=30', color='orange')

plt.title('不同滑动窗口大小的传输时间')
plt.xlabel('丢包率 (%)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 子图2：吞吐量
plt.subplot(1, 2, 2)
plt.plot(loss_rate, size_5_throughput, 'o-', label='size=5', color='red')
plt.plot(loss_rate, size_10_throughput, 's-', label='size=10', color='blue')
plt.plot(loss_rate, size_20_throughput, 'd-', label='size=20', color='green')
plt.plot(loss_rate, size_30_throughput, 'x-', label='size=30', color='orange')

plt.title('不同滑动窗口大小的吞吐量')
plt.xlabel('丢包率 (%)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group3.png',dpi=500)

import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号

# 数据
delay = [0, 20, 40, 60, 80, 100]  # 传输延时（ms）

# 停等机制
stop_and_wait_time = [1.121, 4.286, 8.430, 10.611, 11.474, 14.014]
stop_and_wait_throughput = [1657, 433.4, 220.3, 175.0, 161.8, 132.5]

# 滑动窗口大小 = 5
size_5_time = [0.620, 3.431, 6.072, 8.723, 11.157, 14.559]
size_5_throughput = [2996, 541.3, 305.9, 212.9, 166.5, 127.6]

# 滑动窗口大小 = 10
size_10_time = [0.610, 9.183, 12.263, 18.673, 23.806, 28.786]
size_10_throughput = [3045, 202.2, 151.4, 99.47, 78.02, 64.52]

# 滑动窗口大小 = 20
size_20_time = [0.620, 9.652, 13.383, 22.417, 31.014, 33.567]
size_20_throughput = [2996, 192.4, 138.8, 82.85, 59.89, 55.33]

# 滑动窗口大小 = 30
size_30_time = [0.641, 18.304, 21.234, 25.545, 31.368, 34.461]
size_30_throughput = [2898, 101.5, 87.47, 72.70, 59.21, 53.89]

# 绘制传输时间折线图
plt.figure(figsize=(12, 6))

# 子图1：传输时间
plt.subplot(1, 2, 1)
plt.plot(delay, stop_and_wait_time, 'o--', label='停等机制', color='gray')  # 停等
plt.plot(delay, size_5_time, 'o-', label='size=5', color='red')
plt.plot(delay, size_10_time, 's-', label='size=10', color='blue')
plt.plot(delay, size_20_time, 'd-', label='size=20', color='green')
plt.plot(delay, size_30_time, 'x-', label='size=30', color='orange')

plt.title('不同滑动窗口大小的传输时间对比')
plt.xlabel('传输延时 (ms)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 子图2：吞吐量
plt.subplot(1, 2, 2)
plt.plot(delay, stop_and_wait_throughput, 'o--', label='停等机制', color='gray')  # 停等
plt.plot(delay, size_5_throughput, 'o-', label='size=5', color='red')
plt.plot(delay, size_10_throughput, 's-', label='size=10', color='blue')
plt.plot(delay, size_20_throughput, 'd-', label='size=20', color='green')
plt.plot(delay, size_30_throughput, 'x-', label='size=30', color='orange')

plt.title('不同滑动窗口大小的吞吐量对比')
plt.xlabel('传输延时 (ms)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group4.png',dpi=500)

import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号

# 数据
loss_rate = [0, 2, 4, 6, 8, 10]  # 丢包率 (%)

# 有拥塞控制
congestion_time = [0.708, 0.745, 0.817, 0.889, 2.930, 9.034]
congestion_throughput = [2623, 2493, 2273, 2089, 633.9, 205.6]

# 无拥塞控制
no_congestion_time = [0.641, 0.729, 0.752, 0.856, 0.948, 1.265]
no_congestion_throughput = [2898, 2547, 2470, 2170, 1959, 1468]

# 绘制折线图
plt.figure(figsize=(12, 6))

# 子图1：传输时间对比
plt.subplot(1, 2, 1)
plt.plot(loss_rate, congestion_time, 'o-', label='有拥塞控制', color='red')
plt.plot(loss_rate, no_congestion_time, 's-', label='无拥塞控制', color='blue')

plt.title('不同丢包率下的传输时间对比')
plt.xlabel('丢包率 (%)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 子图2：吞吐量对比
plt.subplot(1, 2, 2)
plt.plot(loss_rate, congestion_throughput, 'o-', label='有拥塞控制', color='red')
plt.plot(loss_rate, no_congestion_throughput, 's-', label='无拥塞控制', color='blue')

plt.title('不同丢包率下的吞吐量对比')
plt.xlabel('丢包率 (%)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group5.png',dpi=500)
"""

import matplotlib.pyplot as plt
from matplotlib import rcParams
# 配置字体，确保中文显示
rcParams['font.sans-serif'] = ['SimHei']  # 使用黑体
rcParams['axes.unicode_minus'] = False   # 正常显示负号
# 数据
latency = [0, 20, 40, 60, 80, 100]  # 传输延时 (ms)

# 有拥塞控制
congestion_time = [0.708, 10.407, 17.931, 19.955, 22.971, 25.996]
congestion_throughput = [2623, 178.4, 103.5, 93.08, 80.86, 71.45]

# 无拥塞控制
no_congestion_time = [0.641, 18.304, 21.234, 25.545, 31.368, 34.461]
no_congestion_throughput = [2898, 101.5, 87.47, 72.70, 59.21, 53.89]

# 绘制折线图
plt.figure(figsize=(12, 6))

# 子图1：传输时间对比
plt.subplot(1, 2, 1)
plt.plot(latency, congestion_time, 'o-', label='有拥塞控制', color='red')
plt.plot(latency, no_congestion_time, 's-', label='无拥塞控制', color='blue')

plt.title('不同延时下的传输时间对比')
plt.xlabel('传输延时 (ms)')
plt.ylabel('传输时间 (s)')
plt.legend()
plt.grid()

# 子图2：吞吐量对比
plt.subplot(1, 2, 2)
plt.plot(latency, congestion_throughput, 'o-', label='有拥塞控制', color='red')
plt.plot(latency, no_congestion_throughput, 's-', label='无拥塞控制', color='blue')

plt.title('不同延时下的吞吐量对比')
plt.xlabel('传输延时 (ms)')
plt.ylabel('吞吐量 (KB/s)')
plt.legend()
plt.grid()

# 显示图形
plt.tight_layout()
plt.savefig('group6.png',dpi=500)

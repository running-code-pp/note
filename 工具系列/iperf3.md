<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-14 02:35:12
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-14 02:38:40
 * @FilePath: \note\工具系列\iperf3.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 从客户端发起压测（TCP）
在你的本地机器或另一台客户端机器上执行：

bash
编辑
iperf3 -c aliyun -p 5000 -t 30 -P 4 --logfile tcp_test.txt
参数说明：
参数	说明
-c	客户端模式，连接指定服务器
<aliyun-public-ip>	阿里云 ECS 的公网 IP
-p 5000	指定端口
-t 30	测试时长 30 秒
-P 4	并发 4 个并行流（模拟多连接）
-i 5	（可选）每 5 秒输出一次实时带宽
--logfile filename	（可选）保存结果到文件
示例完整命令：
bash
编辑
iperf3 -c 47.115.33.164 -p 5000 -t 60 -P 4 -i 5 --logfile iperf3_tcp_5000.log
✅ 示例输出（客户端）
text
编辑
Connecting to host 47.98.100.200, port 5000
[  4] local 192.168.1.100 port 54321 connected to 47.98.100.200 port 5000
[ ID] Interval           Transfer     Bitrate         Retr
[  4]   0.00-10.00  sec  1.25 GBytes  1.07 Gbits/sec    0             sender
[  4]   0.00-10.00  sec  1.25 GBytes  1.07 Gbits/sec                  receiver
表示：10 秒内传输了 1.25GB，平均速率 1.07 Gbps。

常见问题排查
问题	原因	解决方法
Connection refused	服务端未运行或端口错误	检查 iperf3 -s -p 5000 是否运行
Connection timed out	安全组未放行或防火墙拦截	检查阿里云安全组和系统防火墙（如 firewalld/iptables）
速度上不去	带宽受限、实例规格低、网络拥塞	检查 ECS 实例带宽限制（如 100M、5M 公网带宽）
只能跑几 Mbps	公网带宽瓶颈	建议使用内网压测，或升级带宽
进阶建议
使用内网 IP 在 VPC 内测试，排除公网波动影响。
测试 UDP：加 -u 参数（iperf3 -c x.x.x.x -p 5000 -u -b 1G）
多客户端并发测试，模拟真实负载。
使用 nuttcp 或 qperf 作为替代工具对比。
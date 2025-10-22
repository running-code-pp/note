<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-22 11:30:15
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-22 11:32:01
 * @FilePath: \note\中间件\etcd.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 简介
etcd是golang开发的一个kv存储系统，通常用来做rpc的服务注册和发现

# 基本命令
etcdctl put key value --endpoints=ip:port

etcdctl get key --endpoints=ip:port
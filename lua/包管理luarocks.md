# Luarocks 常用命令

## 安装和卸载包

### 安装包
```bash
# 从官方仓库安装
luarocks install package_name

# 安装指定版本
luarocks install package_name 1.0.0

# 从本地文件安装
luarocks install ./package_name-1.0.0-1.rockspec

# 从git仓库安装
luarocks install package_name --from=https://github.com/user/repo/raw/master/package_name-scm-1.rockspec
```

### 卸载包
```bash
# 卸载包
luarocks remove package_name

# 强制卸载（忽略依赖）
luarocks remove --force package_name
```

## 查询和搜索

### 搜索包
```bash
# 搜索包
luarocks search package_name

# 显示包详细信息
luarocks show package_name
```

### 列出已安装的包
```bash
# 列出所有已安装的包
luarocks list

# 列出指定包的详细信息
luarocks list package_name
```

## 包管理

### 更新包
```bash
# 更新所有包
luarocks install --update-all

# 更新指定包
luarocks install package_name --update
```

### 清理缓存
```bash
# 清理下载缓存
luarocks purge
```

## 开发相关

### 创建rockspec文件
```bash
# 生成rockspec模板
luarocks new_version package_name-1.0.0-1.rockspec
```

### 打包
```bash
# 从rockspec打包
luarocks pack package_name-1.0.0-1.rockspec
```

### 测试安装
```bash
# 在本地测试安装（不影响全局环境）
luarocks make --local
```

## 配置管理

### 查看配置
```bash
# 显示当前配置
luarocks config

# 显示指定配置项
luarocks config rocks_trees
```

### 设置配置
```bash
# 设置本地安装路径
luarocks config local_tree /path/to/local/tree

# 添加自定义仓库
luarocks config rocks_servers http://my-server.com/rocks
```

## 环境管理

### 查看Lua版本和路径
```bash
# 显示Lua版本
luarocks --version

# 显示Lua路径
luarocks path

# 显示Lua C路径
luarocks path --lr-cpath
```

### 使用本地环境
```bash
# 在当前目录创建本地环境
luarocks init

# 使用本地tree
luarocks --local install package_name
```

## 高级用法

### 自定义构建
```bash
# 使用cmake构建
luarocks install package_name CMAKE_BUILD_TYPE=Release

# 自定义编译器
luarocks install package_name CC=gcc CXX=g++
```

### 依赖管理
```bash
# 显示包依赖树
luarocks show --deps package_name

# 检查依赖完整性
luarocks check package_name
```

### 调试
```bash
# 详细输出
luarocks install package_name --verbose

# 调试模式
luarocks install package_name --debug
```

## 常用选项

- `--local`: 在用户本地目录安装（~/.luarocks）
- `--tree`: 指定安装目录
- `--server`: 指定服务器
- `--only-server`: 只从指定服务器安装
- `--force`: 强制操作
- `--deps-mode`: 依赖处理模式（all/one/none）
- `--lua-version`: 指定Lua版本
- `--lua-dir`: 指定Lua安装目录

## 示例工作流

```bash
# 1. 搜索并安装包
luarocks search luasocket
luarocks install luasocket

# 2. 查看安装结果
luarocks list luasocket

# 3. 在Lua中使用
lua -e "require 'socket'"

# 4. 更新包
luarocks install luasocket --update

# 5. 清理
luarocks remove luasocket
```

# Luarocks 网络问题排查和解决

## 常见网络错误

### 错误：Failed downloading manifest - host or service not provided, or not known

**原因：** 网络连接或DNS解析问题

**解决方案：**

#### 1. 检查网络连接
```bash
# 测试网络连接
ping luarocks.org
# 如果出现 "Name or service not known" 说明DNS解析失败

# 测试DNS解析
nslookup luarocks.org

# 测试基础网络连通性
ping 8.8.8.8
```

#### 2. 配置DNS服务器（重要！）
```bash
# 备份原配置
sudo cp /etc/resolv.conf /etc/resolv.conf.backup

# 编辑resolv.conf
sudo vi /etc/resolv.conf

# 添加可靠的DNS服务器（清空原内容，添加以下内容）
nameserver 8.8.8.8
nameserver 1.1.1.1
nameserver 223.5.5.5
nameserver 114.114.114.114

# 保存后测试DNS解析
nslookup luarocks.org

# 如果resolv.conf被覆盖，设置为不可变
sudo chattr +i /etc/resolv.conf
```

#### 3. 使用国内镜像源（推荐解决方案）
```bash
# 方法1：直接指定服务器安装
luarocks install --server=https://luarocks.cn/ luasocket

# 方法2：永久配置国内镜像
# 查看当前配置文件位置
luarocks config --system-config

# 编辑配置文件，添加国内镜像
sudo vi /etc/luarocks/config-5.3.lua
# 或者
sudo vi ~/.luarocks/config-5.3.lua

# 在配置文件中添加：
rocks_servers = {
    "https://luarocks.cn/",
    "https://mirrors.tuna.tsinghua.edu.cn/luarocks/",
    "https://mirrors.ustc.edu.cn/luarocks/"
}

# 或者使用命令行配置
luarocks config rocks_servers "https://luarocks.cn/"
```

#### 4. 配置代理服务器
```bash
# 设置HTTP代理
export http_proxy=http://your-proxy:port
export https_proxy=http://your-proxy:port

# 或者直接在luarocks中设置
luarocks config http_proxy http://your-proxy:port
luarocks config https_proxy http://your-proxy:port
```

#### 5. 手动下载和安装
```bash
# 如果DNS问题无法解决，使用IP地址或其他源
# 从GitHub下载（如果可以访问GitHub）
wget https://raw.githubusercontent.com/lunarmodules/luasocket/master/luasocket-scm-0.rockspec

# 或者从国内镜像下载
wget https://luarocks.cn/manifests/luasocket/luasocket-3.0rc1-2.rockspec

# 本地安装
luarocks install luasocket-3.0rc1-2.rockspec

# 直接从git安装（如果git可用）
luarocks install https://github.com/lunarmodules/luasocket/archive/master.zip
```

### 其他网络相关错误

#### SSL证书问题
```bash
# 禁用SSL验证（不推荐，仅用于测试）
luarocks config verify_ssl false
```

#### 超时问题
```bash
# 设置更长的超时时间
luarocks config connection_timeout 30
luarocks config read_timeout 30
```

## 验证安装

```bash
# 检查luarocks配置
luarocks config

# 测试安装
luarocks install luasocket

# 验证LuaSocket是否可用
lua -e "require 'socket'; print('LuaSocket installed successfully')"
```

## 备用安装方法

### 方法1：使用系统包管理器
```bash
# Ubuntu/Debian
sudo apt-get install lua-socket

# CentOS/RHEL
sudo yum install lua-socket

# macOS
brew install lua
luarocks install luasocket
```

### 方法2：从源码编译
```bash
# 下载源码
wget http://files.luaforge.net/releases/luasocket/luasocket/luasocket-3.0-rc1.tar.gz
tar -xzf luasocket-3.0-rc1.tar.gz
cd luasocket-3.0-rc1

# 编译安装
make
sudo make install
```

### 方法3：使用系统包管理器（推荐，避免网络问题）
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install lua-socket-dev

# CentOS/RHEL (需要EPEL源)
sudo yum install epel-release
sudo yum install lua-socket

# 测试安装结果
lua -e "local socket = require('socket'); print('LuaSocket version:', socket._VERSION)"
```

### 方法4：修改hosts文件解决DNS问题
```bash
# 查询luarocks.org的真实IP（在有网络的机器上）
nslookup luarocks.org

# 编辑hosts文件
sudo vi /etc/hosts

# 添加IP映射（示例IP，请使用实际查询结果）
151.101.193.229 luarocks.org

# 保存后测试
ping luarocks.org
```

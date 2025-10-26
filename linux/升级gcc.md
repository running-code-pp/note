## 一、安装gcc 4.8.5

升级到最新内核后，由于删除旧版本内核，gcc因依赖关系被删除，需事先安装gcc的旧版本。

```bash
yum install -y gcc gcc-c++
```

## 二、下载gcc-12.2.0.tar.gz

官网地址为 https://gcc.gnu.org，选择download中的镜像网站，从/software/gcc/releases/gcc-11.2.0中下载源代码文件。

```bash
wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-12.2.0/gcc-12.2.0.tar.gz
tar -zxvf gcc-12.2.0.tar.gz
```

## 三、下载安装依赖项

```bash
cd gcc-12.2.0
./contrib/download_prerequisites
cd ..
```

## 四、编译gcc

建立 gcc-12.2.0-bulid 目录，在该目录中执行 gcc-12.2.0 目录中的 configure 命令，可在该目录中生成编译文件，不影响 gcc-12.2.0 源代码目录的文件结构。

```bash
mkdir gcc-12.2.0-build
cd gcc-12.2.0-build
../gcc-12.2.0/configure --prefix=/usr/local/gcc-12.2.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib
make -j2 # 2核服务器配置的参数
make install
cd ..
```
## 五、配置环境

### 设置环境变量
```bash
touch /etc/profile.d/gcc.sh
chmod 777 /etc/profile.d/gcc.sh 
echo -e '\nexport PATH=/usr/local/gcc-12.2.0/bin:$PATH\n' >> /etc/profile.d/gcc.sh
source /etc/profile.d/gcc.sh
```

### 设置头文件
```bash
ln -sv /usr/local/gcc-12.2.0/include/c++/12.2.0 /usr/include/c++/12.2.0
```

### 设置库文件
```bash
touch /etc/ld.so.conf.d/gcc.conf
chmod 777 /etc/ld.so.conf.d/gcc.conf 
echo -e "/usr/local/gcc-12.2.0/lib64" >> /etc/ld.so.conf.d/gcc.conf
# 加载动态连接库
ldconfig -v
ldconfig -p |grep gcc
```

### 重启服务器
```bash
reboot
```

## 六、查看gcc版本

```bash
gcc --version
g++ --version
c++ --version
```
升级成功将显示如下内容：

```
gcc (GCC) 11.2.0
Copyright © 2021 Free Software Foundation, Inc.
本程序是自由软件；请参看源代码的版权声明。本软件没有任何担保；
包括没有适销性和某一专用目的下的适用性担保。
```

```
g++ (GCC) 11.2.0
Copyright © 2021 Free Software Foundation, Inc.
本程序是自由软件；请参看源代码的版权声明。本软件没有任何担保；
包括没有适销性和某一专用目的下的适用性担保。
```

```
c++ (GCC) 11.2.0
Copyright © 2021 Free Software Foundation, Inc.
本程序是自由软件；请参看源代码的版权声明。本软件没有任何担保；
包括没有适销性和某一专用目的下的适用性担保。
```

## 七、检验某程序用何版本的gcc编译的

### 检查可执行文件
```bash
readelf -p .comment /usr/local/python39/bin/python3
```

输出示例：
```
String dump of section '.comment':
  [     0]  GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)
  [    2d]  GCC: (GNU) 11.2.0 
```

### 检查静态库文件
```bash
strings -a /usr/local/openssl-3.0.2/lib64/libssl.a | grep "GCC"
```

输出示例：
```
GCC: (GNU) 11.2.0
GCC: (GNU) 11.2.0
```

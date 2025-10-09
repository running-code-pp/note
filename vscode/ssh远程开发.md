配置免密

- **ssh-keygen 生成密钥** \
系统会在你指定的路径（本例子为 C:\Users\YourUsername\.ssh）下生成两个文件，分别是id_rsa_windows.pub和id_rsa_windows

- **添加公钥到服务器**\
touch ~/.ssh/authorized_keys\
vim ~/.ssh/authorized_keys\
把公钥内容复制进去\
sudo systemctl restart sshd

- 修复服务器ssh配置
```
RSAAuthentication yes
PubkeyAuthentication yes
```

- 配置host
```ssh-config
Host aliyun01
    HostName 47.115.33.164
    User root
```
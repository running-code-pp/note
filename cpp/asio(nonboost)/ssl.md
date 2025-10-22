<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-18 23:53:39
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-19 00:03:37
 * @FilePath: \note\cpp\asio(nonboost)\ssl.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# ssl/tls证书

- 证书持有者的身份信息(域名./组织名称)
- 证书持者的公钥
- 证书颁发机构的信息
- CA数字签名
- 证书的有效期
asio 中的证书通常以PEM格式存储，一种base64编码的文本

# 创建和管理证书
## 自签名证书
测试开发中，可用自签名证书，通过openssl在终端生成
```bash
# 生成私钥
openssl genrsa -out server_key.pem 2048

# 生成证书签名请求（CSR）
openssl req -new -key server_key.pem -out server_csr.pem

# 生成自签名证书（有效期365天）
openssl x509 -req -days 365 -in server_csr.pem -signkey server_key.pem -out server_cert.pem

```


签名证书解码:https://www.nethub.com.hk/hk/ssl-certificates/certificate-decoder/


## 证书颁发机构 CA
生产环境中应该使用第三方CA颁发的证书，创建自己的CA并使用它签署server 证书
``` bash
# 创建CA私钥
openssl genrsa -out ca_key.pem 2048

# 创建CA证书
openssl req -new -x509 -days 3650 -key ca_key.pem -out ca_cert.pem

# 生成服务器私钥
openssl genrsa -out server_key.pem 2048

# 生成服务器证书签名请求
openssl req -new -key server_key.pem -out server_csr.pem

# 使用CA签署服务器证书
openssl x509 -req -days 365 -in server_csr.pem -CA ca_cert.pem -CAkey ca_key.pem -CAcreateserial -out server_cert.pem

```


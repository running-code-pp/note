<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-09 01:19:12
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-11-20 10:41:22
 * @FilePath: \note\cpp\包管理器\conan2.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 安装
[官方文档](https://docs.conan.org.cn/2/index.html)\
 
# 初始化
```bash
设置默认的profile\
conan profile detect --force\

查看profile文件的位置\
conan profile path default


conan install . --output-folder=build --build=missing --settings=build_type=Debug

cmake -S . -B build --preset conan-default
cmake --build build -j 16
```


# 安装依赖
conan install 依赖包/版本

# 查找依赖
conan search "boost*" --remote=conancenter


# 查看远程仓库列表
conan remote list

# 移除远程仓库
conan remote remove bincrafters


# 下载某个包
以libcurl为例: conan download libcurl/8.10.0 --remote conancenter

# 查看包的配置项
配置项就是编译选项，如果要查看首先要把包下载下来，然后使用
下载完成的时候会输出一个Local Cache
```bash
Local Cache
  libcurl
    libcurl/8.10.0
      revisions
        d4877a334bcb923055b8445bad6dca50 (2025-04-24 15:33:57 UTC)
          packages
            3410b85f7abaed843044eb5e00366cdc47a1e9a9
              revisions
                79f980edb53938b2423e4aa050643544 (2025-04-24 16:09:36 UTC)
              info
                settings
                  arch: x86_64
                  build_type: Release
                  compiler: gcc
                  compiler.version: 11
                  os: Linux
                options
                  fPIC: True
                  shared: False
                  with_brotli: False
                  with_c_ares: False
                  with_ca_bundle: auto
                  with_ca_fallback: False
                  with_ca_path: auto
                  with_cookies: True
                  with_crypto_auth: True
                  with_dict: True
                  with_docs: False
                  with_file: True
                  with_form_api: True
                  with_ftp: True
                  with_gopher: True
                  with_http: True
                  with_imap: True
                  with_ipv6: True
                  with_largemaxwritesize: False
                  with_ldap: False
                  with_libgsasl: False
                  with_libidn: False
                  with_libpsl: False
                  with_librtmp: False
                  with_libssh2: False
                  with_misc_docs: False
                  with_mqtt: True
                  with_nghttp2: False
                  with_ntlm: True
                  with_ntlm_wb: True
                  with_pop3: True
                  with_proxy: True
                  with_rtsp: True
                  with_smb: True
                  with_smtp: True
                  with_ssl: openssl
                  with_symbol_hiding: False
                  with_telnet: True
                  with_tftp: True
                  with_threaded_resolver: True
                  with_unix_sockets: True
                  with_verbose_debug: True
                  with_verbose_strings: True
                  with_zlib: True
                  with_zstd: False
                requires
                  openssl/3.4.Z
                  zlib/1.3.Z

                     info
                settings
                  arch: x86_64
                  build_type: Release
                  compiler: msvc
                  compiler.runtime: dynamic
                  compiler.runtime_type: Release
                  compiler.version: 193
                  os: Windows
                options
                  shared: True
                  with_brotli: False
                  with_c_ares: False
                  with_ca_bundle: auto
                  with_ca_fallback: False
                  with_ca_path: auto
                  with_cookies: True
                  with_crypto_auth: True
                  with_dict: True
                  with_docs: False
                  with_file: True
                  with_form_api: True
                  with_ftp: True
                  with_gopher: True
                  with_http: True
                  with_imap: True
                  with_ipv6: True
                  with_largemaxwritesize: False
                  with_ldap: False
                  with_libidn: False
                  with_libpsl: False
                  with_librtmp: False
                  with_libssh2: False
                  with_misc_docs: False
                  with_mqtt: True
                  with_nghttp2: False
                  with_ntlm: True
                  with_ntlm_wb: True
                  with_pop3: True
                  with_proxy: True
                  with_rtsp: True
                  with_smb: True
                  with_smtp: True
                  with_ssl: openssl
                  with_symbol_hiding: False
                  with_telnet: True
                  with_tftp: True
                  with_threaded_resolver: True
                  with_unix_sockets: True
                  with_verbose_debug: True
                  with_verbose_strings: True
                  with_zlib: True
                  with_zstd: False
                requires
                  openssl/3.4.Z
                  zlib/1.3.Z
```



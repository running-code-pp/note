# git拉取的时候报错 fatal: could not get a repository handle
最终排查发现是路径太长的问题，默认的最长路径是256个字符，但是如果项目中的目录层级递归深度太大就可能造成超过的情况

## 1. 启用长路径（管理员权限）
Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1

## 2. 配置 Git
git config --global core.longpaths true

## 3. 重新拉取
git submodule update --init --recursive
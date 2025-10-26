安装更新程序
https://wslstorestorage.blob.core.windows.net/wslblob/wsl_update_x64.msi


管理员在powershell中运行，安装完成重启电脑
```bash
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
```

打开powershell将wls2设置为默认版本
```bash
wls --set-default-version 2
```


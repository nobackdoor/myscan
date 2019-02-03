# myscan
多线程扫描ip段端口程序

命令行下:  myscan -p Port1[,Port2,Port3...] [-t Thread](default 10) [-d](DEBUG) StartIp EndIp

Example:

myscan -p 80 192.168.1.1 192.168.1.254

myscan -p 21,22,23,80,443,8080 -t 256 192.168.1.1 192.168.1.254

请确保 StartIp 和 EndIp 为最后两项

Please make sure that StartIp&&EndIp are the last options.

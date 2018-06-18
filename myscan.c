#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#include <windows.h>
#define MAX_THREADS 10

struct ScanData
{
    unsigned int ip;
    int port;
};
void usage();
void SocketInit();
unsigned int getip(char *);
char *ipback(unsigned int);
void scan(unsigned int, unsigned int, int);
DWORD WINAPI threadscan(LPVOID);
void usage()
{
    printf("Usage:\n"
           "program StartIp EndIp Port\n"
           "Example:myscan 192.168.1.1 192.168.254 80\n");
}
void SocketInit()
{
    WSADATA wd;
    int ret = 0;
    ret = WSAStartup(MAKEWORD(2, 2), &wd); /*1.初始化操作*/
    if (ret != 0)
    {
        printf("Init Fail");
        exit(1);
    }
    if (HIBYTE(wd.wVersion) != 2 || LOBYTE(wd.wVersion) != 2)
    {
        printf("Init Fail");
        WSACleanup();
        exit(1);
    }
}
char *ipback(unsigned int ip)
{
    char *ipstr = (char *)malloc(17 * sizeof(char));
    unsigned int ip_temp_numbr = 24, ip_int_index[4];

    for (int j = 0; j < 4; j++)
    {
        ip_int_index[j] = (ip >> ip_temp_numbr) & 0xFF;
        ip_temp_numbr -= 8;
    }
    sprintf(ipstr, "%d.%d.%d.%d", ip_int_index[0], ip_int_index[1], ip_int_index[2], ip_int_index[3]);
    return ipstr;
}
unsigned int getip(char *ip)
{
    char myip[20] = "";
    strcpy(myip, ip);
    char str_ip_index[4] = "";
    int k = 3, j = 0;
    unsigned int ip_add = 0;
    for (int i = 0; i <= strlen(myip); i++)
    {
        if (myip[i] == '.' || myip[i] == '\0')
        {
            unsigned int ip_int = atoi(str_ip_index);
            if (ip_int < 0 || ip_int > 255)
            {
                //printf("%d\n%s\n", ip_int,str_ip_index);
                printf("!!!IP ERROR!!!\n");
                exit(1);
            }
            ip_add += (ip_int * (unsigned int)(pow(256.0, k)));
            k--;
            for (int x = 0; x < 4; x++)
                str_ip_index[x] = '\0';
            j = 0;
            continue;
        }
        str_ip_index[j] = myip[i];
        j++;
    }
    return ip_add;
}
void scan(unsigned int StartIp, unsigned int EndIp, int port)
{
    if (StartIp > EndIp)
    {
        usage();
        exit(1);
    }
    for (unsigned int i = StartIp; i <= EndIp;)
    {
        DWORD dwThreadId[MAX_THREADS];
        HANDLE hThread[MAX_THREADS];
        unsigned int last = EndIp - i;
        if (last >= MAX_THREADS)
        {
            last = MAX_THREADS;
        }
        else if (last == 0)
            last = 1;
        struct ScanData *pData[last];
        for (unsigned int j = 0; j < last; j++)
        {
            pData[j] = (struct ScanData *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct ScanData *));
            pData[j]->ip = i;
            pData[j]->port = port;
            i++;
            hThread[j] = CreateThread(NULL,
                                      0,
                                      threadscan,
                                      pData[j],
                                      0,
                                      &dwThreadId[j]);
            if (hThread[j] == NULL)
            {
                printf("Create Thread ERROR\n");
                ExitProcess(j);
            }
        }
        WaitForMultipleObjects(last, hThread, 1, INFINITE);
        for (int j = 0; j < last; j++)
        {
            CloseHandle(hThread[j]);
        }
    }
}
DWORD WINAPI threadscan(LPVOID lpParam)
{
    //printf("\nthread start\n");
    struct ScanData *pa = (struct ScanData *)lpParam;
    char ip[20] = "";
    sprintf(ip, "%u", pa->ip);
    
    SOCKET c;
    SOCKADDR_IN saddr;
    
    /*2.创建客户端socket*/
    c = socket(AF_INET, SOCK_STREAM, 0);
    /*3.定义要连接的服务端信息*/
    saddr.sin_addr.S_un.S_addr = inet_addr(ip);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(pa->port);
    /*4.连接服务端*/
    if (connect(c, (SOCKADDR *)&saddr, sizeof(SOCKADDR)) != -1)
    {
        printf("%-16s %d Open\n", ipback(pa->ip), pa->port);
    }
    closesocket(c);
    //WSACleanup();
    return 0;
}
int main(int argc, char **argv)
{
    if (argc!=4)  //ProgramName StartIp EndIp Port
    {
        usage();
        return 1;
    }
    SocketInit();
    scan(getip(argv[1]), getip(argv[2]), atoi(argv[3]));
    //printf("%u %d %d", getip(argv[1]), getip(argv[2]), atoi(argv[3]));
    WSACleanup();
    return 0;
}
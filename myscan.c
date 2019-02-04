#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#include <pthread.h>
#include <Windows.h>
#include <unistd.h>
#define DEFAULT_THREADS 10              //默认线程数
unsigned int portlist[65536];           //端口池,portlist[0]为需要测试的端口数
unsigned int Threads = DEFAULT_THREADS; //线程数
int debug = 0;
struct ScanData //分配给线程的数据
{
    unsigned int ip;
    int port;
};
const struct ScanData NILDATA; //初始化用的空数据
pthread_attr_t t_c;            //线程属性
void usage();
void Init();
unsigned int getip(char *);
char *ipback(unsigned int);
void scan(unsigned int, unsigned int, unsigned int);
void *threadscan(void *);
void usage()
{
    printf("Usage:\n"
           "program -p Port1[,Port2,Port3...] [-t Thread](default 10) [-d](DEBUG) StartIp EndIp\n"
           "Example:myscan -p 80 192.168.1.1 192.168.1.254\n"
           "        myscan -p 21,22,23,80,443,8080 -t 256 192.168.1.1 192.168.1.254\n");
}
void Init()
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

    pthread_attr_init(&t_c);                                    //初始化线程属性
    pthread_attr_setdetachstate(&t_c, PTHREAD_CREATE_DETACHED); //设置线程属性
}
char *ipback(unsigned int ip) //把10进制无符号整形ip转换为点IP
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
unsigned int getip(char *ip) //把IP地址转换为10进制无符号整形
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

                printf("!!!IP ERROR!!!   Not regular IP\n");
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
void scan(unsigned int StartIp, unsigned int EndIp, unsigned int Thread)
{
    if (StartIp > EndIp)
    {
        usage();
        printf("!!!ERROR!!!  Your StartIp is bigger than your EndIp.\n");
        exit(1);
    }
    int port_p = 1; //用于循环端口
    for (int i = StartIp; i <= EndIp;) //i为当前的ip（无符号整型式）
    {
        int last = EndIp - i; //last为每组的数量
        if (last >= Thread)
        {
            last = Thread;
        }
        else if (last == 0)
            last = 1;
        struct ScanData pData[last];
        for (int k = 0; k < last; k++)
            pData[k] = NILDATA; //初始化扫描数据为空
        pthread_t t[last];
        for (int j = 0; j < last; j++)
        {
            pData[j].ip = i;
            pData[j].port = portlist[port_p];
            port_p++;
            if (port_p > portlist[0])
            {
                port_p = 1;
                i++;
            }
            if (pthread_create(&t[j], &t_c, threadscan, (void *)&pData[j]) != 0)//创建线程
                printf("\nCREATE THREAD ERROR\n");
            else
                pthread_join(t[j], NULL);
            Sleep(10);
        }
        Sleep(1000);
    }
    Sleep(3000);
}
void *threadscan(void *sd)
{
    struct ScanData *pa = (struct ScanData *)sd;
    char ip[20] = "";
    sprintf(ip, "%u", pa->ip);
    if (debug)
        printf("Testing %-16s %d ...\n", ipback(pa->ip), pa->port);
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
    pthread_exit(NULL);
    return NULL;
}
int main(int argc, char **argv)
{

    Init();
    char opt;
    if (argc < 4) //ProgramName -pPort StartIp EndIp
    {
        usage();
        WSACleanup();
        return 1;
    }
    while ((opt = getopt(argc, argv, "p:t:d")) != -1)
    {
        switch (opt)
        {
        case 'p': //port
        {
            portlist[0] = 1;
            char *p = NULL;
            char *optarg_copy = strdup(optarg); //copy
            for (p = optarg_copy; *p != '\0'; p++)
            {
                if (*p == ',' && *(p + 1) != ',' && *(p + 1) != '\0') //计算输入的端口数
                    portlist[0]++;
                else if (*p < '0' || *p > '9') //检查合法
                {
                    printf("!!!PORT ERROR!!!  Check your port set.\n");
                    free(optarg_copy);
                    exit(1);
                }
            }

            int i = 1;
            if (portlist[0] != 1) //输入的端口不唯一
            {
                if ((p = strtok(optarg_copy, ",")) != NULL)  //分割字符串，分离端口
                {
                    int temp = atoi(p);
                    if (temp <= 0 || temp > 65535)
                    {
                        printf("!!!PORT ERROR!!!  Check your port set.\n");
                        free(optarg_copy);
                        exit(1);
                    }
                    portlist[i++] = temp;
                    while ((p = strtok(NULL, ",")) != NULL) //strtok用法：第一次为字符串，后面用NULL
                    {
                        int temp = atoi(p);
                        if (temp <= 0 || temp > 65535)
                        {
                            printf("!!!PORT ERROR!!!  Check your port set.\n");
                            free(optarg_copy);
                            exit(1);
                        }
                        portlist[i++] = temp;
                    }
                }
            }
            else
            {
                int temp = atoi(optarg_copy);
                if (temp <= 0 || temp > 65535)
                {
                    printf("!!!PORT ERROR!!!  Check your port set.\n");
                    free(optarg_copy);
                    exit(1);
                }
                portlist[i++] = temp;
            }
            free(optarg_copy);
            break;
        }
        case 't': //thread
        {
            int temp = atoi(optarg);
            if (temp <= 0)
            {
                printf("!!!THREAD ERROR!!!  Check your thread set.");
                exit(1);
            }
            else
                Threads = temp;
            break;
        }
        case 'd': //debug
            debug = 1;
            break;
        default:
            usage();
            exit(1);
            break;
        }
    }
    scan(getip(argv[argc - 2]), getip(argv[argc - 1]), Threads); 
    WSACleanup();
    return 0;
}
/*
 * 本程序是httpsqs的C语言客户端函数库
 * 支持：get put view reset maxqueue status功能，暂不支持keep-alive
 * 编译：gcc -o client client.c
 * 使用方法：
 * GET:       httpsqs_get(char * your_queue_name)
 * PUT:       httpsqs_put(char * your_queue_name, char * data)
 * VIEW:      httpsqs_view(char * your_queue_name, char * pos)
 * MAXQUEUE   httpsqs_maxqueue(char * your_queue_name, char * num)
 * STATUS     httpsqs_status(char * your_queue_name)
 * RESET      httpsqs_reset(char * your_queue_name)
 * 作者：李博 lb13810398408@gmail.com
 * 日期：2010年4月7日
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define HttpsqsIp "192.168.1.102"
#define HttpsqsPort 1218
#define Charset "utf-8"

int htconnect(char *domain,int port)
{
        int white_sock;
        struct hostent * site;
        struct sockaddr_in me;
        site = gethostbyname(domain);
        if(site == NULL) return -2;
        white_sock = socket(AF_INET,SOCK_STREAM, 0);
        if(white_sock < 0) return -1;
        memset(&me, 0, sizeof(struct sockaddr_in));
        memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
        me.sin_family = AF_INET;
        me.sin_port = htons(port);
        return (connect(white_sock, (struct sockaddr *)&me, sizeof(struct sockaddr)) < 0) ? -1 : white_sock;
}

int htsend(int sock,char *fmt,...)
{
        char BUF[1024];
        va_list argptr;
        va_start(argptr, fmt);
        vsprintf(BUF,fmt, argptr);
        va_end(argptr);
        return send(sock, BUF, strlen(BUF), 0);
}

int http_get(char * query)
{
        int black_sock;

        black_sock = htconnect(HttpsqsIp, HttpsqsPort);
        if (black_sock < 0) return;

        htsend(black_sock, "GET %s HTTP/1.1\r\n", query, 10);
        htsend(black_sock, "Host: %s\r\n", HttpsqsIp, 10);
        htsend(black_sock, "Connection: close\r\n", 10);
        htsend(black_sock, "\r\n", 10);

        return black_sock;
}

int http_put(char * data, char * query)
{
        int black_sock;
        int len = 0;

        black_sock = htconnect(HttpsqsIp, HttpsqsPort);
        if (black_sock < 0) return;
        len = strlen(data);
        htsend(black_sock, "POST %s HTTP/1.1\r\n", query, 10);
        htsend(black_sock, "Host: %s\r\n", HttpsqsIp, 10);
        htsend(black_sock, "Content-Length: %d\r\n", len, 10);
        htsend(black_sock, "Connection: close\r\n", 10);
        htsend(black_sock, "\r\n", 10);
        htsend(black_sock, "%s", data, 10);

        return black_sock;
}

char * httpsqs_maxqueue(char * name, char * num)
{
	char data[3];
	int sock;
	char * query;
	char max[300];
	int i = 0, j = 0;
	char * fs;

	query = (char *)malloc((strlen(name) + strlen(num) + strlen(Charset) + 35) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=maxqueue&num=%s", Charset, name, num);

	sock = http_get(query);
	free(query);

	while (read(sock, data, 1) > 0)
	{
		if(j >= 7)
		{
			max[i] = data[0];
			i ++;
		}
		if(data[0] == '\n')
		{
			j ++;
		}
	}

	if(strlen(max) > 0)
	{
		max[i] = '\0';
	}
	fs = (char *)malloc(strlen(max) * sizeof(char));
	strcpy(fs, max);
	return fs;
}

char * httpsqs_reset(char * name)
{
	char data[3];
	int sock;
	char * query;
	char reset[300];
	int i = 0, j = 0;
	char * fs;

	query = (char *)malloc((strlen(name) + strlen(Charset) + 27) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=reset", Charset, name);

	sock = http_get(query);
	free(query);

	while (read(sock, data, 1) > 0)
	{
		if(j >= 7)
		{
			reset[i] = data[0];
			i ++;
		}
		if(data[0] == '\n')
		{
			j ++;
		}
	}
	if(strlen(reset) > 0)
	{
		reset[i] = '\0';
	}
	fs = (char *)malloc(strlen(reset) * sizeof(char));
	strcpy(fs, reset);
	return fs;
}

char * httpsqs_status(char * name)
{
	char data[3];
	int sock;
	char * query;
	char status[300];
	int i = 0, j = 0;
	char * fs;

	query = (char *)malloc((strlen(name) + strlen(Charset) + 28) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=status", Charset, name);

	sock = http_get(query);
	free(query);

	while (read(sock, data, 1) > 0)
	{
		if(j >= 7)
		{
			status[i] = data[0];
			i ++;
		}
		if(data[0] == '\n')
		{
			j ++;
		}
	}
	if(strlen(status) > 0)
	{
		status[i] = '\0';
	}
	fs = (char *)malloc(strlen(status) * sizeof(char));
	strcpy(fs, status);
	return fs;
}

char * httpsqs_put(char * name, char * bodydata)
{
	char data[3];
	char dataarr[2][20];
	char pos[10];
	int sock;
	char * query;
	int i = 0, j = 0, flag = 0;

	char queuedata[2000];
	char * fq;
	//fq = queuedata;

	query = (char *)malloc((strlen(name) + strlen(Charset) + 25) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=put", Charset, name);

	sock = http_put(bodydata, query);
	free(query);

	memset(dataarr[0], 0, strlen(dataarr[0]));
	memset(dataarr[1], 0, strlen(dataarr[1]));
	while (read(sock, data, 1) > 0)
	{
		if(j == 3)
		{
			if(data[0] == 'P')
			{
				flag = 1;
			}
			if(flag == 1)
			{
				i++;
				if(i >= 6)
				{
					dataarr[0][i-6] = data[0];
				}
			}
		}
		if(j == 4 && strlen(dataarr[0]) > 2)
		{
			for(i = 0; i < strlen(dataarr[0])-2; i++)
			{
				pos[i] = dataarr[0][i];
			}
			pos[strlen(pos)] = '\0';
			memset(dataarr[0], 0, strlen(dataarr[0]));
			strcpy(dataarr[0], pos);
			i = 0;
			if(strlen(pos) > 0)
			{
				flag = 2;
			}
		}
		if(flag == 2 && strlen(pos) > 0)
		{
			if(j == 8)
			{
				dataarr[1][i] = data[0];
				i++;
			}
		}
		if(data[0] == '\n')
		{
			j ++;
		}

	}
	if(strlen(dataarr[1]) > 0)
	{
		dataarr[1][i] = '\0';
	}
	memset(queuedata, 0, strlen(queuedata));
	sprintf(queuedata, "Pos:%s\nData:%s", dataarr[0], dataarr[1]);
	fq = (char *)malloc(strlen(queuedata) * sizeof(char));
	strcpy(fq, queuedata);
	//printf("%s\n\n\n", queuedata);
	return fq;
}

char * httpsqs_view(char * name, char * pos)
{
	char data[3];
	int sock;
	char * query;
	int i = 0, j = 0;

	char queuedata[2000];
	char * fq;

	query = (char *)malloc((strlen(name) + strlen(Charset) + strlen(pos) + 31) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=view&pos=%s", Charset, name, pos);

	sock = http_get(query);
	free(query);

	while (read(sock, data, 1) > 0)
	{
		if(j >= 7)
		{
			queuedata[i] = data[0];
			i ++;
		}
		if(data[0] == '\n')
		{
			j ++;
		}
	}
	if(strlen(queuedata) > 0)
	{
		queuedata[i] = '\0';
	}
	fq = (char *)malloc(strlen(queuedata) * sizeof(char));
	strcpy(fq, queuedata);
	return fq;
}

char * httpsqs_get(char * name)
{
	char data[3];
	char dataarr[2][2000];
	char pos[10];
	int sock;
	char * query;
	int i = 0, j = 0, flag = 0;

	char queuedata[2000];
	char * fq;

	query = (char *)malloc((strlen(name) + strlen(Charset) + 25) * sizeof(char));
	sprintf(query, "/?charset=%s&name=%s&opt=get", Charset, name);

	sock = http_get(query);
	free(query);
	
	memset(dataarr[0], 0, strlen(dataarr[0]));
	memset(dataarr[1], 0, strlen(dataarr[1]));
	while (read(sock, data, 1) > 0)
	{
		if(j == 3)
		{
			if(data[0] == 'P')
			{
				flag = 1;
			}
			if(flag == 1)
			{
				i++;
				if(i >= 6)
				{
					dataarr[0][i-6] = data[0];
				}
			}
		}
		if(j == 4 && strlen(dataarr[0]) > 2)
		{
			for(i = 0; i < strlen(dataarr[0])-2; i++)
			{
				pos[i] = dataarr[0][i];
			}
			pos[strlen(pos)] = '\0';
			memset(dataarr[0], 0, strlen(dataarr[0]));
			strcpy(dataarr[0], pos);
			i = 0;
			if(strlen(pos) > 0)
			{
				flag = 2;
			}
		}
		if(flag == 2 && strlen(pos) > 0)
		{
			if(j == 8)
			{
				dataarr[1][i] = data[0];
				i++;
			}
		}
		if(data[0] == '\n')
		{
			j ++;
		}
	}
	
	if(strlen(dataarr[1]) > 0)
	{
		dataarr[1][strlen(dataarr[1])] = '\0';
	}

	memset(queuedata, 0, strlen(queuedata));
	sprintf(queuedata, "Pos:%s\nData:%s", dataarr[0], dataarr[1]);
	fq = (char *)malloc(strlen(queuedata) * sizeof(char));
	strcpy(fq, queuedata);
	return fq;
}

int main(int argc,char **argv)
{
	char * data;

	data = httpsqs_put(argv[1], argv[2]);
	printf("%s\n", data);
	free(data);

	data = httpsqs_status(argv[1]);
	printf("%s\n", data);
	free(data);
	
	data = httpsqs_get(argv[1]);
	printf("%s\n", data);
	free(data);
	
	data = httpsqs_reset(argv[1]);
	printf("%s\n", data);
	free(data);
	/*
	data = httpsqs_maxqueue(argv[1], argv[2]);
	printf("%s\n", data);
	free(data);

	data = httpsqs_view(argv[1], argv[2]);
	printf("%s\n", data);
	free(data);
	*/
}

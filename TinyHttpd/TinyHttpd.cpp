// TinyHttpd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "TinyHttpd.h"
#include"ThreadPool.h"

#include <string>
#include <iostream>
#include <algorithm>
#include<fstream>
#include<io.h>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_STRING "Server: TinyHttpd/0.1.0\r\n"

using namespace spider;

std::string getContentType(std::string filepath)
{
	int i = filepath.size() - 1;
	while (i > 0 && filepath[i] != '.')
		--i;
	std::string fileType = filepath.substr(i+1);
	std::transform(fileType.begin(), fileType.end(),fileType.begin(), ::tolower);

	// todo 以后需要修改，根据请求获取

	if (fileType == "js")
		return "application/javascript";
	if (fileType == "css")
		return "text/css";

	return "text/html";
}
void headers(SOCKET client,std::string file)
{
	std::string buf;
	buf = "HTTP/1.0 200 OK\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = SERVER_STRING;
	send(client, buf.c_str(), buf.size(), 0);
	buf = "Content-Type: "+ getContentType(file) +";charset=UTF-8\r\n";//todo 按照文件输出
	send(client, buf.c_str(), buf.size(), 0);
	buf = "\r\n";
	send(client, buf.c_str(), buf.size(), 0);
}
void errorMethod(SOCKET client)
{
	std::string buf;
	std::cout << "errorMethod" << std::endl;
	/* HTTP method 不被支持*/
	buf = "HTTP/1.0 501 Method Not Implemented\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	/*服务器信息*/
	buf = SERVER_STRING;
	send(client, buf.c_str(), buf.size(), 0);
	buf = "Content-Type: text/html\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "<HTML><HEAD><TITLE>Method Not Implemented\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "</TITLE></HEAD>\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "<BODY><P>HTTP request method not supported.\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "</BODY></HTML>\r\n";
	send(client, buf.c_str(), buf.size(), 0);
}
void notfound(SOCKET client){
	std::string buf;
	std::cout << "404 not found" << std::endl;
	/* HTTP method 不被支持*/
	buf="HTTP/1.0 404 Method Not Implemented\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	/*服务器信息*/
	buf = SERVER_STRING;
	send(client, buf.c_str(), buf.size(), 0);
	buf = "Content-Type: text/html\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "<HTML><HEAD><TITLE>404 Not Found\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "</TITLE></HEAD>\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "<BODY><P>404 Not Found. File is not exist.\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "</BODY></HTML>\r\n";
	send(client, buf.c_str(), buf.size(), 0);
}
int splitString(std::string input,std::vector<std::string>& res,std::function<bool(char)> ignore)
{
	int i = 0;
	while (i<input.size()) {
		while (i < input.size() && ignore(input[i]))
			++i;
		std::string cur;
		while (i < input.size() && !ignore(input[i]))
			cur.push_back(input[i++]);
		if(!cur.empty())
			res.emplace_back(cur);
	}
	return res.size();
}
int getlineFromSocket(SOCKET client, std::string &line,int maxSize=-1)
{
	line.clear();
	line.resize(0);
	char buf='\0';
	int n;
	while (buf != '\n'&&(maxSize==-1||line.size()<maxSize)) {
		n = recv(client, &buf, 1, 0);
		std::cout << buf;
		if (n > 0)
		{
			if (buf == '\r')
			{
				n = recv(client, &buf, 1, MSG_PEEK);
				if ((n > 0) && (buf == '\n'))
					recv(client, &buf, 1, 0);
				else
					buf = '\n';
			}
			line.push_back(buf);
		}
		else
			buf = '\n';
	}
	std::cout << line << std::endl;

	return line.size();
}
void serveFile(SOCKET client, std::string filepath) {
	{
		std::string temp;
		int i = 1;
		
		while (i > 0 &&( temp.empty() || (temp.front() != '\n'&&temp.front() != '\r')))
		{
			i = getlineFromSocket(client, temp);
		}
		//if ((temp.front() == '\n'||temp.front()== '\r')) {
		//	std::cout << "aaaaaa: ";
		//	getlineFromSocket(client, temp);
		//	if ((temp.front() == '\n' || temp.front() == '\r')) {
		//		getlineFromSocket(client, temp);
		//	}
		//}
	}
	std::ifstream openfile;
	openfile.open(filepath);
	if (!openfile)
		notfound(client);
	else {
		headers(client,filepath);
		while (!openfile.eof())
		{
			std::string temp;
			std::getline(openfile, temp);
			//temp +=" aaa";
			//std::cout << temp << std::endl;
			send(client, temp.c_str(), temp.size(), 0);
		}
	}
}

void acceptHttpd(SOCKET client) {
	bool needExcute = false;
	std::string line;

	//char buf = '\0';
	//int s=1;
	//while (s>0)
	//{
	//	s = recv(client, &buf, 1, 0);
	//	if (buf == '\n') std::cout << "\\n";
	//	if (buf == '\r') std::cout << "\\r";
	//	std::cout << buf ;
	//	line.push_back(buf);
	//}
	//std::cout << line << std::endl;

	//return ;



	int n = getlineFromSocket(client, line,1024);
	//std::cout << "=====  accept in  =====" << std::endl;
	//std::cout << "First header: " << line << std::endl;

	int i = 0, j = 0;
	std::vector<std::string> res;
	
	if (splitString(line, res, [](char a) {return isspace(a); }) < 3) {
		errorMethod(client);
		closesocket(client);
		return;
	}
	//while (!isspace(line[j]) && (i < 254))
	//{
	//	method.push_back(line[j]);
	//	++i; ++j;
	//}

	std::string method = res[0];
	std::string url = res[1];
	std::string version = res[2];
	//std::cout << "method: " << method << std::endl;
	//std::cout << "url: " << url << std::endl;
	//std::cout << "version: " << version << std::endl;
	std::transform(method.begin(), method.end(), method.begin(), ::toupper);
	if (method != "GET"&&method!="POST")
	{
		errorMethod(client);
		closesocket(client);
		return;
	}

	// post 请求处理
	if (method == "POST")
	{
		needExcute = true;
	}

	// 处理url，将url映射到文件中
	// 获取get请求的内容
	std::string query_string;
	if (method == "GET") {
		int i = 0;
		needExcute = true;
		while (i < url.size() && url[i] != '?')++i;
		if (i < url.size()-1)
		{
			query_string = url.substr(i + 1, (url.size() - (i + 1)));
			url = url.substr(0, i);
		}
	}
	std::string path = "htdoc" + url;
	if (path.back() == '/')
		path +="index.html";
	
	//std::cout <<"path: "<< path << std::endl;
	// 打开文件
	
	if (_access(path.data(), 2)==-1)
	{
		std::cout << "查找失败" << std::endl;
		while (n > 0&&line.front()!='\n')
			n = getlineFromSocket(client, line, 1024);

		notfound(client);
		closesocket(client);
		return;
	}

	// todo 处理post或get请求
	if (needExcute)
	{
		serveFile(client, path);
	}
	else {
		serveFile(client, path);
	}

	closesocket(client);
}

int startHttpd(u_short port,int maxThread) {
	WSADATA socketData;
	if (WSAStartup(MAKEWORD(2, 2), &socketData) != 0)
	{
		std::cout << "创建失败" << std::endl;
		return -1;
	}
	SOCKET httpd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpd == INVALID_SOCKET)
	{
		std::cout << "创建失败" << std::endl;
		return -1;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(httpd, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		std::cout << "创建失败" << std::endl;
		return -1;
	}

	if (listen(httpd, 5) == SOCKET_ERROR)
	{
		std::cout << "创建失败" << std::endl;
		return -1;
	}
	SOCKET client;
	sockaddr_in client_in;
	int naddrLen = sizeof(client_in);
	ThreadPool pool(maxThread);
	while (true) {
		client = accept(httpd, nullptr, nullptr);
		//client = accept(httpd,(sockaddr*)(&client_in), &naddrLen);
		if (client == -1)
			return -1;
		//acceptHttpd(client);
		pool.append(std::bind(acceptHttpd, client));
	}
	return -1;
}





int main()
{
	startHttpd(8081, 10);

}


#include <iostream>
#include <winsock2.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include "IRequest.h"
using namespace std;

#define MAXPATH 1024

void initializeWinsock();
bool getOptions(SOCKET client_socket, char* recv_buf);
bool setupTrans(SOCKET client_socket, char* recv_buf, unsigned short port);

int seq = 0;

int main() {

	initializeWinsock();

	unsigned short port = 7777;

	char *recv_buf = new char[MAXPATH];

	SOCKET client_socket;

	// 配置服务端地址和端口
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5000);

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	SOCKADDR_IN client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(port);
	bind(client_socket, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));

	if (connect(client_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "[!]fail to connect" << endl;
		WSACleanup();
		closesocket(client_socket);
		return 0;
	}
	else {
		cout << "[*]connect succ" << endl;
	}

	int message;
	// getOptions(client_socket, recv_buf);
	while (1) {
		cout << "0:OPTIONS, 1:SETUP, 2:PLAY, 3:TEARDOWN" << endl;
		cout << "input->";
		cin >> message;
		if (message == 0) {
			if (getOptions(client_socket, recv_buf)) {
				cout << "res text: \n " << recv_buf << endl;
				seq++;
			}
			else {
				cout << "[!]ERROR: get options failed" << endl;
			}
		}
		else if (message == 1) {
			if (setupTrans(client_socket, recv_buf, port)){
				cout << "res text: \n " << recv_buf << endl;
				seq++;
			}
			else {
				cout << "[!]ERROR: get options failed" << endl;
			}
		}
	}

	closesocket(client_socket);
	WSACleanup();

	return 0;
}

void initializeWinsock() {
	WORD w_req = MAKEWORD(2, 2);
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "initialize fail" << endl;
	}
	else {
		cout << "initialize succ" << endl;
	}

	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "Version number doesn't match" << endl;
		WSACleanup();
	}
	else {
		cout << "[*] initialize winsock succ!" << endl;
	}
}

bool getOptions(SOCKET client_socket, char* recv_buf) {
	IRequest* request;
	request = new IRequest("options", "127.0.0.1:5000", seq);
	string post = request->toString();
	const char* send_buf = post.c_str();

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}

	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}
	return true;
}

bool setupTrans(SOCKET client_socket, char* recv_buf, unsigned short port){

	IRequest* request;
	
	string body;
	body = string("Transport:TCP") + "\r\n";
	body += "client_port=" + to_string(port) + "\r\n";

	request = new IRequest("setup", "127.0.0.1:5000", seq, body);
	string post = request->toString();
	const char* send_buf = post.c_str();
	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}

	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}
	return true;
}

bool play(SOCKET client_socket, char* recv_buf, int sessionId, int min, int sec) {
	IRequest* request;

	string body;
	body = "Session_id=" + to_string(sessionId) + "\r\n";
	body += "Range: ntp=" + to_string(min) + '-' + to_string(sec) + "\r\n";

	request = new IRequest("setup", "127.0.0.1:5000", seq, body);
	string post = request->toString();
	const char* send_buf = post.c_str();

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}

	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}
	return true;
}

bool stopPlay(SOCKET client_socket, char* recv_buf, int sessionId) {
	IRequest* request;

	string body;
	body = "Session_id=" + to_string(sessionId) + "\r\n";
 
	request = new IRequest("setup", "127.0.0.1:5000", seq, body);
	string post = request->toString();
	const char* send_buf = post.c_str();

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}

	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}
	return true;
}

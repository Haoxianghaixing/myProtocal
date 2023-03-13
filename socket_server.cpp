#include <iostream>
#include <winsock2.h>
#include <unistd.h>
#include <sstream>
#include <vector>
using namespace std;

#define MAXPATH 1024

const char* methods[4] = {"OPTIONS", "SETUP", "PLAY", "TEARDOWN"};

void initializeWinsock();

int main(){

	initializeWinsock();

	int send_len = 0;
	int recv_len = 0;
	int len = 0;

	char* send_buf = new char[MAXPATH];
	char* recv_buf = new char[MAXPATH];

	SOCKET server_socket;
	SOCKET accept_socket;

	// 配置服务端地址和端口
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;	// IPv4
	// server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");	// ip
	server_addr.sin_port = htons(5000);	// port

	SOCKADDR_IN accept_addr;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(bind(server_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR){
		cout << "[!]fail to bind" << endl;
		WSACleanup();
	}
	else{
		cout << "[*]bind succ: " << endl;
		cout << inet_ntoa(server_addr.sin_addr) << ':' << ntohs(server_addr.sin_port) << endl;
	}
	if(listen(server_socket, 1) < 0){
		cout << "[!]fail to listen" << endl;
		WSACleanup();
	}
	else{
		cout << "[*]listen succ" << endl;
	}


	len = sizeof(SOCKADDR);
	accept_socket = accept(server_socket, (SOCKADDR *)&accept_addr, &len);
	if(int(accept_socket) == SOCKET_ERROR){
		cout << "fail to connect" << endl;
		WSACleanup();
		return 0;
	}
	cout << "connected succ" << endl;

	while(1){
		recv_len = recv(accept_socket, recv_buf, MAXPATH, 0);
		if(recv_len	< 0){
			cout << "fail to receive" << endl;
			break;
		}
		else{
			// 处理报文
			vector<string> res;
			stringstream input(recv_buf);
			string s;
			while(input >> s)
				res.push_back(s);
			// for(int i = 0; i < res.size(); i++){
			// 	cout << res[i] << endl;
			// }
			cout << recv_buf << endl;
			if(res[0] =="OPTIONS"){
				sprintf(send_buf, "%s %s %s %s", methods[0], methods[1], methods[2], methods[3]);
				cout << send_buf << endl;
			}

			send_len = send(accept_socket, send_buf, MAXPATH, 0);
			if(send_len < 0){
				cout << "fail to send" << endl;
				break;
			}
		}
	}

	closesocket(server_socket);
	closesocket(accept_socket);
	WSACleanup();

	return 0;
}

// 初始化 winsock 环境
void initializeWinsock(){
	WORD w_req = MAKEWORD(2, 2);
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if(err != 0){
		cout << "initialize fail" << endl;
	}
	else{
		cout << "initialize succ" << endl;
	}

	if(LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2){
		cout << "Version number doesn't match" << endl;
		WSACleanup();
	}
	else{
		cout << "[*] initialize winsock succ!" << endl;
	}
}
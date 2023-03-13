#include <iostream>
#include <winsock2.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
using namespace std;

#define MAXPATH 1024

// 报文类
class IRequestPost{
private:
	string version;
	string method;
	string url;
	int seq;
	string params;
public:
	IRequestPost(string _method, string _url, int _seq): method(_method), url(_url), seq(_seq)
	{
		version = "0.5";
		cout << "create request post succ" << endl;
	}
	string concatenetPost();
};

// 拼接报文
string IRequestPost::concatenetPost() {
	ostringstream oss;
	string myPost;
	transform(method.begin(), method.end(), method.begin(), ::toupper);
	oss << method << ' ' << url << ' ' << version << "\r\n";
	oss << "CSeq: " << seq << "\r\n";
	myPost = oss.str();
	return myPost;
}

void initializeWinsock();
bool getOptions(SOCKET client_socket, char* recv_buf);

int seq = 0;

int main() {

	initializeWinsock();

	char *send_buf = new char[MAXPATH];
	char *recv_buf = new char[MAXPATH];

	SOCKET client_socket;

	// 配置服务端地址和端口
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5000);

	client_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(connect(client_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR){
		cout << "[!]fail to connect" << endl;
		WSACleanup();
		closesocket(client_socket);
		return 0;
	}
	else{
		cout << "[*]connect succ" << endl;
	}

	int message;
	while(1){
		cout << "0:OPTIONS, 1:SETUP, 2:PLAY, 3:TEARDOWN" << endl;
		cout << "input->";
		cin >> message;
		if(message == 0){
			if(getOptions(client_socket, recv_buf)){
				cout << "server info: " << recv_buf << endl;
				seq++;
			}
			else{
				cout << "[!]ERROR: get options failed" << endl;
			}
		}
		else if(message == 3){

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

bool getOptions(SOCKET client_socket, char* recv_buf){
	IRequestPost* request;
	string post;
	request = new IRequestPost("options", "127.0.0.1:5000", seq);
	post = request->concatenetPost();
	char* send_buf = (char *)post.data();


	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if(send_len < 0){
		cout << "[!]fail to send" << endl;
		return false;
	}

	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if(recv_len < 0){
		cout << "[!]fail to recv" << endl;
		return false;
	}	
	return true;
}
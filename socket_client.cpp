#include <iostream>
#include <winsock2.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include "IRequest.h"
#include <conio.h>
#include <vector>
using namespace std;

#define MAXPATH 1024

void initializeWinsock();
bool getOptions(SOCKET client_socket, char* recv_buf);
bool setupTrans(SOCKET client_socket, char* recv_buf, unsigned short port);
bool stopPlay(SOCKET client_socket, char* recv_buf, int session_id);
bool play(SOCKET client_socket, char* recv_buf, int session_id, int min, int sec);
int seq = 0;

vector<IRequest*> session_record;

int main() {

	initializeWinsock();

	unsigned short port = 6666;

	char recv_buf[MAXPATH];

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

	if(getOptions(client_socket, recv_buf)){
		cout << "OPTIONS: " << recv_buf << endl;
	}
	else{
		cout << "[!]Failed to get options" << endl;
	}
	string message;
	int session_id;
	// getOptions(client_socket, recv_buf);
	while (1) {
		cout << "command->";
		cin >> message;
		transform(message.begin(), message.end(), message.begin(), ::tolower);
		if(message == "options"){
			getOptions(client_socket, recv_buf);
		}
		else if (message == "setup") {
			seq++;
			if (setupTrans(client_socket, recv_buf, port)){
				cout << "res text: \n " << recv_buf << endl;
			}
			else {
				cout << "[!]ERROR: get options failed" << endl;
			}
		}
		else if (message == "play"){
			seq++;
			play(client_socket, recv_buf, session_id, 0, 0);	
			break;		
		}
		else if (message == "teardown"){
			seq++;
			if(stopPlay(client_socket, recv_buf, 0)){
				cout << "End connect" << endl;
				break;
			}
		}
	}

	cout << "\n[*]End the session" << endl;
	for(int i = 0; i < (int)session_record.size(); i++){
		IRequest* request = session_record[i];
		cout << request->toString();
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

// parse an request and return an IRequest object
IRespond parseRespond(string res) {
	istringstream ss(res);
	string line;
	string version, code, status, body;
	int cseq = -1;

	// parse the first line to get the method, url and version
	getline(ss, line);
	istringstream lineStream(line);
	lineStream >> version >> code >> status;

	// parse the remaining lines to get the headers
	while (getline(ss, line) && line != "\r") {
		if (line.substr(0, 5) == "CSeq:") {
			// parse the CSeq
			istringstream cseqStream(line.substr(5));
			cseqStream >> cseq;
		} else {
			// get body
			body += line + "\r\n";
		}
	}

	return IRespond(cseq, code, body);
}


bool getOptions(SOCKET client_socket, char* recv_buf) {
	IRequest* request;
	request = new IRequest("options", "127.0.0.1:5000", seq);
	string post = request->toString();
	const char* send_buf = post.c_str();

	session_record.push_back(request);

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}
	memset(recv_buf, 0, sizeof(recv_buf));
	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}

	IRespond res = parseRespond(string(recv_buf));
	string body = res.getBody();
	strcpy(recv_buf, body.c_str());
	return true;
}

bool setupTrans(SOCKET client_socket, char* recv_buf, unsigned short port){

	IRequest* request;
	
	string body;
	body = string("Transport:TCP") + "\r\n";
	body += "client_port=" + to_string(port) + "\r\n";

	request = new IRequest("setup", "127.0.0.1:5000", seq, body);
	session_record.push_back(request);

	string post = request->toString();
	const char* send_buf = post.c_str();
	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}
	memset(recv_buf, 0, sizeof(recv_buf));
	int recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
	if (recv_len < 0) {
		cout << "[!]fail to recv" << endl;
		return false;
	}

	IRespond res = parseRespond(string(recv_buf));
	string res_body = res.getBody();
	strcpy(recv_buf, body.c_str());
	return true;
}

bool play(SOCKET client_socket, char* recv_buf, int session_id, int min, int sec) {
	IRequest* request;
	int recv_len = 0;

	string body;
	body = "Session_id=" + to_string(session_id) + "\r\n";
	body += "Range: ntp=" + to_string(min) + '-' + to_string(sec) + "\r\n";

	request = new IRequest("play", "127.0.0.1:5000", seq, body);
	session_record.push_back(request);

	string post = request->toString();
	const char* send_buf = post.c_str();

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}
	while(true){
		if(!kbhit()){
			recv_len = recv(client_socket, recv_buf, 100, 0);
			cout << recv_buf;
			if (recv_len < 0) {
				cout << "[!]fail to recv" << endl;
				break;
			}
		}
		else{
			char c = getch();
			if(c == ' '){
				seq++;
				stopPlay(client_socket, recv_buf, session_id);
				break;
			}
		}
	}
	return true;
}

bool stopPlay(SOCKET client_socket, char* recv_buf, int session_id) {
	IRequest* request;

	string body;
	body = "session_id=" + to_string(session_id) + "\r\n";
 
	request = new IRequest("teardown", "127.0.0.1:5000", seq, body);
	session_record.push_back(request);
	string post = request->toString();
	const char* send_buf = post.c_str();

	int send_len = send(client_socket, send_buf, MAXPATH, 0);
	if (send_len < 0) {
		cout << "[!]fail to send" << endl;
		return false;
	}
	return true;
}

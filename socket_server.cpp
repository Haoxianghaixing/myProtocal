#include <iostream>
#include <winsock2.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include "IRequest.h"
#include <sstream>
#include <ctime>
#include <map>
#include <random>

using namespace std;

#define MAXPATH 1024

string methods[4] = {"OPTIONS", "SETUP", "PLAY", "TEARDOWN"};

void initializeWinsock();
void bindToPort(SOCKET server_socket, unsigned short port);
void runServer(int port);
IRequest parseHttpRequest(string request);
unsigned short getClientPort(IRequest request);
void playTheVideo(void* arg);

map<int, SOCKET> session_map;

int main() {

	initializeWinsock();


	runServer(5000);

	WSACleanup();

	return 0;
}

// 初始化 winsock 环境
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

IRequest parseHttpRequest(string request) {
	istringstream ss(request);
	string line;
	string method, url, version, body;
	int cseq = -1;

	// parse the first line to get the method, url and version
	getline(ss, line);
	istringstream lineStream(line);
	lineStream >> method >> url >> version;

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

	return IRequest(method, url, cseq, body);
}
unsigned short getClientPort(IRequest request){

	string body = request.getBody();
	size_t pos = body.find("client_port");
	unsigned short port;
	if (pos != std::string::npos) {
		pos = body.find("=", pos);
        string portStr = body.substr(pos + 1);
        port = stoi(portStr);
        cout << "port: " << port << endl;
    }
    return port;
}

void handleClient(void *arg){
	HANDLE thread;
	int client_socket = *(int*)arg;
	int recv_len = 0;
	int send_len = 0;
	char* send_buf = new char[MAXPATH];
	// get client request
	char* recv_buf = new char[MAXPATH];
	while(true){
		cout << "=========" << endl;
		recv_len = recv(client_socket, recv_buf, MAXPATH, 0);
		if(recv_len < 0){
			break;
		}
		cout << recv_buf << endl;
		// parse request
		IRequest recv = parseHttpRequest(string(recv_buf));
		string method = recv.getMethod();
		int seq = recv.getCseq();

		if (method == "OPTIONS") {
			string body;
			body = methods[0] + ' ' + methods[1] + ' ' + methods[2] + ' ' + methods[3];
			IRespond* respond = new IRespond(seq, "200", body);
			string text = respond->toString();
			send_buf = (char *)text.c_str();
			cout << send_buf << endl;

			send_len = send(client_socket, send_buf, MAXPATH, 0);
			if (send_len < 0) {
				cout << "fail to send" << endl;
			}
		} 
		else if (method == "SETUP") {
			// get port
			unsigned short port = getClientPort(recv);

			// use timestamp to get session_id
			time_t current_time = time(nullptr);
			mt19937 rng(current_time);
			int session_id = rng();

			session_map[session_id] = client_socket;

			string body;
			body = string("session_id:") + to_string(session_id);
			IRespond* respond = new IRespond(seq, "200", body);
			string text = respond->toString();
			send_buf = (char *)text.c_str();

			send_len = send(client_socket, send_buf, MAXPATH, 0);
			if (send_len < 0) {
				cout << "fail to send" << endl;
			}
		} 
		else if (method == "PLAY") {
			thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)playTheVideo, &client_socket, 0, NULL);
		} 
		else if (method == "TEARDOWN") {
			cout << "close connect" << endl;
			string body = recv.getBody();
			size_t pos = body.find("session_id");
			int session_id;
			if (pos != std::string::npos) {
				pos = body.find("=", pos);
		        string portStr = body.substr(pos + 1);
		        session_id = stoi(portStr);
		    }
			CloseHandle(thread);

			close(session_map[session_id]);
			break;
		} 
		else {}
	}
	
}

void playTheVideo(void* arg){
	int client_socket = *(int*)arg;
	while(true){
		string send_buf = string(100, 'x');
		int send_len = send(client_socket, send_buf.c_str(), send_buf.size(), 0);
		usleep(1000000); 
		if(send_len < 0){
			return;
		}
	}
}

void runServer(int port){
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cout << "[!]Failed to create server_socket" << endl;
        return;
    }

    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

	if (bind(server_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "[!]fail to bind" << endl;
		return;
	}
	if (listen(server_socket, 5) < 0) {
		cout << "[!]fail to listen" << endl;
		return;
	}

    cout << "Waiting for incoming connections on port: " << port << " ..." << endl;

    while (true) {
        SOCKADDR_IN client_addr;
        int clinet_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (SOCKADDR *)&client_addr, &clinet_addr_len);
        if (client_socket < 0) {
            cout << "[!]Failed to accept connection" << endl;
            continue;
        }

        cout << "Connected to client " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;

        // create a new thread to handle the client connection
        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, &client_socket, 0, NULL);
        if(thread != NULL){
        	CloseHandle(thread);
        }
    }

    close(server_socket);
}
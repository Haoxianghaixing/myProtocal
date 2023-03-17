#include <iostream>
#include <algorithm>
using namespace std;
class IRequest {
private:
	string method;
	string url;
	string version;
	int cseq;
	string body;

public:
	IRequest(string method, string url, int cseq, string body = "")
		: method(method), url(url), cseq(cseq), body(body) {
		version = "0.5";
	}

	// getters
	string getMethod() const { return method; }
	string getUrl() const { return url; }
	string getVersion() const { return version; }
	int getCseq() const { return cseq; }
	string getBody() const { return body; }

	// concatenet the entire respond as a string
	string toString() {
		transform(method.begin(), method.end(), method.begin(), ::toupper);
		string respond = method + " diantp://" + url + " " + version + "\r\n";
		respond += "CSeq: " + to_string(cseq) + "\r\n";
		respond += body + "\r\n";
		respond += "\r\n";
		return respond;
	}
};

class IRespond{
private:
	string version;
	int cseq;
	string code;
	string body;
public:
	IRespond(int cseq, string code, string body = "")
		: cseq(cseq), code(code), body(body) {
		version = "0.5";
	}

	// getters
	string getVersion() const { return version; }
	string getCode() const { return code; }
	int getCseq() const { return cseq; }
	string getBody() const { return body; }

	// concatenet the entire respond as a string
	string toString() {
		if(code == "200")
			code += " OK";
		string respond = version + ' ' + code + "\r\n";
		respond += "CSeq: " + to_string(cseq) + "\r\n";
		respond += body + "\r\n";
		respond += "\r\n";
		return respond;
	}
};
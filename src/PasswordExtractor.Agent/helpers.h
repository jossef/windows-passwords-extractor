#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "zlib.h"

using namespace std;

static const string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(BYTE c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64Encode(const vector<BYTE>& data) {

	BYTE const* buf = &data[0];
	unsigned int bufLen = data.size();
	string ret;
	int i = 0;
	int j = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];

	while (bufLen--) {
		char_array_3[i++] = *(buf++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

void readFile(const string& filePath, vector<BYTE>& data)
{
	// open the file:
	ifstream file(filePath, ios::binary);

	// Stop eating new lines in binary mode!!!
	file.unsetf(ios::skipws);

	// get its size:
	file.seekg(0, ios::end);
	auto fileSize = file.tellg();
	file.seekg(0, ios::beg);

	// reserve capacity
	data.reserve(fileSize);

	// read the data:
	data.insert(data.begin(),
		istream_iterator<BYTE>(file),
		istream_iterator<BYTE>());

}

string getTempPath()
{
	char buffer[MAX_PATH];
	GetTempPath(MAX_PATH, buffer);
	return buffer;
}

string trimEnd(const string& str, char character)
{
	auto last = str.find_last_not_of(character);
	return str.substr(0, last + 1);
}

string join(vector<string> items)
{
	stringstream path;

	for (auto iter = items.begin(); iter != items.end(); iter++)
	{
		if (iter != items.begin())
		{
			path << "\\";
		}

		auto item = trimEnd(*iter, '\\');
		path << item;
	}

	return path.str();
}

int execute(vector<string> args, bool print = true)
{
	stringstream command;

	for (auto iter = args.begin(); iter != args.end(); iter++)
	{
		if (iter != args.begin())
		{
			command << " ";
		}

		auto item = *iter;
		command << item;
	}

	if (print)
	{
		cout << command.str() << endl;
	}

	// return system(command.str().data());

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pInfo = { 0 };

	int result = CreateProcess(NULL, (LPSTR)command.str().data(), NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pInfo);
	
	if (result == 0)
	{
		return S_FALSE;
	}

	WaitForSingleObject(pInfo.hProcess, INFINITE);

	DWORD exitCode;
	GetExitCodeProcess(pInfo.hProcess, &exitCode);

	CloseHandle(pInfo.hProcess);
	CloseHandle(pInfo.hThread);

	return exitCode;
}

void postHTTP(const string& host, const string& page, const vector<BYTE>& data)
{
	// ---------------------
	// Init Socket

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "WSAStartup failed" << endl;
		return;
	}

	SOCKET socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct hostent* hostInfo = gethostbyname(host.data());
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)hostInfo->h_addr);

	if (connect(socketId, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0){
		cout << "Could not connect to " << host << endl;
		return;
	}


	// ---------------------
	// Build request

	stringstream requestHeaders;


	requestHeaders << "POST " << page << " HTTP/1.1" << "\r\n";
	requestHeaders << "Host: " << host << "\r\n";
	requestHeaders << "User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.95 Safari/537.36" << "\r\n";
	requestHeaders << "Content-Type: " << "application/json" << "\r\n";
	requestHeaders << "Content-Length: " << data.size() << "\r\n";
	requestHeaders << "\r\n";

	auto requestHeadersString = requestHeaders.str();
	std::vector<BYTE> requestData;
	requestData.insert(requestData.end(), requestHeadersString.begin(), requestHeadersString.end());
	requestData.insert(requestData.end(), data.begin(), data.end());

	// ---------------------
	// Send request

	send(socketId, (const char*) &requestData[0], requestData.size(), 0);

	char buffer[10000];
	int nDataLength;
	nDataLength = recv(socketId, buffer, 10000, 0);
	int i = 0;
	while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
		cout << buffer[i];
		i += 1;
	}

	cout << endl;

	closesocket(socketId);
	WSACleanup();
}


void compress(const vector<BYTE>& uncompressedData, const string& compressedFilePath)
{
	gzFile fi = gzopen(compressedFilePath.data(), "wb");
	gzwrite(fi, &uncompressedData[0], uncompressedData.size());
	gzclose(fi);
}
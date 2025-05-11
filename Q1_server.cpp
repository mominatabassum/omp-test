// server.cpp
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
WSADATA wsaData;
SOCKET serverSocket, clientSocket;
sockaddr_in serverAddr, clientAddr;

#define PORT 8080

void init() {
    int clientSize = sizeof(clientAddr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 1);

    cout << "[SERVER] Waiting for connection...\n";
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
    cout << "[SERVER] Client connected.\n";
}

int main() {
    cout << "\033[2J\033[1;1H";

    init();

    int size;
    recv(clientSocket, (char*)&size, sizeof(int), 0);
    cout << "[SERVER] Received size: " << size << "\n";

    int* buffer = new int[size];

    int received = 0;
    while (received < size * sizeof(int)) {
        int r = recv(clientSocket, ((char*)buffer) + received, (size * sizeof(int)) - received, 0);
        if (r <= 0) break;
        received += r;
    }

    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }

    send(clientSocket, (char*)&sum, sizeof(sum), 0);
    cout << "[SERVER] Sent partial sum: " << sum << "\n";

    delete[] buffer;
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

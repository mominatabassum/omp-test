#include <iostream>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")


using namespace std;
WSADATA wsaData;
SOCKET clientSocket;
sockaddr_in serverAddr;

#define PORT 8080
#define BYTES 400 * 1024 * 1024 // 400 MB in bytes
#define SIZ (BYTES / sizeof(int))
int arr[SIZ];

int ssum = 0;
int psum = 0;

// ------------ Init ------------
void init() {
    srand(time(0));
    for (int i = 0; i < SIZ; i++) {
        arr[i] = rand() % 10;
    }

    // --- Setup socket ---
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Could not connect to server. Is it running?\n";
        closesocket(clientSocket);
        WSACleanup();
        return;
    }
}



// ------------ Serial ------------
void serialAdd() {
    ssum = 0;
    for (int i = 0; i < SIZ; i++) {
        ssum += arr[i];
    }
    cout << "Serial Sum: " << ssum << "\n\n";
}



// ------------ Parallel ------------
void parallelAdd() {
    psum = 0;
    omp_set_num_threads(8);
    #pragma omp parallel for reduction(+:psum)
    for (int i = 0; i < SIZ; i++) {
        psum += arr[i];
    }
    cout << "Parallel Sum (8 threads): " << psum << "\n\n";
}




// ------------ Distributed ------------
void distributedAdd() {
    int half = SIZ / 2;
    long long localSum = 0;



    // --- Send half array to server ---
    int sizeToSend = SIZ - half;
    send(clientSocket, (char*)&sizeToSend, sizeof(int), 0);
    send(clientSocket, (char*)&arr[half], sizeToSend * sizeof(int), 0);

    // --- Receive sum from server ---
    long long remoteSum;
    recv(clientSocket, (char*)&remoteSum, sizeof(remoteSum), 0);

    closesocket(clientSocket);
    WSACleanup();

    for (int i = 0; i < half; i++) {
        localSum += arr[i];
    }

    long long total = localSum + remoteSum;
    cout << "Distributed Sum (client+server): " << total << "\n\n";
}




int main() {
    cout << "\033[2J\033[1;1H"; // OCD screen clear

    init();

    double start = omp_get_wtime();
    serialAdd();
    double end = omp_get_wtime();
    cout << "Time taken: " << (end - start) * 1000 << " ms\n\n";

    start = omp_get_wtime();
    parallelAdd();
    end = omp_get_wtime();
    cout << "Time taken: " << (end - start) * 1000 << " ms\n\n";

    start = omp_get_wtime();
    distributedAdd();
    end = omp_get_wtime();
    cout << "Time taken: " << (end - start) * 1000 << " ms\n\n";

    return 0;
}

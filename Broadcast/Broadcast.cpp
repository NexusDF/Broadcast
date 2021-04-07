// Broadcast.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

// Константы
#define PORT 25565
#define BUFFER_SIZE 8192


// Методы
int initWS();
int setSocketOption();
void printer(std::string msg);
void tcpHandler();

// Переменные
int counter = 0;
std::string message;
char buffer[BUFFER_SIZE];
SOCKET sock;
sockaddr_in local_addr, other_addr;
SOCKET clients[100];

int main()
{
    setlocale(LC_CTYPE, "rus");
    system("cls");

    if (initWS()) {
        printer("Ошибка: Возникли проблемы при инициализации...");
        return -1;
    }
    printer("Инициализация прошла успешна!");
    
    // Создание сокета
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Установка широковещательного сигнала
    if (setSocketOption()) {
        printer("Ошибка: Возникли проблемы при установки широковещательного сигнала...");
        return -1;
    }
    printer("Установка прошла успешна!");

    int stuctureLength = sizeof(sockaddr);

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        printer("Ошибка: Возникли проблемы при связывании сокета...");
        return -1;
    }
    printer("Связывание прошло успешно!");
    printer("Ждём сообщения");

    message = "TCP Соединение успешно создано!";

    HANDLE tcpSender = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)tcpHandler, NULL, NULL, NULL);

    while (true) {
        recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&other_addr, &stuctureLength);
        std::string answer = std::string(buffer);
        if (answer == "R") {
            sendto(sock, message.c_str(), message.size(), 0, (sockaddr*)&other_addr, sizeof(other_addr));
        }
        if (answer == "C") {
            sendto(sock, "OK", 3, 0, (sockaddr*)&other_addr, sizeof(other_addr));    
        }
    } 

    closesocket(sock);
    WSACleanup();
    return 0;
}

int initWS() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

int setSocketOption() {
    char broadcast = '1';
    return setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
}

void printer(std::string msg) {
    std::cout << msg << std::endl;
}

void clientHandler(int index) {
    send(clients[index], "Ас-саляму алейкум", 64, 0);
    recv(clients[index], buffer, BUFFER_SIZE, 0);
    std::cout << buffer;
}

void tcpHandler() {
    SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind(tcp_sock, (sockaddr*)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
    {
        std::cout << "Ошибка: Возникли проблемы при связывании сокета...";
    }
    if (listen(tcp_sock, SOMAXCONN) != 0) {
        std::cout << "Ошибка: Прослушка не удалась";
    }
    char ipAddress[256];
    for (int i = 0; i < 100; i++) {
        SOCKET newClient = accept(tcp_sock, NULL, NULL);
        send(newClient, message.c_str(), message.size(), 0);
        std::cout << "Клиент: подключился\n";
        clients[i] = newClient;
        counter++;
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)(i), NULL, NULL);
    }
}
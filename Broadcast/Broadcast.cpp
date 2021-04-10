// Broadcast.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
#include <iostream>
#include <WS2tcpip.h>
#include <algorithm>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

// Константы
#define PORT 2620
#define BUFFER_SIZE 8192


// Методы
int initWS();
int setSocketOption();
void printer(std::string msg);
void tcpHandler();
void game();

// Переменные
int counter = 0;
std::string message;
char buffer[BUFFER_SIZE];
bool running = true;
bool start = false;
SOCKET sock;
sockaddr_in local_addr, other_addr;
SOCKET clients[10];
int score[10];

struct Quest {
public:
    Quest(std::string q, int a) {
        quest = q;
        answer = a;
    }
    std::string quest;
    int answer;
};

std::vector<Quest> question = {
    Quest("Длина китайской стены", 21196),
    Quest("Год смерти Иосифа Сталина", 1953)
    //Quest("Дата окончания 1 мировой войны", 1918),
    //Quest("Год развала СССР", 1991),
    //Quest("Расстояние от Земли до Луны", 384400)
};

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
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)game, NULL, NULL, NULL);

    while (running) {
        recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&other_addr, &stuctureLength);
        std::string answer = std::string(buffer);
        if (answer == "R") {
            sendto(sock, (char*)&counter, sizeof(int), 0, (sockaddr*)&other_addr, sizeof(other_addr));
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
    recv(clients[index], buffer, BUFFER_SIZE, 0);
    if (std::string(buffer) == "S" && counter > 0 && counter < 11) {
        start = true;
        for (int i = 0; i < counter; i++) {
            if (i == index) continue;
            send(clients[i], "START", 6, 0);
        }
    }
}

struct Max {
public:
    Max(int index, int max) {
        this->index = index;
        this->max = max;
    }
    int index;
    int max;
};

bool mysort(Max i, Max j) {
    return (i.max < j.max);
}

void game() {
    int r = 0;
    bool first = true;
    std::vector<Max> maximus;
    while (true) {
        if (!start) continue;
        running = false;

        maximus.clear();

        r = rand() % question.size();

        if (first) {
            for (int i = 0; i < counter; i++) {
                send(clients[i], "OK", 3, 0);
            }
        }

        int answer[10];
        
        for (int i = 0; i < counter; i++) {
            send(clients[i], question[r].quest.c_str(), question[r].quest.size(), 0);
            recv(clients[i], (char*)&answer[i], sizeof(int), 0);
        }
        int max = 999999999;
        int index_max = 0;
        for (int j = 0; j < counter; j++) {
            max = abs(question[r].answer - answer[j]);
            if (max == 0) score[j] += 2;
            maximus.push_back(Max(j, max));    // [2] [1] [1]  -> [1] [1] [1] [2] [2] [3]
            index_max = j;
        }

        std::sort(maximus.begin(), maximus.end(), mysort);
        if (maximus[0].max == 0) {
            send(clients[maximus[0].index], "You win son!", 13, 0);
            score[maximus[0].index] += 1;
            for (int x = 1; x < maximus.size(); x++) {

                if (maximus[0].max == maximus[x].max) {
                    send(clients[maximus[x].index], "You win son!", 13, 0);
                    score[maximus[x].index] += 1;
                }
                else {
                    send(clients[maximus[x].index], "You lose fckn slave", 20, 0);
                }

            }
        }
        else {
            send(clients[maximus[0].index], "Ты был близок", 13, 0);
            score[maximus[0].index] += 1;
            for (int x = 1; x < maximus.size(); x++) {

                if (maximus[0].max == maximus[x].max) {
                    send(clients[maximus[x].index], "Ты был близок", 13, 0);
                    score[maximus[x].index] += 1;
                }
                else {
                    send(clients[maximus[x].index], "You lose fckn slave", 20, 0);
                }

            }
        }
        
        question.erase(question.begin() + r);
        first = false;
        if (question.size() <= 0) break;
    }

    for (int i = 0; i < counter; i++) {
        send(clients[i], "END", 4, 0);
        Sleep(1000);
        send(clients[i], (char*)&score[i], sizeof(int), 0);
    }
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
    for (int i = 0; i < 10; i++) {
        SOCKET newClient = accept(tcp_sock, NULL, NULL);
        std::cout << "Клиент: подключился\n";
        clients[i] = newClient;
        counter++;
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)(i), NULL, NULL);
    }
}


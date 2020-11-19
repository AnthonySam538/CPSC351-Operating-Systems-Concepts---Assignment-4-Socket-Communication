#include <iostream>
#include <thread>
#include <mutex>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

mutex mtx;

void sendMessage()
{
    mtx.lock();

    const char* sourceIP = "127.0.0.1";
    const char* destinationIP = "127.0.0.1";
    sockaddr_in source;
    sockaddr_in destination;
    WSAData data;

    //Initialize the Windows Socket
    WSAStartup(MAKEWORD(2, 2), &data);

    //Create our sender socket to send messages
    SOCKET mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //Bind the socket to the specified IP address
    source.sin_family = AF_INET;
    inet_pton(AF_INET, sourceIP, &source.sin_addr.s_addr);
    source.sin_port = htons(0);

    destination.sin_family = AF_INET;
    inet_pton(AF_INET, destinationIP, &destination.sin_addr.s_addr);
    destination.sin_port = htons(3514);

    bind(mySocket, (sockaddr*)&source, sizeof(source));

    //The rest of the program
    const short maximumMessageLength = 2048;
    const char terminationString[] = "--exit";
    char myMessage[maximumMessageLength];

    while (true)
    {
        cout << "Your message (enter \"--exit\" to exit the program): ";
        mtx.unlock();
        cin.getline(myMessage, maximumMessageLength);

        mtx.lock();
        //if the user did not enter the termination string
        if (strcmp(myMessage, terminationString))
            sendto(mySocket, myMessage, strlen(myMessage), 0, (sockaddr*)&destination, sizeof(destination));
        //if the user entered the termination string
        else
        {
            cout << "Terminating the program...";

            closesocket(mySocket);
            WSACleanup();

            mtx.unlock();

            return;
        }
    }
}

//void receiveMessage()
//{}

int main()
{
    thread sendThread(sendMessage);
    sendThread.join();

    return 0;
}
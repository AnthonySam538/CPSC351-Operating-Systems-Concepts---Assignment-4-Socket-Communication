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

    const char* destinationIP = "127.0.0.1";
    const short destinationPort = 3514;
    sockaddr_in destination;
    WSAData wsadata;
    const short maximumMessageLength = 2048;
    char myMessage[maximumMessageLength];
    const char terminationString[] = "-disconnect";

    //Initialize a Windows Socket
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    //Create our sender socket to send messages
    SOCKET senderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //Bind the socket to the specified IP address
    destination.sin_family = AF_INET;
    inet_pton(AF_INET, destinationIP, &destination.sin_addr.s_addr);
    destination.sin_port = htons(destinationPort);
    bind(senderSocket, (sockaddr*)&destination, sizeof(destination));

    mtx.unlock();

    //The rest of the program
    while (true)
    {
        cout << "Say something (Enter in \"-disconnect\" to disconnect): ";
        cin.getline(myMessage, maximumMessageLength);

        //if the user did not enter the termination string
        if (strcmp(myMessage, terminationString))
        {
            sendto(senderSocket, myMessage, strlen(myMessage), 0, (sockaddr*)&destination, sizeof(destination));
            cout << "You (" << destinationIP << ':' << destinationPort << ") sent: " << myMessage << endl;
        }
        //if the user entered the termination string
        else
        {
            cout << "Terminating the program...";

            closesocket(senderSocket);
            WSACleanup();

            exit(0);
        }
    }
}

void receiveMessage()
{
    mtx.lock();

    const char* sourceIP = "127.0.0.1";
    const unsigned short port = 3515;
    sockaddr_in receiverAddress;
    int receiverAddressSize = sizeof(receiverAddress);
    WSADATA wsaData;
    const short maximumMessageLength = 2048;
    char receivedMessage[maximumMessageLength];

    //Initialize the Windows Socket
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //Create our receiver socket to receive messages
    SOCKET receiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //Bind the receiver socket to the specified IP address
    receiverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, sourceIP, &receiverAddress.sin_addr.s_addr);
    receiverAddress.sin_port = htons(port);
    bind(receiverSocket, (sockaddr*)&receiverAddress, sizeof(receiverAddress));

    mtx.unlock();

    while (true)
    {
        //Call the recvfrom function to receive messages
        recvfrom(receiverSocket, receivedMessage, maximumMessageLength, 0, (sockaddr*)&receiverAddress, &receiverAddressSize);

        mtx.lock();

        cout << endl << sourceIP << ':' << port << " sent: " << receivedMessage << endl;

        mtx.unlock();

        ////Close the socket when finished receiving datagrams
        //cout << "Finished receiving. Closing socket.\n";
        //closesocket(receiverSocket);
        //WSACleanup();
    }
}

int main()
{
    thread sendThread(sendMessage);
    thread receiveThread(receiveMessage);

    sendThread.join();
    receiveThread.join();

    return 0;
}

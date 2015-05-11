/*
 * Communicator.cpp
 *
 *  Created on: 14 Aug 2013
 *      Author: thomas
 */

#include "Communicator.h"

Communicator::Communicator()
 : port(54322),
   outSocket(-1),
   inSocket(-1),
   header("BLAH"),
   heartbeat("HERP"),
   broadcast("wlan1")
{
    outSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int opt = 1;
    setsockopt(outSocket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port + 1);
    servaddr.sin_addr.s_addr = inet_addr(getBroadcastAddress().c_str());

    inSocket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    int on = 1;
    setsockopt(inSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bind(inSocket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
}

Communicator::~Communicator()
{
    close(outSocket);
    close(inSocket);
}

void Communicator::sendInfo(const std::string & info)
{
    int size = header.length() + info.length() + sizeof(int);

    unsigned char * data = (unsigned char *)calloc(size, sizeof(unsigned char));

    int * dataInt = (int *)data;
    dataInt[0] = size;

    std::string msg = header;
    msg.append(info);

    memcpy(&dataInt[1], msg.data(), msg.length());

    sendto(outSocket, data, size, 0, (struct sockaddr *) &servaddr, sizeof(servaddr));

    free(data);
}

std::string Communicator::getBroadcastAddress()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;

    getifaddrs(&ifap);

    for(ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if(ifa->ifa_addr->sa_family == AF_INET && !strcmp(ifa->ifa_name, broadcast.c_str()))
        {
#if defined __APPLE__ && defined __MACH__
            sa = (struct sockaddr_in *) ifa->ifa_broadaddr;
#else
            sa = (struct sockaddr_in *) ifa->ifa_ifu.ifu_broadaddr;
#endif
            std::string addr(inet_ntoa(sa->sin_addr));
            freeifaddrs(ifap);
            std::cout << "Got broadcast address: " << addr << std::endl;
            return addr;
        }
    }

    freeifaddrs(ifap);

    assert(false && "Can't get broadcast address");

    return "";
}

std::string Communicator::tryRecv()
{
    char buffer[65535];

    int recvSize = 0;

    if((recvSize = recv(inSocket, &buffer[0], 65535, 0)) > 0)
    {
        int size = ((int *)&buffer[0])[0];

        std::string msg = header;

        if(recvSize == size)
        {
            std::string testStr(&buffer[sizeof(int)], header.length());

            if(testStr.compare(header) == 0)
            {
                std::string retStr(&buffer[sizeof(int) + header.length()], recvSize - (sizeof(int) + header.length()));
                return retStr;
            }
        }
    }

    return "";
}

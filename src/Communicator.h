/*
 * Communicator.h
 *
 *  Created on: 14 Aug 2013
 *      Author: thomas
 */

#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_

#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <unistd.h>
#include <iostream>

class Communicator
{
    public:
        Communicator();

        virtual ~Communicator();

        void sendInfo(const std::string & info);

        std::string getBroadcastAddress();

        std::string tryRecv();

    private:
        int port;
        int outSocket, inSocket;
        struct sockaddr_in servaddr;
        const std::string header;
        const std::string heartbeat;
        const std::string broadcast;
};


#endif /* COMMUNICATOR_H_ */

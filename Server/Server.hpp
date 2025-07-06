#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <iostream>
#include "MyData.hpp"
#include "AgentConnection.hpp"

#pragma comment(lib, "Ws2_32.lib")

typedef struct ThreadArgs {
    SOCKET sock;
    BOOL    bIsToClient;
} ThreadArgs;
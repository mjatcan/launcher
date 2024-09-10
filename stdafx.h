#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <ShellAPI.h>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <streambuf>
#include <mutex>
#include "Packet.h"
#include "xorstr.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

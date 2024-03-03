#include "pch.h"
#include "ServerSocket.h"

// CServerSocket server;
CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket::CHelper CServerSocket::m_helper;   // 使用语法规则进行保护，类外面访问不了

CServerSocket* pserver = CServerSocket::getInstance();

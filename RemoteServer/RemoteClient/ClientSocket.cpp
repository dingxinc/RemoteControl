#include "pch.h"
#include "ClientSocket.h"

// CServerSocket server;
CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;   // 使用语法规则进行保护，类外面访问不了

CClientSocket* pclient = CClientSocket::getInstance();
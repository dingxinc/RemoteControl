#include "pch.h"
#include "ServerSocket.h"

// CServerSocket server;
CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket::CHelper CServerSocket::m_helper;   // ʹ���﷨������б�������������ʲ���

CServerSocket* pserver = CServerSocket::getInstance();

#include "pch.h"
#include "ClientSocket.h"

// CServerSocket server;
CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;   // ʹ���﷨������б�������������ʲ���

CClientSocket* pclient = CClientSocket::getInstance();
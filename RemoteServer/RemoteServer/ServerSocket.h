#pragma once
#include "pch.h"
#include "framework.h"

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == nullptr) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket() {
		if (m_socket == -1) return false;
		sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(9527);
        // 绑定
		if (bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) return false;
        // 监听
		if (listen(m_socket, 1) == -1) return false;  // 远程控制是 1 对 1
		return true;
	}

	bool AcceptClient() {
		sockaddr_in client_adr;
		int client_sz = sizeof(client_adr);
		m_client = accept(m_socket, (sockaddr*)&client_adr, &client_sz);
		if (m_client == -1) return false;
		return true;
	}

	int DealCommand() {
		if (m_client == -1) return false;
		char buffer[1024] = "";
		while (true) {
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) return -1;
			// TODO: 处理命令
		}
	}

	bool Send(const char* pData, int nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}

private:
	CServerSocket() {
		m_client = INVALID_SOCKET;   // 等价于 m_client == -1
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
	}

	~CServerSocket() {
		closesocket(m_socket);
		WSACleanup();
	}

	CServerSocket(const CServerSocket&) = delete;
	CServerSocket* operator=(const CServerSocket) = delete;

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) return FALSE;    // 初始化网络环境
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != nullptr) {
			CServerSocket* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;
		}
	}

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}

		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

	static CServerSocket* m_instance;
	static CHelper m_helper;
	SOCKET m_socket;         // 服务器套接字
	SOCKET m_client;
};

// extern CServerSocket server;   // 引用这个变量，全局可见


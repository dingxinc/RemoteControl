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
        // ��
		if (bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) return false;
        // ����
		if (listen(m_socket, 1) == -1) return false;  // Զ�̿����� 1 �� 1
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
			// TODO: ��������
		}
	}

	bool Send(const char* pData, int nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}

private:
	CServerSocket() {
		m_client = INVALID_SOCKET;   // �ȼ��� m_client == -1
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ����������������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) return FALSE;    // ��ʼ�����绷��
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
	SOCKET m_socket;         // �������׽���
	SOCKET m_client;
};

// extern CServerSocket server;   // �������������ȫ�ֿɼ�


#pragma once
#include "pch.h"
#include "framework.h"

class CPacket {
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
	}

	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nLength; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;  // 假设只有两个字节且就等于 FEFF，后面没有数据就会挂
				break;
			}
		}

		if (i + 4 + 2 + 2 > nSize) {   // 4：sLength  2：sCmd   2：sSum;
			nSize = 0;                 // 包数据可能不全，或者包头未能全部接收到
			return;  // 解析失败
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {     // 包未完全接收到，就返回，解析失败
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;   // i 表示当前用到哪了
		// 解析包数据
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);  // cmd sum
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		// 校验数据
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;  // head2 leghth4 data...
			return;
		}
		nSize = 0;   // nSize 只要等于 0 就是解析失败了，只要大于 0 就是解析成功了
	}

	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			sSum = pack.sSum;
		}
		return *this;
	}

	~CPacket() {}

public:
	WORD sHead;   // 包头 固定位 0xFEFF
	DWORD nLength;// 包长度 从控制命令开始，到和校验结束
	WORD sCmd;    // 控制命令
	std::string strData; // 包数据
	WORD sSum;    // 和校验
};

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

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_client == -1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);  // 接收数据包
			if (len <= 0) return -1;
			// TODO: 处理命令
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);  // 解析数据包
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);  // 将接收到的包数据解析完后，将后面的数据前移，保证 buffer 可用
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
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
	CPacket m_packet;
};

// extern CServerSocket server;   // 引用这个变量，全局可见


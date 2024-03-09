#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>

#pragma pack(push)   // 入栈，保存当前状态
#pragma pack(1)      // 设置字节对齐
class CPacket {
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
	}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {  // 打包
		sHead = 0xFEFF;
		nLength = nSize + 4; // 4 = cmd + sum
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else strData.clear();
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
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

	int Size() {   // 包数据的大小
		return nLength + 6;
	}

	const char* Data() {
		strOut.resize(nLength + 6);
		// strOut 本身就是一个缓冲区，我们让 pData 指向这个缓冲区，然后将包头，长度，数据，命令...逐个复制进去
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;   // 包头 固定位 0xFEFF
	DWORD nLength;// 包长度 从控制命令开始，到和校验结束
	WORD sCmd;    // 控制命令
	std::string strData; // 包数据
	std::string strOut;  // 整个包的数据
	WORD sSum;    // 和校验
};
#pragma pack(pop)    // 还原状态

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;   // 动作：点击、移动、双击
	WORD nButton;   // 按键：左键、右键、中键
	POINT ptXY;     // 坐标
} MOUSEEV, * PMOUSEEV;

std::string GetErrorInfo(int wsaErrCode);

class CClientSocket
{
public:
	static CClientSocket* getInstance() {
		if (m_instance == nullptr) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}

	bool InitSocket(const std::string strIpAddress) {
		if (m_socket != INVALID_SOCKET) CloseSocket();
		m_socket = socket(PF_INET, SOCK_STREAM, 0);
		TRACE("client socket：%d\r\n", m_socket);
		if (m_socket == -1) return false;
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(strIpAddress.c_str());
		serv_addr.sin_port = htons(9527);
		if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox("指定的IP地址不存在！");
			return false;
		}
		// 连接
		int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
		if (ret == -1) {
			AfxMessageBox("连接失败！！");
			TRACE("连接失败：%d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_socket == -1) return -1;
		char* buffer = m_buffer.data();
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_socket, buffer + index, BUFFER_SIZE - index, 0);  // 接收数据包
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
		if (m_socket == -1) return false;
		return send(m_socket, pData, nSize, 0) > 0;
	}

	bool Send(CPacket& pack) {
		TRACE("m_socket = %d\r\n", m_socket);
		if (m_socket == -1) return false;
		return send(m_socket, pack.Data(), pack.Size(), 0) > 0;  // 6 = cmd + self
	}

	bool GetFilePath(std::string strPath) {
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4)) {  // 当命令等于 2 的时候才获取文件信息
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& GetPacket() {
		return m_packet;
	}

	void CloseSocket() {
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

private:
	CClientSocket() {
		// m_client = INVALID_SOCKET;   // 等价于 m_client == -1
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
	}

	~CClientSocket() {
		closesocket(m_socket);
		WSACleanup();
	}

	CClientSocket(const CClientSocket&) = delete;
	CClientSocket* operator=(const CClientSocket) = delete;

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) return FALSE;    // 初始化网络环境
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != nullptr) {
			CClientSocket* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;
		}
	}

	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}

		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};

	static CClientSocket* m_instance;
	static CHelper m_helper;
	SOCKET m_socket;         // 服务器套接字
	std::vector<char> m_buffer;  // 缓冲区
	CPacket m_packet;
};

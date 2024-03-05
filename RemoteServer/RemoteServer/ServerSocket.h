#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)   // ��ջ�����浱ǰ״̬
#pragma pack(1)      // �����ֽڶ���
class CPacket {
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
	}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {  // ���
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
				i += 2;  // ����ֻ�������ֽ��Ҿ͵��� FEFF������û�����ݾͻ��
				break;
			}
		}

		if (i + 4 + 2 + 2 > nSize) {   // 4��sLength  2��sCmd   2��sSum;
			nSize = 0;                 // �����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
			return;  // ����ʧ��
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {     // ��δ��ȫ���յ����ͷ��أ�����ʧ��
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;   // i ��ʾ��ǰ�õ�����
		// ����������
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);  // cmd sum
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		// У������
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;  // head2 leghth4 data...
			return;
		}
		nSize = 0;   // nSize ֻҪ���� 0 ���ǽ���ʧ���ˣ�ֻҪ���� 0 ���ǽ����ɹ���
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

	int Size() {   // �����ݵĴ�С
		return nLength + 6;
	}

	const char* Data() {
		strOut.resize(nLength + 6);
		// strOut �������һ���������������� pData ָ�������������Ȼ�󽫰�ͷ�����ȣ����ݣ�����...������ƽ�ȥ
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

public:
	WORD sHead;   // ��ͷ �̶�λ 0xFEFF
	DWORD nLength;// ������ �ӿ������ʼ������У�����
	WORD sCmd;    // ��������
	std::string strData; // ������
	std::string strOut;  // ������������
	WORD sSum;    // ��У��
};
#pragma pack(pop)    // ��ԭ״̬

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

#define BUFFER_SIZE 4096

	int DealCommand() {
		if (m_client == -1) return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);  // �������ݰ�
			if (len <= 0) return -1;
			// TODO: ��������
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);  // �������ݰ�
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);  // �����յ��İ����ݽ�����󣬽����������ǰ�ƣ���֤ buffer ����
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

	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;  // 6 = cmd + self
	}

	bool GetFilePath(std::string strPath) {
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3)) {  // ��������� 2 ��ʱ��Ż�ȡ�ļ���Ϣ
			strPath = m_packet.strData; 
			return true;
		}
		return false;
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
	CPacket m_packet;
};

// extern CServerSocket server;   // �������������ȫ�ֿɼ�


#include "pch.h"
#include "ClientSocket.h"

// CServerSocket server;
CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;   // ʹ���﷨������б�������������ʲ���

CClientSocket* pclient = CClientSocket::getInstance();

/*
* �����ʵ�ַֿ�д��ԭ���ǣ����붨���ʵ�ֶ�д�� .h �ļ��� ����ô��ͬʱ������ .cpp �ļ�������� .h ʱ����Ϊ c/c++ ���õ�ͷ�ļ��Ǿ͵�չ��������к�����ʵ�־ͻ����
* ���ʱ�򣬾ͻ�����������ͬ�ķ��ţ������ֻ�� .h �ļ��ж��壬�� .cpp ��ʵ�ֵĻ����ȫ�ֱ���һ��Ψһ�ķ��š�
*/
std::string GetErrorInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}
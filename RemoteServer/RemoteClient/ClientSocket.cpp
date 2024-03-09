#include "pch.h"
#include "ClientSocket.h"

// CServerSocket server;
CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;   // 使用语法规则进行保护，类外面访问不了

CClientSocket* pclient = CClientSocket::getInstance();

/*
* 定义和实现分开写的原因是，加入定义和实现都写在 .h 文件中 ，那么当同时有两个 .cpp 文件引用这个 .h 时，因为 c/c++ 引用的头文件是就地展开，如果有函数的实现就会编译
* 这个时候，就会生成两个相同的符号，而如果只在 .h 文件中定义，在 .cpp 中实现的活，就在全局编译一个唯一的符号。
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
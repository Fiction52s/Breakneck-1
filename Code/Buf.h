#ifndef __BUF_H__
#define __BUF_H__
#include <boost\thread.hpp>


struct Buf
{
	char buf[10000]; //no checks for boundaries atm
	int byteIndex;
	void Send(int x);
	void Send(double x);
	void Send(bool x);
	void Send(float x);
	void Send(unsigned char x);
	void Send(char *buf, int size);
	static void ThreadedBufferWrite(Buf *testBuf, std::ofstream *p_of );
	void SetRecOver(bool b);
	boost::mutex mut;
	bool recOver;
	boost::mutex overMut;
};

#endif
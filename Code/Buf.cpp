#include "Buf.h"
#include "GameSession.h"
#include <sstream>
#include <fstream>

using namespace std;
using namespace sf;

void Buf::ThreadedBufferWrite(Buf *testBuf, std::ofstream *p_of )
{
	ofstream &of = *p_of;
	int over;
	while (true)
	{

		testBuf->mut.lock();
		if (testBuf->byteIndex > 0)
		{
			//cout << session->totalGameFrames << " writing " << testBuf.byteIndex << " characters" << endl;
			of.write((char*)testBuf->buf, testBuf->byteIndex);
			testBuf->byteIndex = 0;
		}
		testBuf->mut.unlock();


		testBuf->overMut.lock();
		over = testBuf->recOver;
		testBuf->overMut.unlock();

		if (over)
			break;
	}
}

void Buf::Send(int x)
{
	int size = sizeof(x);

	mut.lock();

	std::memcpy(&(buf[byteIndex]), &x, size);
	byteIndex += size;

	/*buf[byteIndex] = '\n';
	++byteIndex;*/

	mut.unlock();
}

void Buf::Send(double x)
{
	int size = sizeof(x);

	mut.lock();

	std::memcpy(&(buf[byteIndex]), &x, size);
	byteIndex += size;

	/*buf[byteIndex] = '\n';
	++byteIndex;*/

	mut.unlock();
}

void Buf::Send(bool x)
{
	int size = sizeof(x);

	mut.lock();

	std::memcpy(&(buf[byteIndex]), &x, size);
	byteIndex += size;

	/*buf[byteIndex] = '\n';
	++byteIndex;*/

	mut.unlock();
}

void Buf::Send(float x)
{
	int size = sizeof(x);

	mut.lock();

	std::memcpy(&(buf[byteIndex]), &x, size);
	byteIndex += size;

	/*buf[byteIndex] = '\n';
	++byteIndex;*/

	mut.unlock();
}

void Buf::Send(unsigned char x)
{
	int size = sizeof(x);

	mut.lock();

	std::memcpy(&(buf[byteIndex]), &x, size);
	byteIndex += size;

	/*buf[byteIndex] = '\n';
	++byteIndex;*/

	mut.unlock();
}

void Buf::Send(char *buf, int bytes)
{
	mut.lock();

	std::memcpy(&(buf[byteIndex]), buf, bytes);
	byteIndex += bytes;

	mut.unlock();
}

void Buf::SetRecOver( bool b )
{
	overMut.lock();
	recOver = b;
	overMut.unlock();
}
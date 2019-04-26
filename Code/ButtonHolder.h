#ifndef __BUTTONHOLDER_H__
#define __BUTTONHOLDER_H__

struct ButtonHolder
{
	ButtonHolder(int holdLength);
	void Reset();
	int holdFrame;
	int holdLength;
	void Update(bool held);
	bool IsHoldComplete();
};

#endif
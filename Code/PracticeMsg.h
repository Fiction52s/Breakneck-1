#ifndef __PRACTICE_MSG_H__
#define __PRACTICE_MSG_H__

#include "Input.h"


struct PracticeMsg
{
	COMPRESSED_INPUT_TYPE input;
	int frame;

	PracticeMsg()
	{
		Clear();
	}
	
	void Clear()
	{
		frame = -1;
		input = 0;
	}
};

#endif
#ifndef __PRACTICE_MSG_H__
#define __PRACTICE_MSG_H__

#include "Input.h"
#include "VectorMath.h"
#include <SFML\Graphics.hpp>

struct PracticeMsgHeader
{
	enum MsgType
	{
		MSG_TYPE_START,
		MSG_TYPE_INPUT,
		MSG_TYPE_Count,
	};

	sf::Uint16 msgType;
	

	PracticeMsgHeader()
	{
		Clear();
	}

	void Clear()
	{
		msgType = 0;
		
	}
};

struct PracticeStartMsg
{
	PracticeMsgHeader header;
	sf::Uint32 skinIndex;
	sf::Uint32 upgradeField[8];

	PracticeStartMsg()
	{
		Clear();
	}

	void Clear()
	{
		header.Clear();

		skinIndex = 0;
		for (int i = 0; i < 8; ++i)
		{
			upgradeField[i] = 0;
		}
	}
};

struct PracticeInputMsg
{
	PracticeMsgHeader header;
	V2d desyncCheckPos;
	int desyncCheckAction;
	COMPRESSED_INPUT_TYPE input;
	int frame;
	

	PracticeInputMsg()
	{
		Clear();
	}
	
	void Clear()
	{
		header.Clear();
		desyncCheckAction = -1;
		desyncCheckPos.x = 0;
		desyncCheckPos.y = 0;
		frame = -1;
		input = 0;
	}
};

#endif
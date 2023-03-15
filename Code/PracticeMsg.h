#ifndef __PRACTICE_MSG_H__
#define __PRACTICE_MSG_H__

#include "Input.h"
#include "VectorMath.h"
#include "BitField.h"
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
	
	PracticeMsgHeader(sf::Uint16 mType);
	void Clear();
};

struct PracticeStartMsg
{
	PracticeMsgHeader header;
	sf::Uint32 skinIndex;
	sf::Uint32 upgradeField[8];

	PracticeStartMsg();
	void Clear();
	void SetUpgradeField(BitField &bf);
};

struct PracticeInputMsg
{
	PracticeMsgHeader header;
	V2d desyncCheckPos;
	int desyncCheckAction;
	COMPRESSED_INPUT_TYPE input;
	int frame;
	

	PracticeInputMsg();
	void Clear();
};

#endif
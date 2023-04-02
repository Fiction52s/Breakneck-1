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
		MSG_TYPE_INIT,
		MSG_TYPE_START,
		MSG_TYPE_INPUT,
		MSG_TYPE_SEQUENCE_CONFIRM,
		MSG_TYPE_STATE_CHANGE,
		MSG_TYPE_RACE_START,
		MSG_TYPE_INVITE,
		MSG_TYPE_ACCEPT_INVITE,
		MSG_TYPE_Count,
	};

	sf::Uint8 netType;//to separate it from ggpo
	sf::Uint16 msgType;
	
	PracticeMsgHeader(sf::Uint16 mType);
	void Clear();
};

struct PracticeStartMsg
{
	PracticeMsgHeader header;
	sf::Uint32 skinIndex;
	sf::Uint32 upgradeField[8];
	int numSyncBytes;
	int startFrame;

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

struct PracticeSequenceConfirmMsg
{
	PracticeMsgHeader header;
	int frame;

	PracticeSequenceConfirmMsg();
	void Clear();
};

struct PracticeStateChangeMsg
{
	PracticeMsgHeader header;
	int frame;
	int state;
	PracticeStateChangeMsg();
	void Clear();
};

struct PracticeRaceStartMsg
{
	PracticeMsgHeader header;
	sf::Uint64 raceClients[3]; //3 is max_players - 1 since the host doesn't need to send their own ID

	PracticeRaceStartMsg();
	void Clear();
};

struct PracticeInviteMsg
{
	PracticeMsgHeader header;
	//eventually have parameters for what type of game or custom invite etc.
	PracticeInviteMsg();
	void Clear();
};

struct PracticeAcceptInviteMsg
{
	PracticeMsgHeader header;
	//eventually have parameters for what type of game or custom invite etc.
	PracticeAcceptInviteMsg();
	void Clear();
};

#endif
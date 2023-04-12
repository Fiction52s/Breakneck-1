#include "PracticeMsg.h"
#include <assert.h>
#include "NetPacketTypes.h"
//#include "ggpo\network\udp_msg.h"

PracticeMsgHeader::PracticeMsgHeader(sf::Uint16 mType)
{
	netType = (sf::Uint8)PACKET_NET_TYPE_PRACTICE;
	msgType = mType;
	Clear();
}

void PracticeMsgHeader::Clear()
{
}

PracticeStartMsg::PracticeStartMsg()
	:header( PracticeMsgHeader::MSG_TYPE_START )
{
	Clear();
}

void PracticeStartMsg::Clear()
{
	header.Clear();

	startFrame = 0;

	skinIndex = 0;

	numSyncBytes = 0;

	for (int i = 0; i < 8; ++i)
	{
		upgradeField[i] = 0;
	}

	wantsToPlay = false;
}

void PracticeStartMsg::SetUpgradeField(BitField &bf)
{
	assert(bf.numFields == 8);
	for (int i = 0; i < bf.numFields; ++i)
	{
		upgradeField[i] = bf.optionField[i];
	}
}

PracticeInputMsg::PracticeInputMsg()
	:header(PracticeMsgHeader::MSG_TYPE_INPUT)
{
	Clear();
}

void PracticeInputMsg::Clear()
{
	header.Clear();
	desyncCheckAction = -1;
	desyncCheckPos.x = 0;
	desyncCheckPos.y = 0;
	frame = -1;
	input = 0;
}


PracticeSequenceConfirmMsg::PracticeSequenceConfirmMsg()
	:header( PracticeMsgHeader::MSG_TYPE_SEQUENCE_CONFIRM )
{
	Clear();
}

void PracticeSequenceConfirmMsg::Clear()
{
	header.Clear();
	frame = -1;
}

PracticeStateChangeMsg::PracticeStateChangeMsg()
	:header( PracticeMsgHeader::MSG_TYPE_STATE_CHANGE )
{
	Clear();
}

void PracticeStateChangeMsg::Clear()
{
	header.Clear();
	frame = -1;
	state = -1;
}


PracticeWantsToPlayMsg::PracticeWantsToPlayMsg()
	:header(PracticeMsgHeader::MSG_TYPE_WANTS_TO_PLAY)
{
	Clear();
}

void PracticeWantsToPlayMsg::Clear()
{
	header.Clear();
}

PracticeDoesntWantToPlayMsg::PracticeDoesntWantToPlayMsg()
	:header(PracticeMsgHeader::MSG_TYPE_DOESNT_WANT_TO_PLAY)
{
	Clear();
}

void PracticeDoesntWantToPlayMsg::Clear()
{
	header.Clear();
}



PracticeRaceStartRequestMsg::PracticeRaceStartRequestMsg()
	:header( PracticeMsgHeader::MSG_TYPE_RACE_START_REQUEST)
{
	Clear();
}

void PracticeRaceStartRequestMsg::Clear()
{
	header.Clear();
	for (int i = 0; i < 3; ++i)
	{
		raceClients[i] = 0;
	}
}


PracticeRaceStartResponseMsg::PracticeRaceStartResponseMsg()
	:header(PracticeMsgHeader::MSG_TYPE_RACE_START_RESPONSE)
{
	Clear();
}

void PracticeRaceStartResponseMsg::Clear()
{
	header.Clear();
	acceptedRequest = false;
}



PracticeRaceCancelMsg::PracticeRaceCancelMsg()
	:header(PracticeMsgHeader::MSG_TYPE_RACE_CANCEL)
{
	Clear();
}

void PracticeRaceCancelMsg::Clear()
{
	header.Clear();
}


#include "PracticeMsg.h"
#include <assert.h>

PracticeMsgHeader::PracticeMsgHeader()
{
	Clear();
}

void PracticeMsgHeader::Clear()
{
	msgType = 0;
}

PracticeStartMsg::PracticeStartMsg()
{
	Clear();
}

void PracticeStartMsg::Clear()
{
	header.Clear();

	skinIndex = 0;
	for (int i = 0; i < 8; ++i)
	{
		upgradeField[i] = 0;
	}
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
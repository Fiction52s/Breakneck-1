#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"

using namespace std;
using namespace sf;

RaceMode::RaceMode()
{
	endSeq = new FightEndSequence;
	endSeq->SetIDAndAddToAllSequencesVec();
	endSeq->Init();
}

RaceMode::~RaceMode()
{
	delete endSeq;
}

int RaceMode::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void RaceMode::StoreBytes(unsigned char *bytes)
{
	data.done = (int)done;
	//data.endSeqState = endSeq->state;
	//data.endSeqFrame = endSeq->frame;
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;
}

void RaceMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;
	//endSeq->state = data.endSeqState;
	//endSeq->frame = data.endSeqFrame;

	bytes += sizeof(MyData);
}

void RaceMode::Setup()
{
}

void RaceMode::StartGame()
{
}

HUD *RaceMode::CreateHUD()
{
	return NULL;
	//return new FightHUD;
}

bool RaceMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}

	int goalHitIndex = -1;
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = sess->GetPlayer(i);
		if (p != NULL)
		{
			if (p->hitGoal)
			{
				goalHitIndex = i;
				break;
			}
		}
	}

	if (goalHitIndex >= 0)
	{
		int placings[4];
		for (int i = 0; i < 4; ++i)
		{
			p = sess->GetPlayer(i);
			if (i == goalHitIndex)
			{
				placings[i] = 0;
			}
			else
			{
				if (p != NULL)
				{
					placings[i] = 1;
				}
				else
				{
					placings[i] = -1;
				}
			}
		}
		sess->SetMatchPlacings(placings[0], placings[1], placings[2], placings[3]);
		return true;
	}

	return false;
}

void RaceMode::EndGame()
{
	cout << "game over" << endl;
	endSeq->Reset();
	sess->SetActiveSequence(endSeq);
	sess->ActiveSequencesUpdate();
	done = true;
	//sess->RestartGame();
}

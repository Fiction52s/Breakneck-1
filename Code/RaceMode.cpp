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
	data.endSeqState = endSeq->state;
	data.endSeqFrame = endSeq->frame;
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;
}

void RaceMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;
	endSeq->state = data.endSeqState;
	endSeq->frame = data.endSeqFrame;

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

	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = sess->GetPlayer(i);
		if (p != NULL)
		{
			if (p->hitGoal)
			{
				sess->SetMatchPlacings(i);
				return true;
			}
		}
	}

	return false;
}

void RaceMode::EndGame()
{
	cout << "game over" << endl;
	endSeq->Reset();
	sess->SetActiveSequence(endSeq);
	sess->ActiveSequenceUpdate();
	done = true;
	//sess->RestartGame();
}

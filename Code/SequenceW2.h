#ifndef __SEQUENCEW2_H__
#define __SEQUENCEW2_H__

#include "Sequence.h"

//struct TextDisp;
//struct Conversation;
struct ConversationGroup;
struct CameraShot;
struct Barrier;



struct BirdBossScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TALK,
		Count
	};

	BirdBossScene(GameSession *owner);
	void SetupStates();
	void UpdateState();
};

struct CoyoteBossScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TALK,
		Count
	};

	CoyoteBossScene(GameSession *owner);
	void SetupStates();
	void UpdateState();
};


#endif
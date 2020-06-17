#include "GroundTrigger.h"
#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "StorySequence.h"
#include "Sequence.h"
#include "SequenceW1.h"
#include "MainMenu.h"
#include "SaveFile.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

GroundTrigger::GroundTrigger(ActorParams*ap)//Edge *g, double q, bool p_facingRight,const std::string &trigTypeStr)
	:Enemy(EnemyType::EN_GROUNDTRIGGER, ap )//, false, 1), ground(g), edgeQuantity(q),
	//facingRight( p_facingRight )//, trigType( tType )
{
	GroundTriggerParams *gtParams = (GroundTriggerParams*)ap;

	facingRight = gtParams->facingRight;
	trigType = GetTriggerType(gtParams->typeStr);

	sess = Session::GetSession();

	ts = sess->GetSizedTileset("Ship/shipleave_128x128.png");
	sprite.setTexture(*ts->texture);

	storySeq = NULL;
	gameSequence = NULL;
	

	switch (trigType)
	{
	case TRIGGER_HOUSEFAMILY:
		//gameSequence = new MonumentSeq(game);
		storySeq = new StorySequence;
		storySeq->Load("kinhouse");
		break;
	case TRIGGER_GETAIRDASH:
		sess->SetDrainOn(false);
		if (sess->GetPlayer(0)->HasUpgrade( 
			Actor::UpgradeType::UPGRADE_POWER_AIRDASH) )
		{
		}
		else
		{
			gameSequence = new GetAirdashPowerScene;//GetAirdashPowerSeq(game);
		}

		//storySeq = new StorySequence(game);
		//storySeq->Load("getairdash");
		break;
	case TRIGGER_DESTROYNEXUS1:
		gameSequence = new NexusCore1Seq;
		break;
	case TRIGGER_CRAWLERATTACK:
		gameSequence = new CrawlerAttackSeq;
		break;
	case TRIGGER_TEXTTEST:
		gameSequence = new TextTestSeq;
		//storySeq = new StorySequence(game);
		//storySeq->Load("kinhouse");
		//gameSequence = new TextTestSeq(game);
		break;
	}

	SetOffGroundHeight(ts->tileHeight / 2.0 - 10);
	SetCurrPosInfo(startPosInfo);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);// / 2 );
	sprite.setPosition(startPosInfo.GetPositionF());
	sprite.setRotation(startPosInfo.GetGroundAngleDegrees());

	actionLength[IDLE] = 20;
	actionLength[FOUND] = 6;

	animFactor[IDLE] = 2;
	animFactor[FOUND] = 3;

	ResetEnemy();
}

GroundTrigger::~GroundTrigger()
{
	if (gameSequence != NULL)
	{
		delete gameSequence;
	}
}

void GroundTrigger::ResetEnemy()
{
	frame = 0;
	action = IDLE;

	switch (trigType)
	{
	case TRIGGER_GETAIRDASH:
	{
		if (sess->GetPlayer(0)->HasUpgrade(
			Actor::UpgradeType::UPGRADE_POWER_AIRDASH) )
		{
			action = DONE;
		}
		break;
	}
	}
	
	if( storySeq != NULL )
		storySeq->Reset();

	if( gameSequence != NULL )
		gameSequence->Reset();
}

void GroundTrigger::DirectKill()
{

}

TriggerType GroundTrigger::GetTriggerType(const std::string &typeStr)
{
	string testStr = typeStr;
	std::transform(testStr.begin(), testStr.end(), testStr.begin(), ::tolower);
	if (testStr == "housefamily")
		return TRIGGER_HOUSEFAMILY;
	else if (testStr == "getairdash")
	{
		return TRIGGER_GETAIRDASH;
	}
	else if (testStr == "destroynexus1")
	{
		return TRIGGER_DESTROYNEXUS1;
	}
	else if (testStr == "crawlerattack")
	{
		return TRIGGER_CRAWLERATTACK;
	}
	else if (testStr == "texttest")
	{
		return TRIGGER_TEXTTEST;
	}
	else
	{
		assert(0);
		return TRIGGER_NEXUSCORE1;
	}
		
}

void GroundTrigger::ProcessState()
{
	if (action == DONE)
	{
		return;
	}

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case FOUND:
			action = DONE;
			frame = 0;
			break;
		}
	}
}

void GroundTrigger::UpdateEnemyPhysics()
{
	Actor *player = sess->GetPlayer(0);
	Edge* ground = currPosInfo.GetEdge();
	double edgeQuantity = currPosInfo.GetQuant();

	if (player->ground == ground && action == IDLE)
	{
		if (ground->Normal().y == -1)
		{
			if (abs((player->edgeQuantity + player->offsetX) - edgeQuantity) < 5)
			{
				player->HandleGroundTrigger( this );
				//player->GroundTriggerPoint(edgeQuantity, facingRight);
				action = FOUND;
				frame = 0;
			}
		}
		else
		{
			if (abs(player->edgeQuantity - edgeQuantity) < 5)
			{
				player->HandleGroundTrigger(this);
				//player->GroundTriggerPoint(edgeQuantity, facingRight);
				action = FOUND;
				frame = 0;
			}
		}
	}
}


void GroundTrigger::EnemyDraw(sf::RenderTarget *target)
{
	if (action != DONE)
		target->draw(sprite);
}

void GroundTrigger::DrawMinimap(sf::RenderTarget *target)
{
}

void GroundTrigger::UpdateSprite()
{
	if (action == DONE)
		return;

	int f;
	switch (action)
	{
	case IDLE:
		f = frame / animFactor[IDLE];
		break;
	case FOUND:
		f = (frame / animFactor[FOUND]) + actionLength[IDLE];
		break;
	}
	sprite.setTextureRect(ts->GetSubRect(f));
	//sprite.setPosition( position.x, position.y );
}

void GroundTrigger::DebugDraw(sf::RenderTarget *target)
{
}
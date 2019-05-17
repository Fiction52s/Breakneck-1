#include "GroundTrigger.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "StorySequence.h"
#include "Sequence.h"
#include "MainMenu.h"
#include "SaveFile.h"

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

GroundTrigger::GroundTrigger(GameSession *owner, Edge *g, double q, bool p_facingRight,const std::string &trigTypeStr)
	:Enemy(owner, EnemyType::EN_GROUNDTRIGGER, false, 1), ground(g), edgeQuantity(q),
	facingRight( p_facingRight )//, trigType( tType )
{
	trigType = GetTriggerType(trigTypeStr);
	initHealth = 40;
	health = initHealth;

	double height = 128;
	ts = owner->GetTileset("Ship/shipleave_128x128.png", 128, height);
	sprite.setTexture(*ts->texture);

	V2d gPoint = g->GetPoint(edgeQuantity);
	storySeq = NULL;
	gameSequence = NULL;

	switch (trigType)
	{
	case TRIGGER_HOUSEFAMILY:
		gameSequence = new MonumentSeq(owner);
		//storySeq = new StorySequence(owner);
		//storySeq->Load("kinhouse");
		break;
	case TRIGGER_GETAIRDASH:
		owner->drain = false;
		if (owner->HasPowerUnlocked(Actor::POWER_AIRDASH))
		{
		}
		else
		{
			gameSequence = new GetAirdashPowerSeq(owner);
		}
		
		//storySeq = new StorySequence(owner);
		//storySeq->Load("getairdash");
		break;
	case TRIGGER_DESTROYNEXUS1:
		gameSequence = new NexusCore1Seq(owner);
		break;
	case TRIGGER_CRAWLERATTACK:
		gameSequence = new CrawlerAttackSeq(owner);
		break;
	}

	receivedHit = NULL;

	V2d gn = g->Normal();
	float angle = atan2(gn.x, -gn.y);

	position = gPoint - gn * (height / 2.0 - 10);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);// / 2 );
	sprite.setPosition(gPoint.x, gPoint.y);
	sprite.setRotation(angle / PI * 180);

	spawnRect = sf::Rect<double>(gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2);

	actionLength[IDLE] = 20;
	actionLength[FOUND] = 6;

	animFactor[IDLE] = 2;
	animFactor[FOUND] = 3;

	//action = IDLE;

	sprite.setPosition(position.x, position.y);

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
	health = initHealth;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;

	action = IDLE;

	switch (trigType)
	{
	case TRIGGER_GETAIRDASH:
	{
		if (owner->HasPowerUnlocked(Actor::POWER_AIRDASH))
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
	Actor *player = owner->GetPlayer(0);

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
#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Nexus.h"
#include "SequenceW1.h"
#include "MapHeader.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


Nexus::Nexus( ActorParams *ap)
	:Enemy(EnemyType::EN_NEXUS, ap )
{

	SetNumActions(A_Count);
	SetEditorActions(A_SITTING, A_SITTING, 0);
	double width;
	double height;

	switch (world)
	{
	case 1:
	case 2:
		width = 288;
		height = 320;
		ts_nexusOpen = sess->GetTileset("Enemies/General/Nexus/nexus_open_1296x904.png", 1296, 904);
		ts_nexusDestroyed = sess->GetTileset("Enemies/General/Nexus/nexus_destroyed_1296x904.png", 1296, 904);

		ts_mini = sess->mainMenu->GetTileset("HUD/Minimap/minimap_icons_64x64.png", 64, 64);
		ts_explosion = sess->GetTileset("Enemies/General/Goal/goal_w01_b_480x480_0.png", 480, 480);
		ts_explosion1 = sess->GetTileset("Enemies/General/Goal/goal_w01_b_480x480_1.png", 480, 480);
		ts_node1 = sess->GetTileset("Enemies/General/Goal/nexus_node_1_512x512.png", 512, 512);
		ts_node2 = sess->GetTileset("Enemies/General/Goal/nexus_node_2_512x512.png", 512, 512);
		explosionLength = 24;
		explosionAnimFactor = 3;
		explosionYOffset = 80;
		initialYOffset = 30;
		break;
	}

	if (world == 2)
	{
		sprite.setColor(Color::Green);
		nodeSprite.setColor(Color::Green);
	}

	sprite.setTexture(*ts_nexusOpen->texture);
	sprite.setTextureRect(ts_nexusOpen->GetSubRect( 0 ));
	
	miniSprite.setTexture(*ts_mini->texture);
	miniSprite.setTextureRect(ts_mini->GetSubRect(2));
	miniSprite.setScale(16, 16);

	switch (world)
	{
	case 0:
		miniSprite.setTextureRect(ts_mini->GetSubRect(5));
		break;
	case 1:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 2:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 3:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 4:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	case 5:
		miniSprite.setTextureRect(ts_mini->GetSubRect(4));
		break;
	}

	miniSprite.setOrigin(miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height);

	SetOffGroundHeight(904 / 2.0);
	//SetOffGroundHeight(906.0 - 600.0);
	//SetGroundOffset(575.0 - 648.0);

	SetCurrPosInfo(startPosInfo);

	

	//miniSprite.setPosition( position.x, position.y );
	miniSprite.setPosition(startPosInfo.GetPositionF());
	miniSprite.setRotation(startPosInfo.GetGroundAngleDegrees());

	nodeSprite.setTexture(*ts_node1->texture);
	nodeSprite.setTextureRect(ts_node1->GetSubRect(0));
	nodeSprite.setOrigin(nodeSprite.getLocalBounds().width / 2, nodeSprite.getLocalBounds().height / 2);// - initialYOffset);

	BasicRectHurtBodySetup(80, 100, currPosInfo.GetGroundAngleRadians(), V2d(0, 20), currPosInfo.GetPosition());
	//currHurtboxes->GetCollisionBoxes(0)->front().globalPosition = position;
	double angle = 0;

	angle = atan2(gn.x, -gn.y);

	SetNumActions(5);

	frame = 0;
	animationFactor = 7;

	insideSeq = new NexusCoreSeq;
	insideSeq->nexus = this;
	insideSeq->Init();

	action = A_SITTING;

	ResetEnemy();
}


Nexus::~Nexus()
{
	delete insideSeq;
}

void Nexus::ConfirmKill()
{
	dead = true;
	HandleNoHealth();
}

void Nexus::ResetEnemy()
{
	frame = 0;
	action = A_SITTING;
	insideSeq->Reset();
	SetHurtboxes(&hurtBody, 0);
	sprite.setTexture(*ts_nexusOpen->texture);

	nodeSprite.setTexture(*ts_node1->texture);
	nodeSprite.setTextureRect(ts_node1->GetSubRect(0));
	UpdateSprite();
}

void Nexus::StartInsideSeq()
{
	insideSeq->Reset();
	sess->SetActiveSequence(insideSeq);
}

void Nexus::ProcessState()
{
	if (action == A_KINKILLING)
	{
		if (frame == 1)
		{
			sess->cam.Ease(Vector2f(GetKillPos()), 1, 60, CubicBezier());
		}
		if (frame == 46 * 2)
		{
			action = A_EXPLODING;
			//exploding = true;
			//kinKilling = false;
			frame = 0;
		}
	}
	else if (action == A_EXPLODING)
	{
		if (frame == explosionLength * explosionAnimFactor)
		{
			action = A_DESTROYED;
		}
	}
}

void Nexus::Setup()
{
	Enemy::Setup();

	//sess->goalNodePos = GetKillPos();
}

V2d Nexus::GetKillPos()
{
	V2d nodePos = currPosInfo.GetPosition() + V2d(575.0 - 648.0, 150);
	return nodePos;
}

void Nexus::HandleNoHealth()
{
	sess->KillAllEnemies();
	frame = 0;
	sess->PlayerHitNexus( this, 0 );
	SetHurtboxes(NULL, 0);
	action = A_KINKILLING;
	numHealth = 1;
	dead = false;
}

void Nexus::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
	if (action < A_DESTROYED)
	{
		target->draw(nodeSprite);
	}
}

void Nexus::FinishDestruction()
{
	action = A_NEXUSDESTROYED;
	frame = 0;
	sprite.setTexture(*ts_nexusDestroyed->texture);
}

void Nexus::DrawMinimap(sf::RenderTarget *target)
{
	target->draw(miniSprite);
}

void Nexus::UpdateSprite()
{
	int trueFrame = 0;
	if (action == A_KINKILLING)
	{
		if (frame / 2 < 12)
		{
			trueFrame = 1;
		}
		else if (frame / 2 < 18)
		{
			trueFrame = 2;
		}
		else if (frame / 2 < 30)
		{
			trueFrame = 3;
		}
		else
		{
			trueFrame = 3;
		}
		nodeSprite.setTexture(*ts_node1->texture);
		nodeSprite.setTextureRect(ts_node1->GetSubRect(trueFrame));
		nodeSprite.setOrigin(nodeSprite.getLocalBounds().width / 2, nodeSprite.getLocalBounds().height / 2);


	}
	else if (action == A_EXPLODING)
	{
		trueFrame = frame / explosionAnimFactor;
		int numTiles = 16;//ts_explosion->GetNumTiles();
		if (trueFrame >= numTiles)
		{
			trueFrame -= numTiles;
			nodeSprite.setTexture(*ts_node2->texture);
			nodeSprite.setTextureRect(ts_node2->GetSubRect(trueFrame));
			nodeSprite.setOrigin(nodeSprite.getLocalBounds().width / 2, nodeSprite.getLocalBounds().height / 2);
		}
		else
		{
			nodeSprite.setTexture(*ts_node1->texture);
			nodeSprite.setTextureRect(ts_node1->GetSubRect(trueFrame));
			nodeSprite.setOrigin(nodeSprite.getLocalBounds().width / 2, nodeSprite.getLocalBounds().height / 2);
		}
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
	}

	nodeSprite.setPosition(Vector2f(GetKillPos() ));
	nodeSprite.setRotation(currPosInfo.GetGroundAngleDegrees());

	//SetOffGroundHeight(906.0 - 600.0);
	//SetGroundOffset(575.0 - 648.0);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 - 50);//sprite.getLocalBounds().height - 50);
	sprite.setPosition(currPosInfo.GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}
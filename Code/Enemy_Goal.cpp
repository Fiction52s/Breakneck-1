#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "GoalExplosion.h"
#include "Enemy_Goal.h"
#include "MapHeader.h"
#include "ActorParamsBase.h"
#include "Actor.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


Goal::Goal( ActorParams *ap )
	:Enemy( EnemyType::EN_GOAL, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(A_SITTING, A_SITTING, 0);

	explosionAnimFactor = 3;
	explosionLength = 16;
	ts_mini = sess->mainMenu->GetSizedTileset("HUD/Minimap/minimap_icons_64x64.png");

	//world

	ts = sess->ts_goal;
	ts_crack = sess->ts_goalCrack;
	ts_explosion = sess->ts_goalExplode;

	int w = 1;

	explosionYOffset = 80;
	initialYOffset = 30;

	//switch (w)
	//{
	//case 1:
	//	explosionYOffset = 80;
	//	initialYOffset = 30;
	//	break;
	//case 2:
	//default:
	//	explosionYOffset = 0;
	//	initialYOffset = 0;
	//	break;
	//}

	SetOffGroundHeight(180);//160);//128);
	SetCurrPosInfo(startPosInfo);

	BasicRectHurtBodySetup(80 + 20, 100 + 30, currPosInfo.GetGroundAngleRadians(), V2d(0, 20 + 30), GetPosition());

	frame = 0;
	animationFactor = 7;

	//clean this up 
	if (sess->IsSessTypeGame())
	{
		sess->goal = this;
	}

	miniSprite.setTexture(*ts_mini->texture);
	miniSprite.setTextureRect(ts_mini->GetSubRect(2));

	miniSprite.setOrigin(miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height / 2);
	miniSprite.setPosition(currPosInfo.GetPositionF());
	miniSprite.setRotation(currPosInfo.GetGroundAngleDegrees());
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

	miniSprite.setTextureRect(ts_mini->GetSubRect(5));

	ResetEnemy();
}

Goal::~Goal()
{
}

//void Goal::SetMapGoalPos()
//{
//	sess->goal = this;
//}

V2d Goal::GetGoalNodePos()
{
	double nodeHeight = 104;
	V2d nodePos = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), nodeHeight);
	return nodePos;
}

V2d Goal::GetGoalNodePosFinal()
{
	float space = 78.f;
	V2d nodePos = GetGoalNodePos();
	V2d finalPos = V2d(nodePos.x, nodePos.y - space);
	return finalPos;
}


void Goal::ResetEnemy()
{
	frame = 0;
	action = A_SITTING;
	SetHurtboxes(&hurtBody, 0);
	UpdateSprite();
}

void Goal::ConfirmKill()
{
	//dead = true;
	HandleNoHealth();
}

void Goal::ProcessState()
{
	if (action == A_KINKILLING )
	{
		if (frame == 1)
		{
			
			//owner->cam.Ease(Vector2f(position), 1, 60, CubicBezier());
		}

		
		if (frame == 46 * 2)
		{
			action = A_EXPLODING;
			//exploding = true;
			//kinKilling = false;
			frame = 0;
		}
	}
	else if (action == A_EXPLODING )
	{
		if (frame == explosionLength * explosionAnimFactor)
		{
			action = A_DESTROYED;
		}
	}
}

void Goal::HandleNoHealth()
{
	sess->PlayerHitGoal(receivedHitPlayerIndex);
	sess->KillAllEnemies();
	frame = 0;
	
	SetHurtboxes(NULL, 0);
	action = A_KINKILLING;

	sess->StartGoalPulse(GetPositionF());
}

void Goal::UpdateSprite()
{
	ts = sess->ts_goal;
	ts_crack = sess->ts_goalCrack;
	ts_explosion = sess->ts_goalExplode;

	int trueFrame = 0;
	if (action == A_SITTING)
	{
		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(0));
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);//- initialYOffset);
	}
	else  if (action == A_KINKILLING)
	{
		if (frame / 2 < 12)
		{
			trueFrame = 0;
			
		}
		else if (frame / 2 < 18)
		{
			trueFrame = 1;
		}
		else if (frame / 2 < 30)
		{
			trueFrame = 2;
		}
		else
		{
			trueFrame = 3;
		}

		sprite.setTexture(*ts_crack->texture);
		sprite.setTextureRect(ts_crack->GetSubRect(trueFrame));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		
	}
	else if (action == A_EXPLODING)
	{
		trueFrame = frame / explosionAnimFactor;
		//int numTiles = //ts_explosion->GetNumTiles();
		sprite.setTexture(*ts_explosion->texture);
		sprite.setTextureRect(ts_explosion->GetSubRect(trueFrame));
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);// -explosionYOffset - initialYOffset);
	}
	else if (action == A_DESTROYED)
	{
		trueFrame = explosionLength - 1;
		//int numTiles = ts_explosion->GetNumTiles();
		sprite.setTexture(*ts_explosion->texture);
		sprite.setTextureRect(ts_explosion->GetSubRect(trueFrame));

		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);//- explosionYOffset - initialYOffset);
	}

	sprite.setPosition(currPosInfo.GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Goal::IsSlowed(int index)
{
	return false;
}

void Goal::EnemyDraw(sf::RenderTarget *target )
{
	ts = sess->ts_goal;
	ts_crack = sess->ts_goalCrack;
	ts_explosion = sess->ts_goalExplode;

	int trueFrame = 0;
	if (action == A_SITTING )
	{
		sprite.setTexture(*ts->texture);
	}
	else if (action == A_KINKILLING)
	{
		sprite.setTexture(*ts_crack->texture);
	}
	else if (action == A_EXPLODING || action == A_DESTROYED )
	{
		sprite.setTexture(*ts_explosion->texture);
	}

	DrawSprite(target, sprite);
	//target->draw( sprite );
}

void Goal::DrawMinimap( sf::RenderTarget *target )
{
	target->draw( miniSprite );
}

int Goal::GetNumStoredBytes()
{
	return sizeof(MyData) + sess->goalPulse->GetNumStoredBytes();
}

void Goal::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	sess->goalPulse->StoreBytes(bytes);
	bytes += sess->goalPulse->GetNumStoredBytes();
}

void Goal::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	sess->goalPulse->SetFromBytes(bytes);
	bytes += sess->goalPulse->GetNumStoredBytes();
}
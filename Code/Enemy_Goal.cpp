#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "GoalExplosion.h"
#include "Enemy_Goal.h"
#include "MapHeader.h"
#include "ActorParamsBase.h"

using namespace std;
using namespace sf;


Goal::Goal( ActorParams *ap )
	:Enemy( EnemyType::EN_GOAL, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(A_SITTING, A_SITTING, 0);

	double width;
	double height;

	switch (world)
	{
	case 1:
		width = 288;
		height = 320;
		ts = sess->GetSizedTileset("Goal/goal_w01_a_288x320.png");
		ts_mini = sess->GetSizedTileset("HUD/minimap_icons_64x64.png");
		ts_explosion = sess->GetTileset("Goal/goal_w01_b_480x480_0.png", 480, 480);
		ts_explosion1 = sess->GetTileset("Goal/goal_w01_b_480x480_1.png", 480, 480);
		explosionLength = 18;
		explosionAnimFactor = 3;
		explosionYOffset = 80;
		initialYOffset = 30;
		break;
	case 2:
	default:
		width = 288;
		height = 256;
		ts = sess->GetTileset("Goal/goal_w02_a_288x256.png", width, height);
		ts_mini = sess->GetTileset("HUD/minimap_icons_64x64.png", 64, 64);
		
		ts_explosion = sess->GetTileset("Goal/goal_w02_b_288x320.png", 288, 320);
		ts_explosion1 = NULL;
		explosionLength = 15;
		explosionAnimFactor = 3;
		explosionYOffset = 0;
		initialYOffset = 0;
		break;
	}

	SetOffGroundHeight(height / 2.0);

	BasicRectHurtBodySetup(80, 100, startPosInfo.GetGroundAngleRadians(), V2d(0, 20), GetPosition());

	frame = 0;
	animationFactor = 7;

	//clean this up 
	if (sess->IsSessTypeGame())
	{
		SetMapGoalPos();
	}

	miniSprite.setTexture(*ts_mini->texture);
	miniSprite.setTextureRect(ts_mini->GetSubRect(2));

	miniSprite.setOrigin(miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height);
	miniSprite.setPosition(startPosInfo.GetPositionF());
	miniSprite.setRotation(startPosInfo.GetGroundAngleDegrees());
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

	ResetEnemy();
}

Goal::~Goal()
{
}

void Goal::SetMapGoalPos()
{
	sess->hasGoal = true;
	sess->goalPos = GetPosition();

	double nodeHeight = 104;
	sess->goalNodePos = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), nodeHeight);
	float space = 78.f;
	sess->goalNodePosFinal = V2d(sess->goalNodePos.x, sess->goalNodePos.y - space);
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
	dead = true;
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

		//if (frame == 1)
		//{
		//	//owner->cam.manual = true;
		//	goalKillStartZoom = owner->cam.zoomFactor;
		//	goalKillStartPos = owner->cam.pos;
		//	//
		//}
		//if (frame <= 31 && frame > 1)
		//{
		//	CubicBezier bez(0, 0, 1, 1);
		//	float z = bez.GetValue((double)(frame - 1) / 30);

		//	Vector2f po = goalKillStartPos * (1.f - z) + Vector2f(owner->goalNodePos.x,
		//		owner->goalNodePos.y) * z;

		//	CubicBezier bez1(0, 0, 1, 1);
		//	float z1 = bez1.GetValue((double)(frame - 1) / 30);

		//	float zoom = goalKillStartZoom * (1.f - z) + 1.f * z;

		//	owner->cam.manual = true;
		//	
		//	//owner->cam.manual = true;
		//	//owner->cam.Set(po, zoom, 0);
		//	///Vector2f trueSpot = dropSpot + extra0;
		//	//owner->cam.Set( ( //trueSpot * 1.f/60.f + owner->cam.pos * 59.f/60.f ),
		//	//	1, 0 );

		//}
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
	sess->PlayerHitGoal(0);
	sess->KillAllEnemies();
	frame = 0;
	
	SetHurtboxes(NULL, 0);
	action = A_KINKILLING;

	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		game->goalPulse->StartPulse();
	}
}

void Goal::UpdateSprite()
{
	int trueFrame = 0;
	if (action == A_SITTING)
	{
		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(0));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);
	}
	else  if (action == A_KINKILLING)
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
			trueFrame = 4;
		}
		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(trueFrame));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);
	}
	else if (action == A_EXPLODING)
	{
		trueFrame = frame / explosionAnimFactor;
		int numTiles = ts_explosion->GetNumTiles();
		if (trueFrame >= numTiles)
		{
			trueFrame -= numTiles;
			sprite.setTexture(*ts_explosion1->texture);
			sprite.setTextureRect(ts_explosion1->GetSubRect(trueFrame));
		}
		else
		{
			sprite.setTexture(*ts_explosion->texture);
			sprite.setTextureRect(ts_explosion->GetSubRect(trueFrame));
		}
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
	}
	else if (action == A_DESTROYED)
	{
		trueFrame = explosionLength - 1;
		int numTiles = ts_explosion->GetNumTiles();
		if (trueFrame >= numTiles)
		{
			trueFrame -= numTiles;
			sprite.setTexture(*ts_explosion1->texture);
			sprite.setTextureRect(ts_explosion1->GetSubRect(trueFrame));
		}
		else
		{
			sprite.setTexture(*ts_explosion->texture);
			sprite.setTextureRect(ts_explosion->GetSubRect(trueFrame));
		}

		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - explosionYOffset - initialYOffset);
	}

	sprite.setPosition(startPosInfo.GetPositionF());
	sprite.setRotation(startPosInfo.GetGroundAngleDegrees());
}

void Goal::EnemyDraw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

void Goal::DrawMinimap( sf::RenderTarget *target )
{
	target->draw( miniSprite );
}
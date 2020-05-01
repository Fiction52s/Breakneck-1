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

void Goal::UpdateSpriteFromEditParams()
{
	//editparams always exists here
	if (editParams->posInfo.IsAerial())
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(editParams->GetFloatPos());
		sprite.setRotation(0);
	}
}

void Goal::UpdateOnPlacement( ActorParams *params )
{
	Enemy::UpdateOnPlacement(params);

	if (!params->posInfo.IsAerial())
	{
		BasicRectHurtBodySetup(80, 100, startPosInfo.GetGroundAngleRadians(), V2d(0, 20), GetPosition());
	}
}

Goal::Goal( ActorParams *ap )
	:Enemy( EnemyType::EN_GOAL, ap )
{
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
	UpdateOnPlacement(ap);

	frame = 0;
	animationFactor = 7;

	action = A_SITTING;

	SetNumActions(A_Count);
	SetEditorActions(A_SITTING, A_SITTING, 0);

	//clean this up 
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();

		miniSprite.setTexture(*ts_mini->texture);
		miniSprite.setTextureRect(ts_mini->GetSubRect(2));

		miniSprite.setOrigin(miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height);
		miniSprite.setPosition(startPosInfo.GetPositionF());
		miniSprite.setRotation(startPosInfo.GetGroundAngleDegrees());
		miniSprite.setScale(16, 16);

		switch (sess->mapHeader->envWorldType)
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

		game->hasGoal = true;
		game->goalPos = GetPosition();

		double nodeHeight = 104;
		game->goalNodePos = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), nodeHeight);
		float space = 78.f;
		game->goalNodePosFinal = V2d(game->goalNodePos.x, game->goalNodePos.y - space);
	}

	ResetEnemy();

	SetSpawnRect();
}

Goal::~Goal()
{
}

void Goal::ConfirmKill()
{
	dead = true;
	//owner->absorbParticles->Activate(owner->GetPlayer(0), 64, position);
	HandleNoHealth();
}

void Goal::ResetEnemy()
{
	frame = 0;
	action = A_SITTING;
	SetHurtboxes(&hurtBody, 0);
	UpdateSprite();
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

void Goal::EnemyDraw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

void Goal::DrawMinimap( sf::RenderTarget *target )
{
	target->draw( miniSprite );
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
		if( frame / 2 < 12 )
		{
			trueFrame = 1;
		}
		else if( frame / 2 < 18 )
		{
			trueFrame = 2;
		}
		else if( frame / 2 < 30 )
		{
			trueFrame = 3;
		}
		else
		{
			trueFrame = 4;
		}
		sprite.setTexture( *ts->texture );
		sprite.setTextureRect( ts->GetSubRect( trueFrame ) );
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - initialYOffset);
	}
	else if( action == A_EXPLODING )
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
	else if( action == A_DESTROYED )
	{
		trueFrame = explosionLength-1;
		int numTiles = ts_explosion->GetNumTiles();
		if (trueFrame >= numTiles )
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
#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "GoalExplosion.h"
#include "Enemy_Goal.h"

using namespace std;
using namespace sf;


Goal::Goal( GameSession *owner, Edge *g, double q )
		:Enemy( owner, EnemyType::EN_GOAL, false, 0 ), ground( g ), edgeQuantity( q ), dead( false )
{	
	double width = 288;
	double height = 256;
	ts = owner->GetTileset( "goal_w02_a_288x256.png", width, height );
	ts_mini = owner->GetTileset( "minimap_icons_64x64.png", 64, 64 );
	ts_explosion = owner->GetTileset( "goal_w02_b_288x320.png", 288, 320 );
	sprite.setTexture( *ts->texture );
	
	miniSprite.setTexture( *ts_mini->texture );
	miniSprite.setTextureRect( ts_mini->GetSubRect( 2 ) );
	miniSprite.setScale( 16, 16 );
	
	switch( owner->mh->envType )
	{
	case 0:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 5 ) );
		break;
	case 1:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 4 ) );
		break;
	case 2:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 4 ) );
		break;
	case 3:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 4 ) );
		break;
	case 4:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 4 ) );
		break;
	case 5:
		miniSprite.setTextureRect( ts_mini->GetSubRect( 4 ) );
		break;
	}

	miniSprite.setOrigin( miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height );
	

	V2d gPoint = g->GetPoint( edgeQuantity );


	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	//miniSprite.setPosition( position.x, position.y );
	miniSprite.setPosition( gPoint.x, gPoint.y );
	miniSprite.setRotation( angle / PI * 180 );


	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	currHurtboxes = new CollisionBody(1);
	CollisionBox hurtBody;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 40;
	hurtBody.rh = 40;
	hurtBody.globalAngle = angle;
	hurtBody.globalPosition = position;

	currHurtboxes->AddCollisionBox(0, hurtBody);
	SetHurtboxes(currHurtboxes, 0);
	//currHurtboxes->GetCollisionBoxes(0)->front().globalPosition = position;
	double angle = 0;
		
	angle = atan2( gn.x, -gn.y );
		
	
		//V2d( hitBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
		//hitBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	frame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 160 / 2, gPoint.y - 160 / 2, 160, 160 );

	//health = 1;
	//numHealth = 1;

	action = A_SITTING;
	//kinKillFrame = 0;
}

void Goal::ConfirmKill()
{
	dead = true;
	HandleNoHealth();
}

void Goal::ResetEnemy()
{
	frame = 0;
	action = A_SITTING;
	//numHealth = 1;
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
}

void Goal::ProcessState()
{
	if (action == A_KINKILLING )
	{
		if (frame == 1)
		{
			owner->cam.manual = true;
			owner->cam.Ease(Vector2f(position), 1, 60, CubicBezier());
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
		if (frame == 15 * 2)
		{
			action = A_DESTROYED;
		}
	}
}

void Goal::HandleNoHealth()
{
	owner->KillAllEnemies();
	frame = 0;
	owner->GetPlayer(0)->hitGoal = true;
	SetHurtboxes(NULL, 0);
	action = A_KINKILLING;

	owner->goalPulse->show = true;
}

void Goal::Draw(sf::RenderTarget *target )
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
	if( action == A_KINKILLING )
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
	}
	else if( action == A_EXPLODING )
	{
		trueFrame = frame / 2;
		sprite.setTexture( *ts_explosion->texture );
		sprite.setTextureRect( ts_explosion->GetSubRect( trueFrame ) );
	}
	else if( action == A_DESTROYED )
	{
		trueFrame = 14;
		sprite.setTexture( *ts_explosion->texture );
		sprite.setTextureRect( ts_explosion->GetSubRect( trueFrame ) );
	}
	
	
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
}
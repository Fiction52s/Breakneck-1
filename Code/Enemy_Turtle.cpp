#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Turtle.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Turtle::Turtle( ActorParams *ap )
	:Enemy( EnemyType::EN_TURTLE, ap )
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	bulletSpeed = 5;

	actionLength[NEUTRAL] = 1;
	actionLength[FIRE] = 15;
	actionLength[FADEIN] = 4;//60;
	actionLength[FADEOUT] = 17;//90;
	actionLength[INVISIBLE] = 30;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 2;
	animFactor[FADEIN] = 7;//15;
	animFactor[FADEOUT] = 5;
	animFactor[INVISIBLE] = 1;
	
	
	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::TURTLE, 12, 12, GetPosition(), V2d( 1, 0 ), PI * .5, 90, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	ts = GetSizedTileset("Enemies/W4/turtle_320x256.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d( 1, 0 );
	hitboxInfo->hType = HitboxInfo::ORANGE;

	trackTestCircle.setFillColor(Color(0, 255, 0, 100));
	trackTestCircle.setRadius(50);
	trackTestCircle.setOrigin(trackTestCircle.getLocalBounds().width / 2, trackTestCircle.getLocalBounds().height / 2);



	BasicCircleHitBodySetup(60, 0, V2d( 15, 0 ), V2d());
	BasicCircleHurtBodySetup(50, 0, V2d( 15, 0 ), V2d());

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Turtle::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Turtle::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Turtle::ResetEnemy()
{
	if( PlayerDir().x >= 0 )
		facingRight = true;
	else
		facingRight = false;
	

	action = NEUTRAL;
	frame = 0;
	
	data.playerTrackPos = V2d();
	data.fireCounter = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();	
}

void Turtle::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	//cout << "frame: " << frame << ", actionlength: " << blah << endl;
	V2d playerPos = sess->GetPlayerPos();

	if( frame == actionLength[action] * animFactor[action] )
	{
	switch( action )
	{
	case NEUTRAL:
		frame = 0;
		break;
	case FIRE:
		action = FADEOUT;
		frame = 0;
		break;
	case INVISIBLE:
		currPosInfo.position = data.playerTrackPos;

		if (PlayerDir().x >= 0)
			facingRight = true;
		else
			facingRight = false;

		action = FADEIN;
		frame = 0;
		break;
	case FADEIN:
		action = FIRE;
		frame = 0;
		break;
	case FADEOUT:
		action = INVISIBLE;
		frame = 0;
		
		data.playerTrackPos = playerPos;
		break;
	}
	}
}

void Turtle::ProcessState()
{
	ActionEnded();

	switch (action)
	{
	case NEUTRAL:
		if (PlayerDist(0) <= DEFAULT_DETECT_RADIUS)
		{
			action = FIRE;
			frame = 0;
		}
		//cout << "NEUTRAL";
		break;
	case FIRE:
		//cout << "FIRE";
		break;
	case INVISIBLE:
		//cout << "INVISIBLE";
		break;
	case FADEIN:
		//cout << "FADEIN";
		break;
	case FADEOUT:
		//cout << "FADEOUT";
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case FIRE:
		break;
	case INVISIBLE:
		break;
	case FADEIN:
		if (frame == 5 && slowCounter == 1)
		{
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
		}
		break;
	case FADEOUT:
		if (frame == 15 * animFactor[FADEOUT] && slowCounter == 1)
		{
			HitboxesOff();
			HurtboxesOff();
		}
		break;
	}

	if (action == FIRE && frame == 1 && slowCounter == 1)// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = GetPosition();
		launchers[0]->facingDir = normalize(sess->GetPlayerPos(0) - GetPosition());
		launchers[0]->Reset();
		launchers[0]->Fire();
		data.fireCounter = 0;
	}
}

void Turtle::UpdateEnemyPhysics()
{	
}

void Turtle::UpdateSprite()
{
	int trueFrame;
	switch( action )
	{
	case NEUTRAL:
		trueFrame = 0;
		break;
	case FIRE:
		trueFrame = frame / animFactor[FIRE] + 21;
		break;
	case INVISIBLE:

		trackTestCircle.setPosition(Vector2f(data.playerTrackPos));

		return;
		break;
	case FADEIN:
		trueFrame = frame / animFactor[FADEIN] + 17;
		break;
	case FADEOUT:
		trueFrame = frame / animFactor[FADEOUT];

		trackTestCircle.setPosition(Vector2f(sess->GetPlayerPos(0)));

		break;
	}

	//cout << "trueFrame: " << trueFrame << ", action: " << action << endl;
	IntRect ir = ts->GetSubRect( trueFrame );
	if( !facingRight )
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}

	//sprite.setScale( 2, 2 );
	sprite.setTextureRect( ir );
	sprite.setOrigin( sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2 );
	sprite.setPosition( GetPositionF());

}

void Turtle::EnemyDraw(sf::RenderTarget *target)
{
	if (action != INVISIBLE)
		DrawSprite(target, sprite);

	if (action == INVISIBLE || ( action == FADEOUT && frame >= actionLength[FADEOUT] * animFactor[FADEOUT] / 2) )
	{
		target->draw(trackTestCircle);
	}
}

int Turtle::GetNumStoredBytes()
{
	return sizeof(MyData) + GetNumStoredLauncherBytes();
}

void Turtle::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	StoreBytesForLaunchers(bytes);
	bytes += GetNumStoredLauncherBytes();
}

void Turtle::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	SetLaunchersFromBytes(bytes);
	bytes += GetNumStoredLauncherBytes();
}
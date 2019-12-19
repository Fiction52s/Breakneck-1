#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Specter.h"

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

SpecterTester::SpecterTester(Enemy *en)
	:enemy(en )
{

}

void SpecterTester::HandleEntrant(QuadTreeEntrant *qte)
{
	if (!enemy->specterProtected)
	{
		SpecterArea *sa = (SpecterArea*)qte;
		enemy->CheckTouchingSpecterField(sa);
	}
}

void SpecterTester::Query( Rect<double> &r )
{
	enemy->owner->specterTree->Query(this, r);
}

SpecterArea::SpecterArea( Specter *sp, sf::Vector2i &pos, int rad )
	:radius( rad ), specter( sp )
{
	double extra = 5;
	testRect.left = pos.x - rad - extra;
	testRect.top = pos.y - rad - extra;
	testRect.width = rad * 2 + extra * 2;
	testRect.height = rad * 2 + extra * 2;

	position = V2d(pos);

	//barrier.globalPosition = V2d( pos.x, pos.y );
	//barrier.isCircle = true;
	//barrier.rw = rad;
	//barrier.type 
}

void SpecterArea::HandleQuery( QuadTreeCollider * qtc )
{
	if( !specter->dead )
		qtc->HandleEntrant( this );
}

bool SpecterArea::IsTouchingBox( const sf::Rect<double> &r )
{
	/*CollisionBox b;
	b.isCircle = false;
	b.rw = r.width / 2;
	b.rh = r.height / 2;
	b.globalPosition = V2d( r.left + b.rw, r.top + b.rh );*/

	return r.intersects( testRect );//barrier.Intersects( b );//
}


Specter::Specter( GameSession *owner, bool p_hasMonitor, Vector2i &pos, int p_level )
	:Enemy( owner, EnemyType::EN_SPECTER, p_hasMonitor, 1 )//, myArea( this, pos, 400 )
{
	level = p_level;

	radius = 1000;

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

	myArea = new SpecterArea(this, pos, radius);

	//hopefully this doesnt cause deletion bugs
	owner->specterTree->Insert( myArea );

	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>(position.x - 200, position.y - 200,
		400, 400);

	animationFactor = 10;

	ts = owner->GetTileset( "Enemies/specter_256x256.png", 256, 256 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);
	hitBody->hitboxInfo = hitboxInfo;

	
	radCircle.setFillColor(Color(255, 0, 0, 100));
	radCircle.setRadius(radius);
	radCircle.setOrigin(radCircle.getLocalBounds().width / 2,
		radCircle.getLocalBounds().height / 2);
	radCircle.setPosition(position.x, position.y);

	ResetEnemy();
}

Specter::~Specter()
{
	delete myArea;
}

void Specter::ResetEnemy()
{
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	dead = false;
	frame = 0;
	receivedHit = NULL;
	
	UpdateHitboxes();
	UpdateSprite();
	
}

void Specter::ProcessState()
{
	if( frame == 11 * animationFactor )
	{
		frame = 0;
	}
}

void Specter::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	sprite.setPosition(position.x, position.y);
}

void Specter::EnemyDraw( sf::RenderTarget *target )
{
	target->draw(radCircle);
	DrawSpriteIfExists(target, sprite);
}

bool Specter::CanTouchSpecter() { return false; }
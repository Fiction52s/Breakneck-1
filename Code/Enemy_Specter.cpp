#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Specter.h"

using namespace std;
using namespace sf;

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
	enemy->sess->specterTree->Query(this, r);
}

SpecterArea::SpecterArea( Specter *sp )
	:specter(sp)
{
	UpdatePosition();
	SetRadius(1000);
}

void SpecterArea::SetRadius(int rad)
{
	radius = rad;
}

void SpecterArea::UpdatePosition()
{
	double extra = 5;
	position = specter->GetPosition();
	testRect.left = position.x - radius - extra;
	testRect.top = position.y - radius - extra;
	testRect.width = radius * 2 + extra * 2;
	testRect.height = radius * 2 + extra * 2;
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

	return IsBoxTouchingBox(r, testRect);//r.intersects( testRect );//barrier.Intersects( b );//
}

Specter::Specter( ActorParams *ap )
	:Enemy( EnemyType::EN_SPECTER, ap ), myArea( this )
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	radius = 1000;

	myArea.SetRadius(radius);
	//hopefully this doesnt cause deletion bugs

	animationFactor = 10;

	ts = sess->GetSizedTileset("Enemies/W6/specter_256x256.png");
	sprite.setTexture( *ts->texture );
	
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(48);
	BasicCircleHurtBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	
	radCircle.setFillColor(Color(255, 0, 0, 100));
	radCircle.setRadius(radius);
	radCircle.setOrigin(radCircle.getLocalBounds().width / 2,
		radCircle.getLocalBounds().height / 2);
	

	ResetEnemy();
}

Specter::~Specter()
{
}

void Specter::SetLevel(int lev)
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

void Specter::AddToWorldTrees()
{
	myArea.UpdatePosition();
	myArea.position = GetPosition();
	sess->specterTree->Insert(&myArea);
}

void Specter::ResetEnemy()
{
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

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
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	radCircle.setPosition(GetPositionF());
}

void Specter::EnemyDraw( sf::RenderTarget *target )
{
	target->draw(radCircle);
	DrawSprite(target, sprite);
}

bool Specter::CanTouchSpecter() { return false; }
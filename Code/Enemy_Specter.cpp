#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Specter.h"
#include "Actor.h"

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
	:Enemy( EnemyType::EN_SPECTER, ap )//, myArea( this )
{
	SetNumActions(A_Count);
	SetEditorActions(A_IDLE, A_IDLE, 0);

	//radius = 1000;

	//myArea.SetRadius(radius);

	actionLength[A_IDLE] = 11;
	actionLength[A_ATTACK] = 30;
	actionLength[A_RECOVER] = 60;

	animFactor[A_IDLE] = 10;
	animFactor[A_ATTACK] = 1;
	animFactor[A_RECOVER] = 1;

	ts = GetSizedTileset("Enemies/W6/specter_256x256.png");
	sprite.setTexture( *ts->texture );
	
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(13);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(48);
	BasicCircleHurtBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 12, 1, GetPosition(), V2d(1, 0), 0, 120, false);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->hType = HitboxInfo::MAGENTA;
	launchers[0]->Reset();

	
	//radCircle.setFillColor(Color(255, 0, 0, 100));
	//radCircle.setRadius(radius);
	//radCircle.setOrigin(radCircle.getLocalBounds().width / 2,
	//	radCircle.getLocalBounds().height / 2);
	
	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");


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

//void Specter::AddToWorldTrees()
//{
//	myArea.UpdatePosition();
//	myArea.position = GetPosition();
//	sess->specterTree->Insert(&myArea);
//}

void Specter::ResetEnemy()
{
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	dead = false;
	frame = 0;
	receivedHit.SetEmpty();
	
	UpdateHitboxes();
	UpdateSprite();
	
}

void Specter::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Specter::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Specter::ProcessState()
{
	double dist = PlayerDist();

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_IDLE:
			frame = 0;
			break;
		case A_ATTACK:
			action = A_RECOVER;
			frame = 0;
			break;
		case A_RECOVER:
			if (dist > DEFAULT_IGNORE_RADIUS)
			{
				action = A_IDLE;
				frame = 0;
			}
			else
			{
				action = A_ATTACK;
				frame = 0;
			}
			
			
			break;
		}
	}


	switch (action)
	{
	case A_IDLE:
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = A_ATTACK;
			frame = 0;
		}
		break;
	case A_ATTACK:
		
		break;
	case A_RECOVER:
		break;
	}

	switch (action)
	{
	case A_IDLE:
		break;
	case A_ATTACK:
		if (frame == 10 && slowCounter == 1)
		{
			
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Reset();

			V2d offset(80, 0);
			for (int i = 0; i < 6; ++i)
			{
				launchers[0]->position = GetPosition() + offset;
				launchers[0]->Fire();
				RotateCW(offset, PI / 3.0);
			}
		}
		break;
	case A_RECOVER:
		break;
	}
}

void Specter::UpdateSprite()
{

	int tile = 0;

	switch (action)
	{
	case A_IDLE:
		tile = frame / animFactor[A_IDLE];
		break;
	case A_ATTACK:
		tile = 0;
		break;
	case A_RECOVER:
		tile = 5;
		break;
	}
	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	//radCircle.setPosition(GetPositionF());
}

void Specter::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while (b != NULL)
	{
		BasicBullet *next = b->next;
		double angle = atan2(b->velocity.y, -b->velocity.x);
		sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
		b->launcher->DeactivateBullet(b);

		b = next;
	}

	Enemy::DirectKill();
}

void Specter::EnemyDraw( sf::RenderTarget *target )
{
	//target->draw(radCircle);
	DrawSprite(target, sprite);
}

bool Specter::CanTouchSpecter() { return false; }

int Specter::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Specter::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}

void Specter::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}
#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"
#include <sstream>
#include "Boss.h"
#include "KeyMarker.h"
#include "Enemy_Comboer.h"
#include "Shield.h"
#include "MainMenu.h"
#include "Enemy_CurveTurret.h"
#include "Wire.h"
#include "Enemy_Specter.h"
#include "Actor.h"
#include "AbsorbParticles.h"
#include "Bullet.h"

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



Enemy::Enemy(GameSession *own, EnemyType t, bool p_hasMonitor,
	int world, bool cuttable)
	:owner(own), prev(NULL), next(NULL), spawned(false),
	type(t), zone(NULL), dead(false),
	suppressMonitor(false), ts_hitSpack(NULL), keyShader(NULL)
{
	sess = Session::GetSession();

	if (CanTouchSpecter())
	{
		specterTester = new SpecterTester(this);
	}
	else
	{
		specterTester = NULL;
	}

	scale = 1.f;
	
	comboObj = NULL;
	hurtBody = NULL;
	hitBody = NULL;
	hitboxInfo = NULL;

	if (p_hasMonitor)
	{
		owner->numTotalKeys++;
	}

	pauseFrames = 0;
	ts_zoned = owner->GetTileset("Enemies/enemy_zone_icon_128x128.png", 128, 128);
	zonedSprite.setTexture(*ts_zoned->texture);
	
	genericDeathSound = owner->soundManager->GetSound("Enemies/kill");

	highResPhysics = false;
	numLaunchers = 0;
	launchers = NULL;
	currHitboxes = NULL;
	currHurtboxes = NULL;
	currShield = NULL;
	hurtShader = NULL;
	keyShader = NULL;
	EnemyParams *ep = own->eHitParamsMan->GetHitParams(t);
	ResetSlow();
	if (ep == NULL)
	{
		maxHealth = 1;
		//numHealth = 1;
	}
	else
	{
		maxHealth = ep->maxHealth;
		//numHealth = ep->maxHealth;
	}

	numHealth = maxHealth;
	
	if (cuttable)
	{
		cutObject = new CuttableObject;
	}
	else
	{
		cutObject = NULL;
	}
	hasMonitor = p_hasMonitor;
	if( world == 0 )
	{
		ts_hitSpack = NULL;
		ts_blood = NULL;
		keySprite = NULL;
		return;
	}


	auraColor = Color( 0, 0, 0, 0 );
	switch( t )
	{
	case EnemyType::EN_PATROLLER:
			auraColor = Color( 0x55, 0xbb, 0xff );
			break;
		case EnemyType::EN_CRAWLER:
			auraColor = Color( 0x77, 0xcc, 0xff );
			break;
		case EnemyType::EN_BASICTURRET:
			auraColor = Color( 0x66, 0x99, 0xff );
			break;
		case EnemyType::EN_FOOTTRAP:
			auraColor = Color( 0xaa, 0xcc, 0xff );
			break;
		case EnemyType::EN_BAT:
			auraColor = Color( 0x99, 0xff, 0xcc );
			break;
		case EnemyType::EN_STAGBEETLE:
			auraColor = Color( 0xaa, 0xff, 0x99 );
			break;
		case EnemyType::EN_POISONFROG:
			auraColor = Color( 0x66, 0xff, 0xee );
			break;
		case EnemyType::EN_CURVETURRET:
			auraColor = Color( 0x99, 0xff, 0x99 );
			break;
		default:
			break;
	}


	if( hasMonitor )
	{
		switch( world )
		{
		case 1:
			keyColor = COLOR_BLUE;
			break;
		case 2:
			keyColor = COLOR_GREEN;
			break;
		case 3:
			keyColor = COLOR_YELLOW;
			break;
		case 4:
			keyColor = COLOR_ORANGE;
			break;
		case 5:
			keyColor = COLOR_RED;
			break;
		case 6:
			keyColor = COLOR_MAGENTA;
			break;
		}
		/*keyColor.r = min( 255.f, 1.8f * keyColor.r );
		keyColor.g *= min(255.f, 1.8f * keyColor.g);
		keyColor.b *= min(255.f, 1.8f * keyColor.b);*/
		/*keyColor.r = (sf::Uint8)(floor( (float)(keyColor.r) * .1f + .5f ));
		keyColor.g = (sf::Uint8)(floor( (float)(keyColor.g) * .1f + .5f ));
		keyColor.b = (sf::Uint8)(floor( (float)(keyColor.b) * .1f + .5f ));*/
		keyColor = Color::White;
		
		//keyColor = Color::White;

		//cout << "doing the add monitor thing" << endl;
		keyShader = new Shader();
		if( !keyShader->loadFromFile( "Resources/Shader/key_shader.frag", sf::Shader::Fragment ) )
		{
			cout << "couldnt load enemy key shader" << endl;
			assert( false );
		}

		keyShader->setUniform("toColor", ColorGL(keyColor));//Glsl::Vec4( keyColor.r, keyColor.g, keyColor.b, keyColor.a ) );
		keyShader->setUniform("auraColor", ColorGL(auraColor));//Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a) );

	//	keyFrame = 0;
	//ts_key = owner->GetTileset( "key_w02_1_128x128.png", 128, 128 );
		stringstream ss;
		ss << "FX/key_w0" << world << "_1_128x128.png";
		ts_key = owner->GetTileset( ss.str(), 128, 128 );

		keySprite = new Sprite;
		keySprite->setTexture( *ts_key->texture );
	}
	else
	{
		cout << "doing the no monitor thing" << endl;
		ts_key = NULL;
		keyShader = NULL;
		keySprite = NULL;
	}


	stringstream ss;
	ss << "FX/hit_spack_" << world << "_128x128.png";

	ts_hitSpack = owner->GetTileset( ss.str(), 128, 128 );

	ss.clear();
	ss.str("");

	ss << "FX/w" << world << "_kill_fx_512x512.png";

	ts_killSpack = owner->GetTileset(ss.str(), 512, 512);
	assert(ts_killSpack != NULL);
	ss.clear();
	ss.str("");

	ss << "FX/fx_blood_" << world << "_256x256.png";

	ts_blood = owner->GetTileset( ss.str(), 256, 256 );


	hurtShader = new Shader();
	if( !hurtShader->loadFromFile( "Resources/Shader/enemyhurt_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "couldnt load enemy enemyhurt shader" << endl;
		assert( false );
	}
	hurtShader->setUniform( "toColor", Glsl::Vec4( Color::White.r, Color::White.g, Color::White.b, Color::White.a ));
	hurtShader->setUniform( "auraColor", Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a ) );
}

Enemy::~Enemy()
{
	if (specterTester != NULL)
		delete specterTester;

	if (numLaunchers > 0)
	{
		for (int i = 0; i < numLaunchers; ++i)
		{
			delete launchers[i];
		}
		delete[] launchers;
	}

	if (cutObject != NULL)
	{
		delete cutObject;
	}

	if (currShield != NULL)
		delete currShield;

	if (keyShader != NULL)
		delete keyShader;

	if (hurtShader != NULL)
		delete hurtShader;

	if (keySprite != NULL)
		delete keySprite;

	if (hurtBody != NULL)
		delete hurtBody;
	if (hitBody != NULL)
		delete hitBody;

	if (hitboxInfo != NULL)
		delete hitboxInfo;

	if (comboObj != NULL)
	{
		delete comboObj;
	}
}




bool Enemy::ReadPath(std::ifstream &is,
	int &pLen, std::list<Vector2i> &localPath)
{
	is >> pLen;

	for (int i = 0; i < pLen; ++i)
	{
		int localX, localY;
		is >> localX;
		is >> localY;
		localPath.push_back(Vector2i(localX, localY));
	}

	return true;
}

bool Enemy::ReadBool(std::ifstream &is,
	bool &b)
{
	int x;
	is >> x;
	b = x;
	return true;
}

void Enemy::PlayDeathSound()
{
	owner->ActivateSound( position, genericDeathSound);
}

void Enemy::SetZoneSpritePosition()
{
	zonedSprite.setPosition(position.x, position.y);
}

std::list<CollisionBox> * Enemy::GetComboHitboxes()
{
	if (comboObj != NULL)
	{
		CollisionBody *body = comboObj->enemyHitBody;
		if (body != NULL)
		{
			auto boxes = body->GetCollisionBoxes(comboObj->enemyHitboxFrame);
			return boxes;
		}
	}	

	return NULL;
}

void Enemy::DrawSpriteIfExists( sf::RenderTarget *target, sf::Sprite &spr )
{
	bool b = (owner->pauseFrames < 2 && pauseFrames < 2) || ( receivedHit == NULL && pauseFrames < 2 );
	if (hasMonitor && !suppressMonitor)
	{
		if ( b )
		{
			target->draw(spr, keyShader);
		}
		else
		{
			target->draw(spr, hurtShader);
		}
		target->draw(*keySprite);
	}
	else
	{
		if( b )
		{
			target->draw(spr);
		}
		else
		{
			target->draw(spr, hurtShader);
		}
	}
}

int Enemy::NumTotalBullets()
{
	return 0;
}

void Enemy::Reset()
{
	numHealth = maxHealth;
	
	if (cutObject != NULL)
		cutObject->Reset();
	ResetSlow();
	suppressMonitor = false;
	spawned = false;
	prev = NULL;
	next = NULL;
	spawnedByClone = false;
	currHitboxes = NULL;
	currHurtboxes = NULL;
	dead = false;
	currShield = NULL;
	pauseFrames = 0;

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->Reset();
	}

	ResetEnemy();

	UpdateHitboxes();
}

void Enemy::SetHitboxes(CollisionBody *cb, int frame)
{
	currHitboxes = cb;
	currHitboxFrame = frame;
}

void Enemy::SetHurtboxes(CollisionBody *cb, int frame)
{
	currHurtboxes = cb;
	currHurtboxFrame = frame;
}

bool Enemy::IsTouchingSpecterField( SpecterArea *sa )
{
	return WithinDistance(sa->position, position, sa->radius);
}

void Enemy::CheckTouchingSpecterField(SpecterArea *sa)
{
	specterProtected = IsTouchingSpecterField(sa);
}

bool Enemy::CanTouchSpecter()
{
	return true;
}

void Enemy::HandleQuery( QuadTreeCollider * qtc )
{
	//if( !spawned )
	qtc->HandleEntrant( this );
}

bool Enemy::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( spawnRect, r );//r.intersects( spawnRect );// 
}
	
void Enemy::DirectKill()
{
	owner->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_killSpack, position, true, 0, 10, 4, true);

	dead = true;

	numHealth = 0;
	HandleNoHealth();
	receivedHit = NULL;

	if (cutObject != NULL)
	{
		cutObject->SetCutRootPos(Vector2f(position.x, position.y));
	}
}

V2d Enemy::TurretSetup()
{
	for (int li = 0; li < 1; ++li)
	{
		launchers[li]->Reset();
		launchers[li]->Fire();
		BasicBullet *bb = launchers[li]->activeBullets;
		V2d finalPos;
		bool collide = true;
		while (launchers[li]->GetActiveCount() > 0)
		{
			launchers[li]->UpdatePrePhysics();


			launchers[li]->UpdatePhysics(0, true);

			if (bb->framesToLive == 0)
			{
				finalPos = bb->position;
				collide = false;
			}

			launchers[li]->UpdatePostPhysics();
		}

		if (collide)
		{
			finalPos = launchers[li]->def_pos;
		}

		launchers[li]->interactWithTerrain = false;

		return finalPos;
	}

	return V2d(0, 0);
}

bool Enemy::RightWireHitMe( CollisionBox p_hurtBox )
{
	if( owner->GetPlayer( 0 )->hasPowerRightWire )
	{
		Wire::WireCharge *charge = owner->GetPlayer( 0 )->rightWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = owner->GetPlayer( 0 )->wireChargeInfo;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}

	return false;
}

bool Enemy::LeftWireHitMe( CollisionBox p_hurtBox )
{
	if( owner->GetPlayer( 0 )->hasPowerLeftWire )
	{
		Wire::WireCharge *charge = owner->GetPlayer( 0 )->leftWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = owner->GetPlayer( 0 )->wireChargeInfo;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}

	return false;
}

void Enemy::SyncSpriteInfo(sf::Sprite &dest, sf::Sprite &source)
{
	dest.setTextureRect(source.getTextureRect());
	dest.setOrigin(source.getOrigin());
	dest.setRotation(source.getRotation());
	dest.setScale(source.getScale());
	dest.setPosition(source.getPosition());
}

void Enemy::Record( int enemyIndex )
{
	Buf & b = owner->testBuf;

	b.Send( enemyIndex );

	//note: in order for this to work I can't send any pointers to the buf
	//and expect it to work on the next run. just doesnt work

	//need to index the enemies in the list somehow. list changes all the time

	//b.Send( prev );
	//b.Send( next );
	//b.Send( spawned );
	//b.Send( receivedHit );
	//b.Send( 
}

void Enemy::RecordEnemy()
{
	//stub
}

void Enemy::CheckSpecters()
{
	specterProtected = false;
	if (CanTouchSpecter())
	{
		assert(specterTester != NULL);

		double extra = 5;
		sf::Rect<double> r;
		r.left = position.x - extra;
		r.top = position.y - extra;
		r.width = extra * 2;
		r.height = extra * 2;

		specterTester->Query(r);
	}
}

void Enemy::UpdatePrePhysics()
{
	if (pauseFrames > 0)
	{
		return;
	}
		

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePrePhysics();
	}

	if ( dead )
		return;


	CheckSpecters();

	receivedHit = NULL;

	ProcessState();
	if (currShield != NULL)
	{
		if (!currShield->ProcessState())
		{
			ShieldDestroyed(currShield);
			currShield = NULL;
		}
	}

	double len = length(position - owner->GetPlayer( 0 )->position );
	bool isFar = owner->GetPlayer(0)->EnemyIsFar(position);
	if (isFar)
	{
		numPhysSteps = NUM_STEPS;
	}
	else
	{
		numPhysSteps = NUM_MAX_STEPS;
		owner->GetPlayer(0)->highAccuracyHitboxes = true;
	}

	if (highResPhysics)
	{
		numPhysSteps = NUM_MAX_STEPS;
	}
	
}

bool Enemy::LaunchersAreDone()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		if (launchers[i]->activeBullets != NULL)
			return false;
	}

	return true;
}

void Enemy::UpdatePostPhysics()
{
	
	//cout << "suppress: " << (int)suppressMonitor << endl;
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePostPhysics();
	}

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdateSprites();
	}

	if (currShield != NULL)
	{
		if (currShield->pauseFrames > 0)
		{
			--currShield->pauseFrames;
			//if (UpdateAccountingForSlow()) //shield might have its own at some point
			//{
			//	
			//}
		}
	}

	if (pauseFrames > 0)
	{
		if (UpdateAccountingForSlow())
		{
			--pauseFrames;
		}
		return;
	}
	
	if (currShield != NULL)
	{
		if( currShield->pauseFrames == 0)
			currShield->ProcessHit();
	}
	else
	{
		ProcessHit();
	}
	

	if (numHealth == 0 && LaunchersAreDone()
		&& ( ( cutObject != NULL && !cutObject->active ) 
			|| cutObject == NULL && dead ) )
	{
		dead = true;
		owner->RemoveEnemy(this);
		HandleRemove();
		return;
	}

	if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			//keySprite.setTexture( *ts_key->texture );
			int fac = 5;
			int kFrame = owner->totalGameFrames % (16 * fac);
			keySprite->setTextureRect(ts_key->GetSubRect(kFrame / fac));
			keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
				keySprite->getLocalBounds().height / 2);
			keySprite->setPosition(position.x, position.y);
			keySprite->setColor(Color(255, 255, 255, 255));
		}
		UpdateSprite();
		if (currShield != NULL)
		{
			currShield->UpdateSprite();
		}

	}
	else if( cutObject != NULL )
		cutObject->UpdateCutObject( slowCounter );

	if (UpdateAccountingForSlow())
	{
		++frame;
		if (cutObject != NULL)
			cutObject->IncrementFrame();
		
		if (!dead)
		{
			FrameIncrement();
			if (currShield != NULL && currShield->pauseFrames == 0 )
			{
				currShield->FrameIncrement();
			}
		}
	}
}

void Enemy::CheckedMiniDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	if ( zone == NULL || ( zone != NULL && zone->active ) )
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			DrawMinimap(target);
		}
	}
}

void Enemy::CheckedZoneDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	if (zone != NULL &&  (zone->action == Zone::UNEXPLORED || (zone->action == Zone::OPENING && zone->frame <= 20 && !zone->reexplored) ) )// && !zone->active )
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
			ZoneDraw(target);
		}
	}
}

void Enemy::CheckedZoneUpdate(sf::FloatRect &rect)
{
	if (zone != NULL && !zone->active)
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
		}
	}
}


void Enemy::ProcessShieldHit()
{

}

void Enemy::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0 )
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor )
			{
				owner->keyMarker->CollectKey();
			}

			owner->PlayerConfirmEnemyKill(this);
			ConfirmKill();
		}
		else
		{
			owner->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		//receivedHit = NULL;
	}
}
void Enemy::MovePos(V2d &vel,
	int slowMult,
	int numPhysSteps)
{
	V2d movementVec = vel;
	movementVec /= slowMultiple * (double)numPhysSteps;
	position += movementVec;
}

void Enemy::ConfirmHitNoKill()
{
	assert(receivedHit != NULL);

	HitboxInfo::HitboxType hType = receivedHit->hType;
	if (hType == HitboxInfo::COMBO)
	{
		pauseFrames = 5;
	}
	else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
	{
		pauseFrames = 5;
	}
	else
	{
		owner->Pause(5);
		pauseFrames = 0;
	}
	
	HandleHitAndSurvive();
	owner->cam.SetRumble(.5, .5, 5);
	//owner->cam.SetRumble(3, 3, 5);
}

void Enemy::HandleNoHealth()
{
	PlayDeathSound();
}

void Enemy::ConfirmKill()
{
	assert(receivedHit != NULL);

	HitboxInfo::HitboxType hType = receivedHit->hType;
	if (hType == HitboxInfo::COMBO )
	{
		pauseFrames = 7;
		comboHitEnemy->ComboKill(this);
	}
	else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
	{
		pauseFrames = 7;
	}
	else
	{
		owner->Pause(7);
		pauseFrames = 0;
	}


	owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, ts_killSpack, position, true, 0, 10, 5, true);
	//owner->cam.SetRumble(1, 1, 7 );
	owner->cam.SetRumble(1, 1, 7);
	
	if (hasMonitor && !suppressMonitor)
	{
		owner->absorbDarkParticles->Activate(owner->GetPlayer(0), 1, position);
	}
	else
	{
		owner->absorbParticles->Activate(owner->GetPlayer(0), 6, position);
	}
	

	dead = true;

	HandleNoHealth();
	PlayDeathSound();

	if (cutObject != NULL)
	{
		cutObject->SetCutRootPos(Vector2f(position.x, position.y));

	}

	
}

void Enemy::SetupBodies(int numHurtboxes,
	int numHitboxes)
{
	if (numHurtboxes > 0)
	{
		hurtBody = new CollisionBody(numHurtboxes);
	}
	
	if (numHitboxes > 0)
	{
		hitBody = new CollisionBody(numHitboxes);
	}
}

void Enemy::AddBasicHurtCircle(double rad, int index)
{
	AddBasicHurtCircle(hurtBody, rad, index);
}

void Enemy::AddBasicHitCircle(double rad, int index)
{
	AddBasicHitCircle(hitBody, rad, index);
}

void Enemy::AddBasicHurtCircle( CollisionBody *bod, double rad, int index)
{
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = rad * scale;
	hurtBox.rh = rad * scale;
	bod->AddCollisionBox(index, hurtBox);
}


void Enemy::AddBasicHitCircle( CollisionBody *bod, double rad, int index)
{
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = rad * scale;
	hitBox.rh = rad * scale;
	bod->AddCollisionBox(index, hitBox);
}

void Enemy::AddBasicHitRect(CollisionBody *bod, double w, double h, int index)
{
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = false;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = w/2;
	hitBox.rh = h/2;
	bod->AddCollisionBox(index, hitBox);
}

void Enemy::AddBasicHurtRect(CollisionBody *bod, double w, double h, int index)
{
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hurt;
	hitBox.isCircle = false;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = w / 2;
	hitBox.rh = h / 2;
	bod->AddCollisionBox(index, hitBox);
}

void Enemy::ComboHit()
{
	//empty default
}

void Enemy::ComboKill(Enemy *e)
{
	//empty default
}

void Enemy::Draw(sf::RenderTarget *target)
{
	if (cutObject != NULL)
	{
		if (dead && cutObject->active )
		{
			cutObject->Draw(target);
		}
		else if( !dead )
		{
			EnemyDraw( target );
			if (currShield != NULL)
			{
				currShield->Draw(target);
			}
		}
	}
	else
	{
		EnemyDraw( target );
		if (currShield != NULL)
		{
			currShield->Draw(target);
		}
	}
}

void Enemy::UpdateZoneSprite()
{
	if (zone != NULL && zone->action == Zone::OPENING && zone->frame <= 20 )
	{
		float f = 1.f - zone->frame / 20.f;
		zonedSprite.setColor(Color(255, 255, 255, f * 255));
	}
	else
	{
		zonedSprite.setColor(Color::White);
	}

	int fr = (owner->totalGameFrames % (12 * 4 )) / 4;

	zonedSprite.setTextureRect(ts_zoned->GetSubRect(fr));
	zonedSprite.setOrigin(zonedSprite.getLocalBounds().width / 2,
		zonedSprite.getLocalBounds().height / 2);
	
}

void Enemy::ZoneDraw(sf::RenderTarget *target)
{
	target->draw(zonedSprite);
}



void Enemy::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			CircleShape cs;
			cs.setRadius(50);
			cs.setFillColor(Color::White);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
		else
		{
			CircleShape cs;
			cs.setRadius(40);
			cs.setFillColor(Color::Red);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
	}
}

void Enemy::DebugDraw(sf::RenderTarget *target)
{
	if (!dead)
	{
		for (int i = 0; i < numLaunchers; ++i)
		{
			launchers[i]->DebugDraw(target);
		}


		if( currHitboxes != NULL )
			currHitboxes->DebugDraw( currHitboxFrame, target);
		if( currHurtboxes != NULL )
			currHurtboxes->DebugDraw( currHurtboxFrame, target);
	}
}

void Enemy::UpdateHitboxes()
{
	BasicUpdateHitboxes();
	BasicUpdateHitboxInfo();
}

void Enemy::BasicUpdateHitboxes()
{
	if (hurtBody != NULL)
	{
		auto boxes = hurtBody->GetCollisionBoxes(0);
		if (!boxes->empty())
		{
			CollisionBox &hurtBox = boxes->front();
			hurtBox.globalPosition = position;
			//hurtBox.globalAngle = 0;
		}
	}

	if (hitBody != NULL)
	{
		auto boxes = hitBody->GetCollisionBoxes(0);
		if (!boxes->empty())
		{
			CollisionBox &hitBox = boxes->front();
			hitBox.globalPosition = position;
			//hitBox.globalAngle = 0;
		}
	}

	auto comboBoxes = GetComboHitboxes();
	if (comboBoxes != NULL)
	{
		for (auto it = comboBoxes->begin(); it != comboBoxes->end(); ++it)
		{
			(*it).globalPosition = position;
		}
	}
}

void Enemy::BasicUpdateHitboxInfo()
{
	if (hitboxInfo != NULL)
	{
		hitboxInfo->kbDir = owner->GetPlayerKnockbackDirFromVel();
	}
}

void Enemy::UpdatePhysics( int substep )
{
	if (pauseFrames > 0)
		return;
	
	bool validSubstep = (substep < numPhysSteps);

	if (validSubstep)
	{
		UpdatePreLauncherPhysics();
	}
	
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePhysics( substep );
	}

	if (dead || !validSubstep )
	{
		return;
	}

	UpdateEnemyPhysics();

	UpdateHitboxes();

	SlowCheck(0);

	if (currShield != NULL)
	{
		if( currShield->pauseFrames == 0 )
			currShield->CheckHit(owner->GetPlayer(0), type);
	}
	else
	{
		CheckHit(owner->GetPlayer(0), type);
	}

	if (CheckHitPlayer(0))
	{
	}
}

bool Enemy::IsSlowed( int index )
{
	Actor *player = owner->GetPlayer(index);
	return (player->IntersectMySlowboxes(currHurtboxes, currHurtboxFrame));
}

HitboxInfo * Enemy::IsHit(Actor *player)
{
	if (currHurtboxes == NULL)
		return NULL;

	if (CanBeHitByPlayer())
	{
		if (player->IntersectMyHitboxes(currHurtboxes, currHurtboxFrame))
		{
			return player->currHitboxes->hitboxInfo;
		}
	}

	if (CanBeHitByComboer())
	{
		ComboObject *co = player->IntersectMyComboHitboxes(this, currHurtboxes, currHurtboxFrame);
		if (co != NULL)
		{
			HitboxInfo *hi = co->enemyHitboxInfo;

			co->enemy->ComboHit();
			comboHitEnemy = co->enemy;
			return hi;
		}

	}

	
	Wire *wire = player->IntersectMyWireHitboxes(this, currHurtboxes, currHurtboxFrame);
	if( wire != NULL )
	{
		HandleWireHit(wire);
		return wire->tipHitboxInfo;
	}
	
	return NULL;
}

bool Enemy::CheckHitPlayer(int index)
{
	Actor *player = owner->GetPlayer(index);

	if (currHitboxes != NULL)
	{
		//!player->IsSequenceAction(player->action) &&
		if (player->IntersectMyHurtboxes(currHitboxes, currHitboxFrame))
		{
			IHitPlayer(index);
			player->ApplyHit(currHitboxes->hitboxInfo);
			return true;
		}
	}

	return false;
}

EnemyParamsManager::EnemyParamsManager()
{
	memset(params, 0, sizeof(params));
}

EnemyParams *EnemyParamsManager::GetHitParams(EnemyType et)
{
	EnemyParams *& ep = params[et];
	if ( ep == NULL)
	{
		switch (et)
		{
		case EnemyType::EN_CRAWLER:
			ep = new EnemyParams(1, 5, .8, (3 * 60)/4, 4);
			break;
		case EnemyType::EN_GOAL:
			ep = new EnemyParams(1, 5, .8, 6, 3);
			break;
		case EnemyType::EN_PATROLLER:
			ep = new EnemyParams(1, 5, .8, (3*60)/3, 3);
			break;
		case EnemyType::EN_FOOTTRAP:
			ep = new EnemyParams(1, 5, .8, (3*60)/3, 3);
			break;
		case EnemyType::EN_COMBOER:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_AIRDASHER:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_BASICTURRET:
			ep = new EnemyParams(1, 5, .8, (3*60)/3, 3);
			break;
		case EnemyType::EN_SHROOM:
			ep = new EnemyParams(1, 5, .8, (3*60)/3, 3);
			break;
		case EnemyType::EN_SHROOMJELLY:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_CRAWLERQUEEN:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);//40);//40);// 120);
			break;
		case EnemyType::EN_FLOATINGBOMB:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_BLOCKER:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_BOSS_BIRD:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 40, 400);
			break;
		case EnemyType::EN_BAT:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_AIRDASHJUGGLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_GRAVITYJUGGLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_PRIMARYJUGGLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_BOUNCEJUGGLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_GRINDJUGGLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_POISONFROG:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_CURVETURRET:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_STAGBEETLE:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 4, 4);
			break;
		case EnemyType::EN_GRAVITYFALLER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 4, 4);
			break;
		case EnemyType::EN_SPIDER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_PULSER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_BADGER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_BOUNCEFLOATER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_OWL:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_ROADRUNNER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_CACTUS:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_CACTUSSHOTGUN:
			ep = new EnemyParams(1, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_CHEETAH:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_HUNGRYCOMBOER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_RELATIVECOMBOER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_SWARM:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_SWARMMEMBER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_GHOST:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 1);
			break;
		case EnemyType::EN_GROWINGTREE:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_SHARK:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 3, 3);
			break;
		case EnemyType::EN_SPECTER:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 5, 5);
			break;
		case EnemyType::EN_GORILLA:
			ep = new EnemyParams(2, 5, .8, (3 * 60) / 5, 5);
			break;
		case EnemyType::EN_WIRETARGET:
			ep = new EnemyParams(2, 5, 0, 0, 1);
		default:
			return NULL;
		}
	}

	return ep;
}

EnemyParamsManager::~EnemyParamsManager()
{
	for (int i = 0; i < EN_Count; ++i)
	{
		if (params[i] != NULL)
		{
			delete params[i];
			params[i] = NULL;
		}
	}
}

bool HittableObject::CheckHit( Actor *player, EnemyType et )
{
	if (receivedHit == NULL && !specterProtected )
	{
		comboHitEnemy = NULL;
		receivedHit = IsHit(player);
		if (receivedHit == NULL)
			return false;

		if (receivedHit->hType < HitboxInfo::HitboxType::WIREHITRED)
		{
			player->ConfirmHit(player->owner->eHitParamsMan->GetHitParams(et));
		}
		else
		{
			int ff = 0;
			//assert(0);
		}
	}
	return false;
}

CuttableObject::CuttableObject()
{
	Reset();
	separateSpeed = .4;
	ts = NULL;
	rectWidth = 32;
	rectHeight = 32;
	scale = 1.f;

	for (int i = 0; i < 2; ++i)
	{
		quads[i * 4 + 0].color = Color::Blue;
		quads[i * 4 + 1].color = Color::Blue;
		quads[i * 4 + 2].color = Color::Blue;
		quads[i * 4 + 3].color = Color::Blue;
	}
}

void CuttableObject::SetSubRectFront( int fIndex )
{
	if (ts != NULL)
	{
		tIndexFront = fIndex;
		IntRect fr = ts->GetSubRect(tIndexFront);
		quads[4].texCoords = Vector2f(fr.left, fr.top);
		quads[5].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[6].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[7].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[4].color = Color::White;
		quads[5].color = Color::White;
		quads[6].color = Color::White;
		quads[7].color = Color::White;
	}
}

void CuttableObject::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	rectWidth = ts->tileWidth;
	rectHeight = ts->tileHeight;
}

void CuttableObject::SetSubRectBack( int bIndex )
{
	if (ts != NULL)
	{
		tIndexBack = bIndex;

		IntRect fr = ts->GetSubRect(tIndexBack);
		quads[0].texCoords = Vector2f(fr.left, fr.top);
		quads[1].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[2].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[3].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[0].color = Color::White;
		quads[1].color = Color::White;
		quads[2].color = Color::White;
		quads[3].color = Color::White;
	}
}

void CuttableObject::SetCutRootPos(sf::Vector2f &p_rPos )
{
	separateFrame = 0;
	rootPos = p_rPos;
	active = true;
}
bool CuttableObject::DoneSeparatingCut()
{
	return active
		&& separateFrame == totalSeparateFrames;
}

void CuttableObject::Reset()
{
	splitDir = Vector2f(-1, 0);
	separateFrame = 0;
	totalSeparateFrames = 60;
	active = false;
	rotateAngle = 0;
	flipHoriz = false;
	flipVert = false;
}

void CuttableObject::UpdateCutObject( int slowCounter )
{
	Vector2f currSplitDir = splitDir;

	if (active)
	{
		Transform sprT;
		sprT.rotate(rotateAngle);
		
		if (flipHoriz)
		{
			sprT.scale(-1, 1);
		}
		if (flipVert)
		{
			sprT.scale(1, -1);
		}

		currSplitDir = sprT.transformPoint(currSplitDir);

		int halfWidth = rectWidth / 2 * scale;
		int halfHeight = rectHeight / 2 * scale;

		Vector2f root[2];
		root[0] = rootPos + currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));
		root[1] = rootPos - currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));

		Vector2f temp;

		float f;
		if (separateFrame <= 30)
		{
			f = 1.f;
		}
		else
		{
			f = 1.f - ( separateFrame - (totalSeparateFrames / 2) ) / (float)(totalSeparateFrames / 2);
		}
		for (int i = 0; i < 2; ++i)
		{
			quads[i * 4 + 0].position = root[i] + sprT.transformPoint(Vector2f(-halfWidth, -halfHeight));
			quads[i * 4 + 1].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, -halfHeight));
			quads[i * 4 + 2].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, halfHeight));
			quads[i * 4 + 3].position = root[i] + sprT.transformPoint( Vector2f(-halfWidth, halfHeight) );

			SetRectColor(quads + i * 4, Color(255, 255, 255, 255 * f));
			
			/*if (flipHoriz)
			{
				temp = quads[i * 4 + 0].position;
				quads[i * 4 + 0].position = quads[i * 4 + 1].position;
				quads[i * 4 + 1].position = temp;

				temp = quads[i * 4 + 3].position;
				quads[i * 4 + 3].position = quads[i * 4 + 2].position;
				quads[i * 4 + 2].position = temp;
			}*/
			

			
		}
	}
}

void CuttableObject::SetScale(float p_scale)
{
	scale = p_scale;
}

void CuttableObject::Draw(sf::RenderTarget *target)
{
	if( ts != NULL )
		target->draw(quads, 8, sf::Quads, ts->texture);
	else
		target->draw(quads, 8, sf::Quads);
}

void CuttableObject::IncrementFrame()
{
	if (active)
	{
		if (separateFrame < totalSeparateFrames)
		{
			++separateFrame;
		}
		else
		{
			active = false;
		}
	}
}

V2d ComboObject::GetComboPos()
{
	sf::Rect<double> r = enemyHitBody->GetAABB(enemyHitboxFrame);
	
	return V2d(r.left + r.width / 2.0, r.top + r.height / 2.0);
}

void ComboObject::ComboHit()
{

}

void ComboObject::Reset()
{
	enemyHitboxFrame = 0;
	nextComboObj = NULL;
	active = false;
}

void ComboObject::Draw(RenderTarget *target)
{
	enemyHitBody->DebugDraw( enemyHitboxFrame, target);
}

ComboObject::~ComboObject()
{
	if (enemyHitboxInfo != NULL)
		delete enemyHitboxInfo;
	
	if (enemyHitBody != NULL)
		delete enemyHitBody;
}
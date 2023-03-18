#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GrindJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "EditorRail.h"
#include "MainMenu.h"
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;

void GrindJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedKills)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		maxKilled = jParams->numJuggles;
		UpdateKilledNumberText(maxKilled);
	}
}

GrindJuggler::GrindJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_GRINDJUGGLER, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	if (ap->GetTypeName() == "grindjugglercw")
	{
		clockwise = true;
	}
	else
	{
		clockwise = false;
	}

	SetLevel(ap->GetLevel());

	const string &typeName = ap->GetTypeName();

	if (typeName == "grindjugglercw")
	{
		limitedKills = false;
		clockwise = true;
	}
	else if (typeName == "limitedgrindjugglercw")
	{
		clockwise = true;
		limitedKills = true;
	}
	else if (typeName == "grindjugglerccw")
	{
		clockwise = false;
		limitedKills = false;
	}
	else if (typeName == "limitedgrindjugglerccw")
	{
		clockwise = false;
		limitedKills = true;
	}

	numKilledText.setFont(sess->mainMenu->arial);
	numKilledText.setFillColor(Color::White);
	numKilledText.setOutlineColor(Color::Black);
	numKilledText.setOutlineThickness(3);
	numKilledText.setCharacterSize(32);

	flySpeed = 10;

	maxWaitFrames = 100000;//180;

	CreateSurfaceMover(startPosInfo, 10, this);

	UpdateParamsSettings();

	hitLimit = -1;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(48);
	BasicCircleHurtBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);
	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 10;
	actionLength[S_GRIND] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_GRIND] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

GrindJuggler::~GrindJuggler()
{
}

void GrindJuggler::SetLevel(int lev)
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

bool GrindJuggler::CanLeaveRail()
{
	return true;
}

void GrindJuggler::LeaveRail()
{

}

void GrindJuggler::BoardRail()
{
	if (clockwise)
	{
		//surfaceMover->SetRailSpeed(flySpeed);
	}
	else
	{
		//surfaceMover->SetRailSpeed(-flySpeed);
	}
}

void GrindJuggler::ResetEnemy()
{
	surfaceMover->SetCollisionOn(true);
	//surfaceMover-> =railCollisionOn = true;
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);
	
	action = S_FLOAT;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	data.numKilled = 0;

	UpdateKilledNumberText(maxKilled);

	UpdateHitboxes();

	UpdateSprite();
}

void GrindJuggler::UpdateKilledNumberText(int reps)
{
	if (limitedKills)
	{
		data.numKilledTextNumber = reps;
		numKilledText.setString(to_string(reps));
		numKilledText.setOrigin(numKilledText.getLocalBounds().left
			+ numKilledText.getLocalBounds().width / 2,
			numKilledText.getLocalBounds().top
			+ numKilledText.getLocalBounds().height / 2);
	}
}

void GrindJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	surfaceMover->SetVelocity(vel);
}

void GrindJuggler::Throw(V2d vel)
{
	surfaceMover->SetVelocity(vel);
}

void GrindJuggler::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();
	HitboxesOff();

	surfaceMover->ground = NULL;

	data.numKilled = 0;

	numHealth = maxHealth;
}

void GrindJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	HurtboxesOff();
	HitboxesOff();
	data.waitFrame = 0;
}

void GrindJuggler::PopThrow()
{
	V2d dir;

	dir = Get8Dir(receivedHit.hDir);//normalize(receivedHit->hDir);
							//cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		dir = normalize(V2d(data.velocity.y, -data.velocity.x));//-normalize(velocity);
		assert(dir.x != 0 || dir.y != 0);
		action = S_FLY;
		frame = 0;
	}

	double speed = 14;

	V2d hit = dir * speed;

	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void GrindJuggler::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			action = S_FLY;
			frame = 0;
			PopThrow();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
	}
}

void GrindJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
		{
			UpdateKilledNumberText(maxKilled);
			surfaceMover->Set(startPosInfo);
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
		}
	}




	/*if (action != S_FLOAT && action != S_EXPLODE && action != S_RETURN)
	{
	sf::Rect<double> r(position.x - 50, position.y - 50, 100, 100);
	owner->activeEnemyItemTree->Query(this, r);
	}*/


	/*if (action == S_POP && ((velocity.y >= 0 && !reversedGrav) || (velocity.y <= 0 && reversedGrav)))
	{
	action = S_JUGGLE;
	frame = 0;
	SetHurtboxes(hurtBody, 0);
	}*/
}

void GrindJuggler::HandleNoHealth()
{

}

void GrindJuggler::ExtraQueries(Rect<double> &r)
{
	//Rect<double> r = mover->physBody.GetAABB();//hitBody->GetCollisionBoxes(0)->front().GetAABB();
	/*if (action == S_FLY)
	{
		owner->railEdgeTree->Query(this, r);
	}*/
}

void GrindJuggler::Move()
{
	surfaceMover->Move(slowMultiple, numPhysSteps);
}

void GrindJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_GRIND:
	{
		Move();
		break;
	}
	case S_RAILGRIND:
	{
		Move();
		//currRail->
		break;
	}
		
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void GrindJuggler::FrameIncrement()
{
	if (action == S_FLY || action == S_GRIND)
	{
		if (data.waitFrame == maxWaitFrames)
		{
			Return();
		}
		else
		{
			data.waitFrame++;
		}
	}
}

void GrindJuggler::HandleEntrant(QuadTreeEntrant *qte)
{
	Edge *e = (Edge*)qte;
	Rail *rail = (Rail*)e->info;

	V2d pos = surfaceMover->GetPosition();
	V2d tempVel = surfaceMover->tempVel;

	//if (IsEdgeTouchingCircle(e->v0, e->v1, mover->physBody.globalPosition, mover->physBody.rw))
	
	V2d r;
	V2d eFocus;
	bool ceiling = false;
	V2d en = e->Normal();
	if (en.y > 0)
	{
		r = e->v0 - e->v1;
		eFocus = e->v1;
		ceiling = true;
	}
	else
	{
		r = e->v1 - e->v0;
		eFocus = e->v0;
	}

	V2d along = normalize(r);
	double lenR = length(r);
	double q = dot(pos - eFocus, along);

	double c = cross(pos - e->v0, along);
	double preC = cross((pos - tempVel) - e->v0, along);

	double alongQuantVel = dot(data.velocity, along);

	bool cStuff = (c >= 0 && preC <= 0) || (c <= 0 && preC >= 0);

	if (cStuff && q >= 0 && q <= lenR)//&& alongQuantVel != 0)
	{
		V2d rn(along.y, -along.x);

		railEdge = e;
		//prevRail = (Rail*)grindEdge->info;

		V2d myPos = GetPosition();
		LineIntersection li;
		lineIntersection( li, myPos, myPos - tempVel, 
			railEdge->v0, railEdge->v1);
		if (!li.parallel)
		{
			V2d p = li.position;
			railQuant = railEdge->GetQuantity(p);

			myPos = p;
		}
		else
		{
			assert(0);

			//probably just start at the beginning or end of the rail based on which is closer?
		}

		if (ceiling) //ceiling rail
		{
			railSpeed = -10;
		}
		else
		{
			railSpeed = 10;
		}

		
		//surfaceMover->Set()
		surfaceMover->surfaceMoverData.physBody.globalPosition = myPos;
		surfaceMover->ground = railEdge;
		surfaceMover->surfaceMoverData.edgeQuantity = railQuant;
		surfaceMover->SetSpeed(10);
		
		action = S_RAILGRIND;
		frame = 0;
		surfaceMover->SetVelocity(along * 10.0);//V2d(0, 0);
	}
	//Enemy *en = (Enemy*)qte;
	//if (en->type == EnemyType::EN_JUGGLERCATCHER)
	//{
	//	JugglerCatcher *catcher = (JugglerCatcher*)qte;

	//	CollisionBox &hitB = hurtBody->GetCollisionBoxes(0)->front();
	//	if (catcher->CanCatch() && catcher->hurtBody->Intersects(catcher->currHurtboxFrame, &hitB))
	//	{
	//		//catcher->Catch();
	//		//action = S_EXPLODE;
	//		//frame = 0;
	//	}
	//}
}


void GrindJuggler::ComboKill(Enemy *e)
{
	if (action == S_GRIND)
	{
		action = S_FLY;
		frame = 0;

		surfaceMover->SetSpeed(0);

		surfaceMover->SetVelocity(surfaceMover->ground->Normal() * flySpeed);
		surfaceMover->ground = NULL;

		++data.numKilled;

		if (limitedKills && data.numKilled == maxKilled)
		{
			if (hasMonitor && !suppressMonitor)
			{
				sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
					sess->GetPlayer(0), 1, GetPosition());
				suppressMonitor = true;
			}
			Return();

			return;
		}
		UpdateKilledNumberText(maxKilled - data.numKilled);

		DefaultHurtboxesOn();

		sess->PlayerRemoveActiveComboer(comboObj);
	}
}

void GrindJuggler::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		
		Return();
	}
	else
	{
		/*if (action == S_GRIND)
		{
			action = S_FLY;
			frame = 0;

			mover->SetSpeed(0);

			velocity = mover->ground->Normal() * flySpeed;
			mover->ground = NULL;


			SetHurtboxes(hurtBody, 0);

			owner->PlayerRemoveActiveComboer(comboObj);

		}*/
	}
}

void GrindJuggler::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(6));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (limitedKills)
	{
		numKilledText.setPosition(sprite.getPosition());
	}
}

void GrindJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedKills)
	{
		target->draw(numKilledText);
	}
}

void GrindJuggler::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPosition(quant);

	if (clockwise)
	{
		surfaceMover->SetSpeed(flySpeed);
	}
	else
	{
		surfaceMover->SetSpeed(-flySpeed);
	}
	
	action = S_GRIND;
	frame = 0;
}

int GrindJuggler::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void GrindJuggler::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void GrindJuggler::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	bytes += sizeof(MyData);

	UpdateKilledNumberText(data.numKilledTextNumber);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}
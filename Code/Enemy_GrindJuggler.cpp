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

using namespace std;
using namespace sf;

void GrindJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();

	GrindJugglerParams *gjParams = (GrindJugglerParams*)editParams;

	clockwise = gjParams->clockwise;
}

GrindJuggler::GrindJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_GRINDJUGGLER, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	SetLevel(ap->GetLevel());

	flySpeed = 25;

	CreateSurfaceMover(startPosInfo, 10, this);

	UpdateParamsSettings();

	hitLimit = -1;

	maxMoveFrames = 90;

	ts = GetSizedTileset("Enemies/General/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	BasicCircleHurtBodySetup(48);

	idleTurnDegrees = 4;//1
	moveTurnDegrees = 15;//10

	comboObj = new ComboObject(this);
	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->comboer = true;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::ORANGE;

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 10;
	actionLength[S_GRIND] = 10;
	actionLength[S_RETURN] = 30;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_GRIND] = 1;
	animFactor[S_RETURN] = 1;

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
		scale = 1.5;
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
	data.currHits = 0;
	data.currAngle = 0;
	data.moveFrame = 0;

	surfaceMover->SetCollisionOn(true);
	//surfaceMover-> =railCollisionOn = true;
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);
	
	action = S_FLOAT;
	frame = 0;

	DefaultHurtboxesOn();

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	UpdateHitboxes();

	UpdateSprite();
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

	surfaceMover->ground = NULL;

	numHealth = maxHealth;
}

void GrindJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	HurtboxesOff();
}

void GrindJuggler::PopThrow()
{
	V2d dir;

	action = S_FLY;
	frame = 0;

	data.moveFrame = 0;

	dir = Get8Dir(receivedHit.hDir);//normalize(receivedHit->hDir);
									//cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		dir = normalize(V2d(data.velocity.y, -data.velocity.x));//-normalize(velocity);
		assert(dir.x != 0 || dir.y != 0);
		action = S_FLY;
		frame = 0;
	}

	V2d hit = dir * flySpeed;//speed;

	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void GrindJuggler::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{			
			if (hasMonitor && !suppressMonitor)
			{
				ActivateDarkAbsorbParticles();
				suppressMonitor = true;
				PlayKeyDeathSound();
			}

			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();

			PopThrow();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
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
			surfaceMover->Set(startPosInfo);
			DefaultHurtboxesOn();
			action = S_FLOAT;
			data.moveFrame = 0;
			data.currAngle = 0;
			frame = 0;
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
		}
	}

	if (action == S_FLOAT)
	{
		if (clockwise)
		{
			data.currAngle += idleTurnDegrees;
		}
		else
		{
			data.currAngle += -idleTurnDegrees;
		}
	}
	else if (action == S_FLY || action == S_GRIND || action == S_RAILGRIND )
	{
		if (clockwise)
		{
			data.currAngle += moveTurnDegrees;
		}
		else
		{
			data.currAngle += -moveTurnDegrees;
		}
	}
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
	if (action == S_FLY || action == S_GRIND || action == S_RAILGRIND )
	{
		if (data.moveFrame == maxMoveFrames)
		{
			Return();
		}
		else
		{
			data.moveFrame++;
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
}

void GrindJuggler::ComboHit()
{
	pauseFrames = 6;
	++data.currHits;
	//data.moveFrame = 0;
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

	sprite.setRotation(data.currAngle);

	sprite.setTextureRect(ts->GetSubRect(6));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
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

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}
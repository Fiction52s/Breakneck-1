#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CrawlerQueen.h"
#include "VisualEffects.h"

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

CrawlerQueen::CrawlerQueen(GameSession *owner, Edge *g, double q, bool cw )
	:Enemy(owner, EnemyType::EN_CRAWLERQUEEN, false, 1, false), clockwise(cw)
{
	bombSpeed = 2;
	currDigAttacks = 3;
	highResPhysics = true;
	redecide = false;
	origCW = cw;
	mover = new SurfaceMover(owner, g, q, 32);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);
	baseSpeed = 8;

	numDecisions = 3;

	decMarkerPool = new EffectPool(EffectType::FX_REGULAR, MAX_DECISIONS, 1.f);
	decideIndex = 0;
	initHealth = 60;
	health = initHealth;
	dead = false;


	
	actionLength[DECIDE] = 1;
	actionLength[BOOST] = 1;
	actionLength[TURNAROUNDBOOST] = 4;
	actionLength[STOPBOOST] = 1;
	actionLength[WAIT] = 1;
	actionLength[JUMP] = 3;
	actionLength[BURROW] = 22;
	actionLength[RUMBLE] = 20;
	actionLength[POPOUT] = 27;
	actionLength[UNBURROW] = 12;

	animFactor[DECIDE] = 2;
	animFactor[BOOST] = 1;
	animFactor[TURNAROUNDBOOST] = 2;
	animFactor[STOPBOOST] = 2;
	animFactor[WAIT] = 1;
	animFactor[JUMP] = 2;
	animFactor[BURROW] = 2;
	animFactor[RUMBLE] = 1;
	animFactor[POPOUT] = 2;
	animFactor[UNBURROW] = 2;

	double width = 320;
	double height = 320;

	ts[WAIT] = owner->GetTileset("Bosses/Crawler/crawler_queen_stand_320x320.png", 320, 320);
	ts[DECIDE] = owner->GetTileset("Bosses/Crawler/crawler_queen_stand_320x320.png", 320, 320);
	ts[BOOST] = owner->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[TURNAROUNDBOOST] = owner->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[STOPBOOST] = owner->GetTileset("Bosses/Crawler/crawler_queen_dash_320x320.png", 320, 320);
	ts[JUMP] = owner->GetTileset("Bosses/Crawler/crawler_queen_jump_320x320.png", 320, 320);
	ts[BURROW] = owner->GetTileset("Bosses/Crawler/crawler_queen_dig_in_320x320.png", 320, 320);
	ts[RUMBLE] = NULL;//owner->GetTileset("Bosses/crawler_queen_dash_320x320.png", 320, 320);
	ts[POPOUT] = owner->GetTileset("Bosses/Crawler/crawler_queen_slash_320x320.png", 320, 320);
	ts[UNBURROW] = owner->GetTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png", 320, 320);

	sprite.setTexture(*ts[WAIT]->texture);
	sprite.setTextureRect(ts[WAIT]->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = mover->ground->GetPoint(q);
	sprite.setPosition(gPoint.x, gPoint.y);
	V2d gNorm = mover->ground->Normal();

	decideDelayFrames = 20;

	multSpeed = 1;

	double angle = atan2(gNorm.x, -gNorm.y);
	sprite.setRotation(angle / PI * 180.f);
	position = gPoint + gNorm * height / 2.0;

	double size = max(width, height);
	spawnRect = sf::Rect<double>(gPoint.x - size / 2, gPoint.y - size / 2, size, size);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);


	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = hitboxInfo;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	startGround = g;
	startQuant = q;
	frame = 0;

	

	action = DECIDE;
	frame = 0;

	edgeRef = NULL;

	InitEdgeInfo();

	bombPool = new ObjectPool;
	

	for (int i = 0; i < 100; ++i)
	{
		FloatingBomb *fb = new FloatingBomb(owner, bombPool, i );
		bombPool->AddToInactiveList(fb);
	}

	decidePoints = new EdgeInfo[MAX_DECISIONS];
	decisions = new Decision[MAX_DECISIONS];

	//frame = actionLength[UNDERGROUND];
}

CrawlerQueen::~CrawlerQueen()
{
	delete[] edgeRef;
}

void CrawlerQueen::InitEdgeInfo()
{
	assert(edgeRef == NULL);

	numTotalEdges = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;
	Edge *startEdge = curr;

	do
	{
		numTotalEdges++;

		curr = curr->edge1;
	} while (curr != startEdge);

	edgeRef = new Edge*[numTotalEdges];

	

	int edgeIndex = 0;
	do
	{
		edgeRef[edgeIndex] = curr;
		edgeIndexMap[curr] = edgeIndex;
		curr = curr->edge1;
		++edgeIndex;
	}
	while (curr != startEdge);
}

void CrawlerQueen::ResetEnemy()
{
	redecide = false;
	clockwise = origCW;
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
	decideIndex = 0;
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	position = mover->physBody.globalPosition;

	health = initHealth;

	frame = 0;
	V2d gPoint = mover->ground->GetPoint(startQuant);
	sprite.setPosition(gPoint.x, gPoint.y);

	position = gPoint + mover->ground->Normal() * 64.0 / 2.0;
	V2d gn = mover->ground->Normal();
	dead = false;

	double angle = 0;
	angle = atan2(gn.x, -gn.y);

	sprite.setTextureRect(ts[WAIT]->GetSubRect(0));
	V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation(angle / PI * 180);
	sprite.setPosition(pp.x, pp.y);

	UpdateHitboxes();

	action = DECIDE;
	frame = 0;

	FloatingBomb *fb = (FloatingBomb*)bombPool->activeListStart;
	while( fb != NULL )
	{		
		fb->Reset();
		fb = (FloatingBomb*)fb->pmnext;
	}

	bombPool->DeactivateAll();
}

void CrawlerQueen::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	if (mover->ground != NULL)
	{
		V2d gn = mover->ground->Normal();
		double angle = 0;


		angle = atan2(gn.x, -gn.y);

		hitBox.globalAngle = angle;
		hurtBox.globalAngle = angle;

		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		double maxExtraKB = 15.0;
		double baseKB = 8;
		if (mover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = baseKB + max(abs(mover->groundSpeed), maxExtraKB);
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
			hitboxInfo->knockback = baseKB + max(abs(mover->groundSpeed), maxExtraKB);
		}
	}
	else
	{
		hitBox.globalAngle = 0;
		hurtBox.globalAngle = 0;
	}
	hitBox.globalPosition = mover->physBody.globalPosition;
	hurtBox.globalPosition = mover->physBody.globalPosition;
}

void CrawlerQueen::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case WAIT:
			break;
		case BOOST:
		{
			
			
			break;
		}
		case DECIDE:
			//cout << "decide: " << frame << endl;
			break;
		case TURNAROUNDBOOST:
			break;
		case STOPBOOST:
			break;
		case JUMP:
			break;
		case BURROW:
			action = RUMBLE;
			frame = 0;
			digAttackCounter = 0;
			break;
		case RUMBLE:
			action = POPOUT;
			mover->ground = decidePoints[digAttackCounter].edge;
			mover->edgeQuantity = decidePoints[digAttackCounter].quantity;
			mover->UpdateGroundPos();
			frame = 0;
			break;
		case POPOUT:
			++digAttackCounter;
			if (digAttackCounter == currDigAttacks)
			{
				mover->ground = digInfo.edge;
				mover->edgeQuantity = digInfo.quantity;
				mover->UpdateGroundPos();
				action = UNBURROW;
				frame = 0;
			}
			else
			{
				action = RUMBLE;
				frame = 0;
			}
			break;
		case UNBURROW:
			Boost();
			break;
		}
	}
	
	if (redecide)
	{
		action = DECIDE;
		frame = 0;
		decideIndex = 0;
		mover->SetSpeed(0);
		redecide = false;
	}

//	cout << "action: " << action << endl;
	switch (action)
	{
	case WAIT:
		break;
	case BOOST:
	{
		assert(mover->ground != NULL);

		//int gIndex = edgeIndexMap[mover->ground];
		//double gQuant = mover->edgeQuantity;
		//int dIndex = decidePoints[travelIndex].index;
		//int sIndex = startTravelPoint.index;

		//if (clockwise)
		//{
		//	//past where it needs to stop
		//	if ((dIndex > sIndex && (gIndex < sIndex || gIndex > dIndex))
		//		|| (dIndex < sIndex && (gIndex > dIndex && gIndex < sIndex))
		//		|| (gIndex == dIndex && gQuant > startTravelPoint.quantity))
		//	{
		//		DecideNextAction(gIndex);
		//	}
		//}
		//else
		//{
		//	if (( dIndex < sIndex && ( gIndex > sIndex || gIndex < dIndex ) ) 
		//		|| (dIndex > sIndex && ( gIndex < dIndex && gIndex > sIndex ) )
		//		|| (gIndex == dIndex && gQuant < startTravelPoint.quantity) )
		//	{
		//		DecideNextAction(gIndex);
		//	}
		//}
		break;
	}
	case DECIDE:
		if (frame == 0)
		{
			DecidePoints();
			SetDecisions();
		}
		if (frame % decideDelayFrames == 0 && frame > 0)
		{
			if (decideIndex == numDecisions - 1)
			{
				startTravelPoint.edge = mover->ground;
				startTravelPoint.index = mover->edgeQuantity;
				startTravelPoint.index = edgeIndexMap[mover->ground];
				travelIndex = 0;
				redecide = false;
				DecideAction();
			}
			else
			{
				decideIndex++;
			}
		}
		break;
	case TURNAROUNDBOOST:
		break;
	case STOPBOOST:
		break;
	case JUMP:
		break;
	case BURROW:
		break;
	case RUMBLE:
		break;
	case POPOUT:

		break;
	case UNBURROW:
		break;
	}

	switch (action)
	{
	case WAIT:
		break;
	case DECIDE:
		break;
	case TURNAROUNDBOOST:
		break;
	case STOPBOOST:
		break;
	case JUMP:
		break;
	case BURROW:
		break;
	case RUMBLE:
		break;
	case POPOUT:
		if (frame == 10)
		{
			FloatingBomb *fb = (FloatingBomb*)bombPool->ActivatePoolMember();
			fb->Init(mover->ground->GetPoint(mover->edgeQuantity), mover->ground->Normal() * bombSpeed);
			owner->AddEnemy(fb);
			cout << "spawning bomb: " << fb << ": " << fb->position.x << ", " << fb->position.y << endl;
		}
		break;
	case UNBURROW:
		break;
	}
}

void CrawlerQueen::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple, numPhysSteps);
		position = mover->physBody.globalPosition;

		switch (action)
		{
		case BOOST:
		{
			if (travelIndex < numDecisions)
			{
				V2d middlePoint = decidePoints[travelIndex].edge->GetPoint(decidePoints[travelIndex].quantity)
					+ decidePoints[travelIndex].edge->Normal() * mover->physBody.rw;

				if (length(position - middlePoint) < 20)
				{
					DecideNextAction();
				}
			}
			break;
		}
		}
	}
}

void CrawlerQueen::FrameIncrement()
{
}

void CrawlerQueen::EnemyDraw(sf::RenderTarget *target)
{
	if (action == RUMBLE )
		return;

	if (owner->pauseFrames < 2 || receivedHit == NULL)
	{
		target->draw(sprite);
	}
	else
	{
		target->draw(sprite, hurtShader);
	}
}

void CrawlerQueen::IHitPlayer(int index)
{

}


void CrawlerQueen::HandleNoHealth()
{
}

void CrawlerQueen::HitTerrainAerial(Edge *e, double q)
{
	mover->ground = e;
	mover->edgeQuantity = q;
	mover->UpdateGroundPos();

	Boost();
}

void CrawlerQueen::UpdateSprite()
{
	if (action == RUMBLE)
	{
		return;
	}
	//V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);

	//return;
	float extraVert = 34;

	double angle = 0;

	IntRect ir;

	Tileset *currTS = NULL;

	currTS = ts[action];
	sprite.setTexture(*currTS->texture);

	int currTile = 0;


	switch (action)
	{
	case WAIT:
		currTile = 0;
		break;
	case DECIDE:
		currTile = 0;
		break;
	case BOOST:
		currTile = 0;
		break;
	case TURNAROUNDBOOST:
		currTile = frame / animFactor[action] + 1;
		break;
	case STOPBOOST:
		currTile = 10;
		break;
	case JUMP:
		currTile = 1;
		break;
	case BURROW:
		currTile = frame / animFactor[action];
		break;
	case POPOUT:
		currTile = frame / animFactor[action];
		break;
	case UNBURROW:
		currTile = frame / animFactor[action];
		break;
	}

	ir = currTS->GetSubRect(currTile);

	if (clockwise)
	{
		sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
	}
	else
	{
		sprite.setTextureRect(ir);
	}

	if (mover->ground != NULL)
	{
		V2d gn = mover->ground->Normal();

		if (!mover->roll)
		{
			angle = atan2(gn.x, -gn.y);

			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			if (clockwise)
			{
				V2d vec = normalize(position - mover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
				sprite.setPosition(pp.x, pp.y);
			}
			else
			{
				V2d vec = normalize(position - mover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
				sprite.setPosition(pp.x, pp.y);
			}
		}
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(Vector2f(mover->physBody.globalPosition));
	}


	for (int i = 0; i < numDecisions; ++i)
	{
		if( i == 0 )
			decDebugDraw[i].setFillColor(Color::Red);
		else if( i == 1) 
			decDebugDraw[i].setFillColor(Color::Blue);
		else
		{
			decDebugDraw[i].setFillColor(Color::Green);
		}
			
		decDebugDraw[i].setRadius(32);
		decDebugDraw[i].setOrigin(Vector2f(decDebugDraw[i].getLocalBounds().width / 2,
			decDebugDraw[i].getLocalBounds().height / 2) );
		decDebugDraw[i].setPosition(
			Vector2f(decidePoints[i].edge->GetPoint(decidePoints[i].quantity)) 
			+ Vector2f(decidePoints[i].edge->Normal() * mover->physBody.rw));
	}
}

void CrawlerQueen::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		mover->physBody.DebugDraw(target);
	
	for (int i = 0; i < numDecisions; ++i)
	{
		target->draw(decDebugDraw[i]);
	}
}

void CrawlerQueen::TransferEdge(Edge *e)
{
}

bool CrawlerQueen::PlayerInFront()
{
	V2d dir;
	if (clockwise)
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise)
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double alongDist = dot(owner->GetPlayer(0)->position - mover->physBody.globalPosition, dir);
	if (alongDist > -60)
		return true;
	else
		return false;
}

void CrawlerQueen::Accelerate(double amount)
{
	if (clockwise)
	{
		amount = abs(amount);
	}
	else
	{
		amount = -abs(amount);
	}

	double max = 35.0;
	double am = mover->groundSpeed + amount;
	if (am > max)
		am = max;
	if (am < -max)
		am = -max;
	cout << "speed: " << am << endl;
	mover->SetSpeed(am);
}

void CrawlerQueen::SetForwardSpeed(double speed)
{
	double aSpeed = abs(speed);
	if (clockwise)
	{
		mover->SetSpeed(aSpeed);
	}
	else
	{
		mover->SetSpeed(-aSpeed);
	}
}

void CrawlerQueen::SetDecisions()
{
	for (int i = 0; i < numDecisions; ++i)
	{
		int r = rand() % D_Count;

		decisions[i] = (Decision)r;//D_DIG;//(Decision)r;
	}
}

#include <math.h>
void CrawlerQueen::DecidePoints()
{
	//add more to this algorithm later to balance it

	for (int i = 0; i < numDecisions; ++i)
	{
		int r = rand() % numTotalEdges;
		int quant = rand() % (int)edgeRef[r]->GetLength();
		
		decidePoints[i].edge = edgeRef[r];
		decidePoints[i].index = r;
		decidePoints[i].quantity = quant;
	}
}

double CrawlerQueen::GetDistanceCCW(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;


	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	sum += currQ;
	curr = curr->edge0;

	while (curr != e)
	{
		sum += length(curr->v1 - curr->v0);
		curr = curr->edge0;
	}

	sum += length(curr->v1 - curr->v0) - q;

	return sum;
}

void CrawlerQueen::Jump()
{
	action = JUMP;
	frame = 0;
	mover->velocity = mover->ground->Normal() * 10.0;
	mover->ground = NULL;
}

void CrawlerQueen::Boost()
{
	action = BOOST;
	frame = 0;
	if (GetClockwise(travelIndex))
	{
		clockwise = true;
		mover->SetSpeed(baseSpeed + travelIndex * multSpeed);
		//cout << "travel: " << travelIndex << ", clockwise: " << startTravelPoint.index << " to " << decidePoints[travelIndex].index << endl;
	}
	else
	{
		clockwise = false;
		mover->SetSpeed(-baseSpeed - travelIndex * multSpeed);
		//cout << "travel: " << travelIndex << ", CCW: " << startTravelPoint.index << " to " << decidePoints[travelIndex].index << endl;
	}
}

void CrawlerQueen::DecideAction()
{
	if (travelIndex == numDecisions)
	{
		redecide = true;
		//action = DECIDE;
		//frame = 0;
		//decideIndex = 0;
		return;
	}

	decisions[travelIndex] = D_DIG;
	switch (decisions[travelIndex])
	{
	case D_BOOST:
		{
		Boost();	
		break;
		}
	case D_JUMP:
	{
		Jump();
		break;
	}
	case D_DIG:
	{
		mover->SetSpeed(0);
		digInfo.edge = mover->ground;
		digInfo.quantity = mover->edgeQuantity;
		action = BURROW;
		frame = 0;
		digAttackCounter = 0;
		break;
	}
		
	}
}

double CrawlerQueen::GetDistanceClockwise(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;


	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	sum += length(curr->v1 - curr->v0) - currQ;
	curr = curr->edge1;

	while (curr != e)
	{
		sum += length(curr->v1 - curr->v0);
		curr = curr->edge1;
	}

	sum += q;


	return sum;
}

void CrawlerQueen::DecideNextAction()
{
	++travelIndex;
	startTravelPoint.edge = mover->ground;
	startTravelPoint.index = mover->edgeQuantity;
	startTravelPoint.index = edgeIndexMap[mover->ground];
	DecideAction();
}

bool CrawlerQueen::GetClockwise(int index)
{
	Edge *e = decidePoints[index].edge;
	double q = decidePoints[index].quantity;

	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	if (curr == e)
	{
		if (q > currQ)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		double distanceClockwise = GetDistanceClockwise(index);
		double ccw = GetDistanceCCW(index);
		if (distanceClockwise >= ccw)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

FloatingBomb::FloatingBomb(GameSession *owner, ObjectPool *p_myPool, int index )
	:Enemy( owner, EnemyType::EN_FLOATINGBOMB, false, 1, false ),
	PoolMember( index ), myPool( p_myPool )
{
	//preload
	owner->GetTileset("Enemies/bombexplode_512x512.png", 512, 512);

	mover = new SurfaceMover(owner, NULL, 0, 32);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	ts = owner->GetTileset("Enemies/bomb_128x160.png", 128, 160);
	sprite.setTexture(*ts->texture);

	action = FLOATING;

	actionLength[FLOATING] = 9;
	actionLength[EXPLODING] = 4;

	animFactor[FLOATING] = 3;
	animFactor[EXPLODING] = 3;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);


	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = hitboxInfo;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
}

void FloatingBomb::Init(V2d pos, V2d vel)
{
	position = pos;
	mover->velocity = vel;
	action = FLOATING;
	frame = 0;
	mover->physBody.globalPosition = position;
}

void FloatingBomb::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOATING:
			frame = 0;
			break;
		case EXPLODING:
			numHealth = 0;
			dead = true;
			owner->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, 
				owner->GetTileset("Enemies/bombexplode_512x512.png", 512, 512), 
				position, false, 0, 10, 3, true);
			//cout << "deactivating " << this << " . currently : " << myPool->numActiveMembers << endl;
			//myPool->DeactivatePoolMember(this);
			break;
		}
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}

	switch (action)
	{
	case FLOATING:
		break;
	case EXPLODING:
		break;
	}
}

void FloatingBomb::HandleNoHealth()
{

}

void FloatingBomb::FrameIncrement()
{

}

void FloatingBomb::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void FloatingBomb::IHitPlayer(int index)
{
	if (action == FLOATING)
	{
		action = EXPLODING;
		frame = 0;
	}
}

void FloatingBomb::UpdateSprite()
{
	switch (action)
	{
	case FLOATING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[FLOATING]));
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[EXPLODING]));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(Vector2f(position));
}

void FloatingBomb::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		mover->physBody.DebugDraw(target);
}

void FloatingBomb::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

void FloatingBomb::HitTerrainAerial(Edge * e, double q )
{
	if (action == FLOATING)
	{
		mover->velocity = V2d(0, 0);
		action = EXPLODING;
		frame = 0;
	}
}

void FloatingBomb::ResetEnemy()
{
	mover->ground = NULL;
	mover->edgeQuantity = 0;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	action = FLOATING;
	frame = 0;
}

void FloatingBomb::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple, numPhysSteps);
		position = mover->physBody.globalPosition;
	}
}

void FloatingBomb::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOATING )
	{
		action = EXPLODING;
		frame = 0;
	}
		
}
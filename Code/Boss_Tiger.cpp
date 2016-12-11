#include "Boss.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Sequence.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )



Boss_Tiger::Boss_Tiger( GameSession *owner, sf::Vector2i &pos )
	:Enemy( owner, EnemyType::BOSS_TIGER, false, 6 ), deathFrame( 0 ),
	nodeVA( sf::Quads, 13 * 4 ), debugLines( sf::Lines, 2 * 30 )
{
	lockPath = NULL;
	nodeRadius1 = 800;
	nodeRadius2 = 1600;
	//get point of interest where to place tiger's starting position
	//and thats where you put startground and startquant

	//startGround = edge;
	//startQuant = edgeQuantity;
	originalPos = pos;
	
	gridCenter.x = originalPos.x;
	gridCenter.y = originalPos.y;

	currIndex.x = GRID_SIZE_X / 2;
	currIndex.y = GRID_SIZE_Y / 2;

	gridCellSize.x = 64;
	gridCellSize.y = 64;

	gridOrigin = gridCenter + V2d( -currIndex.x * gridCellSize.x, -currIndex.y * gridCellSize.y );

	actionLength[PLAN] = 10;
	actionLength[GRIND] = 60;
	actionLength[LUNGE] = 60;
	
	animFactor[PLAN] = 1;
	animFactor[GRIND] = 1;
	animFactor[LUNGE] = 1;
	
	testCS.setFillColor( Color::Blue );
	testCS.setRadius( 40 );
	testCS.setOrigin( testCS.getLocalBounds().width / 2, testCS.getLocalBounds().height / 2 );

	projMover = new SurfaceMover( owner, NULL, 0, 64 );
	projMover->surfaceHandler = this;
	
	

	//current num links is 248	
	//position.x = pos.x;
	//position.y = pos.y;

	ts_pillar = owner->GetTileset( "bosstiger_pillar_100x100.png", 100, 100 );




	bulletSpeed = 5;

	receivedHit = NULL;
	
	
	flyDuration = 60;
	

	deathFrame = 0;
	
	//launcher = new Launcher( this, owner, 12, 12, position, V2d( 1, 0 ), 2 * PI, 90, true );
	//launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	//V2d pos = edge->GetPoint( edgeQuantity );
	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	action = PLAN;
	frame = 0;

	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

	ts_nextAttackOrb = owner->GetTileset( "bossbird_orbs_64x64.png", 64, 64 );
	nextAttackOrb.setTexture( *ts_nextAttackOrb->texture );
	nextAttackOrb.setTextureRect( ts_nextAttackOrb->GetSubRect( ORB_PUNCH ) );
	nextAttackOrb.setOrigin( nextAttackOrb.getLocalBounds().width / 2, 
		nextAttackOrb.getLocalBounds().height / 2 );

	

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 64;
	hitBody.rh = 64;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	//hitboxInfo->kbDir;

	dead = false;

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	
	activePillars = NULL;
	inactivePillars = NULL;

	for( int i = 0; i < MAX_PILLARS; ++i )
	{
		AddPillar();
	}

	position.x = pos.x;
	position.y = pos.y;

	ConnectNodes();

	mover = NULL;

	ResetEnemy();
	//UpdateHitboxes();
	
	
}

void Boss_Tiger::Init()
{
	PoiInfo *poi = owner->poiMap["tigerstart"];
	startGround = poi->edge;
	startQuant = poi->edgeQuantity;

	mover = new SurfaceMover( owner, startGround, startQuant, 64 );
	mover->surfaceHandler = this;
	mover->SetSpeed( 0 );
	mover->roll = false;
}

void Boss_Tiger::ProjectCircle( V2d &start, V2d &end )
{
	projMover->ground = NULL;
	projMover->physBody.globalPosition = start;
	projMover->velocity = normalize( end - start ) * 5.0;

	projEdge = NULL;
	projQuant = 0;

	projecting = true;
	for( int i = 0; i < 100000; ++i )
	{
		projMover->Move(1);
		if( projEdge != NULL )
			break;
	}
	if( projEdge == NULL )
	{
		cout << "failed projection!" << endl;
		assert( 0 );
	}
}

void Boss_Tiger::ClearPillars()
{
	FirePillar *active = activePillars;
	while( active != NULL )
	{
		FirePillar *next = active->next;
		DeactivatePillar( active );
		active = next;
	}

	/*for( int i = 0; i < MAX_HOMING; ++i )
	{
		homingVA[i*4+0].position = Vector2f( 0, 0 );
		homingVA[i*4+1].position = Vector2f( 0, 0 );
		homingVA[i*4+2].position = Vector2f( 0, 0 );
		homingVA[i*4+3].position = Vector2f( 0, 0 );
	}*/
}

void Boss_Tiger::ResetEnemy()
{	
	lockPath = NULL;
	//mover->ground = startGround;
	//mover->edgeQuantity = startQuant;
	//mover->UpdateGroundPos();
	action = PLAN;

	for( int i = 0; i < 13; ++i )
	{
		allNodes[i]->numTimesTouched = 0;
	}

	currNode = allNodes[0]; //center;
	currNode->numTimesTouched++;
	//lockPath->node->numTimesTouched++;

	if( mover != NULL )
	{
		mover->ground = startGround;
		mover->edgeQuantity = startQuant;
		mover->roll = false;
		mover->UpdateGroundPos();
	}

	currIndex.x = GRID_SIZE_X / 2;
	currIndex.y = GRID_SIZE_Y / 2;

	flyFrame = 0;
	//fireCounter = 0;
	//launcher->Reset();

	dead = false;
	//dying = false;
	deathFrame = 0;
	frame = 0;

	//position.x = originalPos.x;
	//position.y = originalPos.y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	ClearPillars();
}

void Boss_Tiger::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Tiger::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//b->launcher->DeactivateBullet( b );
}

void Boss_Tiger::BulletHitPlayer(BasicBullet *b )
{
	//owner->player->ApplyHit( b->launcher->hitboxInfo );
}

void Boss_Tiger::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		
	}
}

void Boss_Tiger::UpdatePrePhysics()
{
	ActionEnded();

	//launcher->UpdatePrePhysics();
	mover->SetSpeed( -5 );

	

	
	FirePillar *fp = activePillars;
	while( fp != NULL )
	{
		fp->UpdatePrePhysics();
		fp = fp->next;
	}

	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			//if( hasMonitor && !suppressMonitor )
			//	owner->keyMarker->CollectKey();
			//dying = true;
			//cout << "dying" << endl;
		}

		receivedHit = NULL;
	}

	projecting = false;
}

void Boss_Tiger::HitTerrainAerial(Edge *e, double q)
{
	if( projecting )
	{
		projEdge = e;
		projQuant = q;
	}
}

void Boss_Tiger::TransferEdge( Edge *e )
{

}

void Boss_Tiger::UpdatePhysics()
{	

	if( lockPath == NULL )
	{
		//cout << "pahts: " << currNode->paths.size() << endl;
		for( list<NodePath*>::iterator it = currNode->paths.begin(); 
			it != currNode->paths.end(); ++it )
		{
			if( (*it)->node->numTimesTouched > 0 )
			{
				//cout << "skipping node" << endl;
				continue;
			}
			V2d gn = (*it)->edge->Normal();
			V2d circleCenter = (*it)->edge->GetPoint( (*it)->quant ) + gn * 64.0;
			V2d pos = mover->physBody.globalPosition;

			double len = length( circleCenter - pos );
			if( len < 30 )
			{
				//cout << "lockpath: " << len << endl;
				lockPath = (*it);
			}
			else
			{
				//cout << "not len: " << len << endl;
			}

		}
	}

	if( lockPath != NULL )
	{
		//cout << "lockpath: " << lockPath << endl;
		//cout << "e: " << lockPath->edge << endl;
		V2d norm = lockPath->edge->Normal();
		V2d target = lockPath->pos;//lockPath->edge->GetPoint( lockPath->quant ) + norm * mover->physBody.rw;
		double len = length( mover->physBody.globalPosition - target );
		//cout << "blah len: " << len << ", gspeed: " << mover->groundSpeed << endl;
		if( len < abs( mover->groundSpeed ) )
		{
			mover->ground = lockPath->edge;
			mover->edgeQuantity = lockPath->quant;
			mover->UpdateGroundPos();
			mover->SetSpeed( 0 );
			mover->Jump( normalize( lockPath->node->position - currNode->position ) * 10.0 );
			mover->roll = false;
			currNode = lockPath->node;
			lockPath->node->numTimesTouched++;

			//target = lockPath->node->position;

			lockPath = NULL;
			
		}

		testCS.setPosition( target.x, target.y );
	}

	if( lockPath != NULL )
	{
		if( mover->groundSpeed > 0 )
		{
			
		}
		else if( mover->groundSpeed < 0 )
		{

		}
	}


	mover->Move( 1 );

	//position = mover->po

	position = mover->physBody.globalPosition;


	//if( action == FLY || action == THROWCURVE )
	//{
	//	double a = (double)flyFrame / flyDuration;
	//	double f = flyCurve.GetValue( a );
	//	position = startFly * ( 1.0 - f ) + endFly * ( f );
	//}
	//else if( action == THROWHOMING && frame <= 5 )
	//{
	//	endRing = owner->player->position;
	//	double a = (double)frame / 5;
	//	double f = a;//flyCurve.GetValue( a );
	//	homingPos = startRing * ( 1.0 - f ) + endRing * ( f );
	//}
	
	specterProtected = false;
	if( !dead )
	{
		if( PlayerSlowingMe() )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = 5;
			}
		}
		else
		{
			slowMultiple = 1;
			slowCounter = 1;
		}
	}

	FirePillar *fp = activePillars;
	while( fp != NULL )
	{
		fp->UpdatePhysics();
		fp = fp->next;
	}

	//launcher->UpdatePhysics();

	if( !dead )
	{
		PhysicsResponse();
	}
	return;
}

void Boss_Tiger::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			owner->player->ConfirmHit( 6, 5, .8, 6 );

			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

//			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
		}

		if( IHitPlayer() )
		{
		//	cout << "Boss_Tiger just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Tiger::ConnectNodes()
{
	for( int i = 0; i < 13; ++i )
	{
		allNodes[i] = new Node;
	}

	allNodes[0]->position = V2d( originalPos.x, originalPos.y );

	Vector2f op( originalPos.x, originalPos.y );
	nodeVA[0].position = op + Vector2f( -nodeSize.x / 2, -nodeSize.y / 2 );
	nodeVA[1].position = op + Vector2f( nodeSize.x / 2, -nodeSize.y / 2 );
	nodeVA[2].position = op + Vector2f( nodeSize.x / 2, nodeSize.y / 2 );
	nodeVA[3].position = op + Vector2f( -nodeSize.x / 2, nodeSize.y / 2 );

	nodeVA[0].color = Color::Red;
	nodeVA[1].color = Color::Red;
	nodeVA[2].color = Color::Red;
	nodeVA[3].color = Color::Red;

	Transform t;
	Vector2f offset( 0, -nodeRadius1 );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t.transformPoint( offset ) + Vector2f( originalPos.x, originalPos.y );
		allNodes[i+1]->position = V2d( newP.x, newP.y );
		t.rotate( -360.f / 6.f );

		int j = i + 1;
		nodeVA[j*4+0].position = newP + Vector2f( -nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+1].position = newP + Vector2f( nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+2].position = newP + Vector2f( nodeSize.x / 2, nodeSize.y / 2 );
		nodeVA[j*4+3].position = newP + Vector2f( -nodeSize.x / 2, nodeSize.y / 2 );

		nodeVA[j*4+0].color = Color::Red;
		nodeVA[j*4+1].color = Color::Red;
		nodeVA[j*4+2].color = Color::Red;
		nodeVA[j*4+3].color = Color::Red;
	}

	Transform t2;
	Vector2f offset2( 0, -nodeRadius2 );
	t2.rotate( 360.f / 12.f );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t2.transformPoint( offset2 ) + Vector2f( originalPos.x, originalPos.y );
		allNodes[i+7]->position = V2d( newP.x, newP.y );
		t2.rotate( -360.f / 6.f );

		int j = i + 7;
		nodeVA[j*4+0].position = newP + Vector2f( -nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+1].position = newP + Vector2f( nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+2].position = newP + Vector2f( nodeSize.x / 2, nodeSize.y / 2 );
		nodeVA[j*4+3].position = newP + Vector2f( -nodeSize.x / 2, nodeSize.y / 2 );

		nodeVA[j*4+0].color = Color::Red;
		nodeVA[j*4+1].color = Color::Red;
		nodeVA[j*4+2].color = Color::Red;
		nodeVA[j*4+3].color = Color::Red;
	}

	Node *centerNode = allNodes[0];
	for( int i = 0; i < 6; ++i )
	{
		V2d nextPos = allNodes[i+1]->position;
		V2d thisPos = centerNode->position;

		V2d inBetween = ( nextPos + thisPos ) / 2.0;

		/*rcEdge = NULL;
		rayStart = inBetween;
		rayEnd = thisPos;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		cout << "in towards center: " << i << endl;
		ProjectCircle( inBetween, thisPos );

		//assert( rcEdge != NULL );

		centerNode->paths.push_back( new NodePath( allNodes[i+1], projEdge, projQuant, projMover->physBody.globalPosition ) );
		debugLines[i*2 + 0].position = Vector2f( thisPos.x, thisPos.y );
		debugLines[i*2 + 1].position = Vector2f( nextPos.x, nextPos.y );


		/*rcEdge = NULL;
		rayStart = inBetween;
		rayEnd = nextPos;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		cout << "away from center: " << i << endl;

		ProjectCircle( inBetween, nextPos );
		//assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( centerNode, projEdge, projQuant, projMover->physBody.globalPosition ) );
	}

	//layer 1 and the connections to layer 2
	int debugIndex = 6;
	for( int i = 0; i < 6; ++i )
	{
		int prev = i - 1;
		if( prev < 0 )
			prev += 6;
		int next = i + 1;
		if( next > 5 )
		{
			next -= 6;
		}

		int prev2 = i;
		int next2 = i + 1;
		if( next2 > 5 )
			next2 -= 6;


		V2d a = allNodes[prev+1]->position;
		V2d b = allNodes[i+1]->position;
		V2d c = allNodes[next+1]->position;
		V2d d = allNodes[i+1]->position;
		V2d e = allNodes[prev2+7]->position;
		V2d f = allNodes[i+1]->position;
		V2d g = allNodes[next2+7]->position;
		V2d h = allNodes[i+1]->position;

		V2d midAB = (a+b)/2.0;
		V2d midCD = (c+d)/2.0;
		V2d midEF = (e+f)/2.0;
		V2d midGH = (g+h)/2.0;

		/*rcEdge = NULL;
		rayStart = midAB;
		rayEnd = b;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		ProjectCircle( midAB, b );
		//assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[prev+1], projEdge, projQuant, projMover->physBody.globalPosition ) );

		debugLines[debugIndex * 2 + 0].position = Vector2f( a.x, a.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( b.x, b.y );
		debugIndex++;

		/*rcEdge = NULL;
		rayStart = midAB;
		rayEnd = b;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/

		//assert( rcEdge != NULL );

		//allNodes[prev+1]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		/*debugLines[debugIndex * 2 + 0].position = Vector2f( rayStart.x, rayStart.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( rayEnd.x, rayEnd.y );
		debugIndex++;*/

		/*rcEdge = NULL;
		rayStart = midCD;
		rayEnd = d;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/

		ProjectCircle( midCD, d );
		//assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[next+1], projEdge, projQuant, projMover->physBody.globalPosition ) );

		/*debugLines[debugIndex * 2 + 0].position = Vector2f( c.x, c.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( d.x, d.y );
		debugIndex++;*/

		rcEdge = NULL;
		rayStart = midCD;
		rayEnd = d;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		//assert( rcEdge != NULL );

		//allNodes[next+1]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		/*debugLines[debugIndex * 2 + 0].position = Vector2f( rayStart.x, rayStart.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( rayEnd.x, rayEnd.y );
		debugIndex++;*/

		/*rcEdge = NULL;
		rayStart = midEF;
		rayEnd = f;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		ProjectCircle( midEF, f );
		//assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[prev2+7], projEdge, projQuant, projMover->physBody.globalPosition ) );

		debugLines[debugIndex * 2 + 0].position = Vector2f( e.x, e.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( f.x, f.y );
		debugIndex++;

		rcEdge = NULL;
		rayStart = midEF;
		rayEnd = e;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		//assert( rcEdge != NULL );

		allNodes[prev2+7]->paths.push_back( new NodePath( allNodes[i+1], projEdge, projQuant, projMover->physBody.globalPosition ) );

		/*debugLines[debugIndex * 2 + 0].position = Vector2f( rayStart.x, rayStart.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( rayEnd.x, rayEnd.y );
		debugIndex++;*/

		/*rcEdge = NULL;
		rayStart = midGH;
		rayEnd = h;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		ProjectCircle( midGH, h );
		//assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[next2+7], projEdge, projQuant, projMover->physBody.globalPosition ) );

		debugLines[debugIndex * 2 + 0].position = Vector2f( g.x, g.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( h.x, h.y );
		debugIndex++;

		/*rcEdge = NULL;
		rayStart = midGH;
		rayEnd = g;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/
		ProjectCircle( midGH, g );
		//assert( rcEdge != NULL );

		allNodes[next2+7]->paths.push_back( new NodePath( allNodes[i+1], projEdge, projQuant, projMover->physBody.globalPosition ) );

		/*debugLines[debugIndex * 2 + 0].position = Vector2f( rayStart.x, rayStart.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( rayEnd.x, rayEnd.y );
		debugIndex++;*/
	}

	//cout << "NUM start layer2: " << debugIndex << endl;
	//layer 2 connections to layer 2
	int layer2Start = 7;
	for( int i = 0; i < 6; ++i )
	{
		//int prev = i - 1;
		//if( prev < 0 )
		//	prev += 6;

		int next = i + 1;
		if( next > 5 )
			next -= 6;

		V2d a = allNodes[layer2Start + i]->position;
		V2d b = allNodes[layer2Start + next]->position;

		V2d mid = (a + b)/ 2.0;

		/*rcEdge = NULL;
		rayStart = mid;
		rayEnd = a;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/

		ProjectCircle( mid, a );

		//assert( rcEdge != NULL );

		allNodes[layer2Start + i]->paths.push_back( new NodePath( allNodes[layer2Start+next], projEdge, projQuant, projMover->physBody.globalPosition ) );

		/*rcEdge = NULL;
		rayStart = mid;
		rayEnd = b;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );*/

		ProjectCircle( mid, b );

		allNodes[layer2Start+next]->paths.push_back( new NodePath( allNodes[layer2Start+i], projEdge, projQuant, projMover->physBody.globalPosition ) );
		//allNodes[layer2Start+prev]->paths.push_back( new NodePath( allNodes[layer2Start+i], rcEdge, rcQuantity ) );
		//cout << "debugindex: " << debugIndex << endl;
		debugLines[debugIndex * 2 + 0].position = Vector2f( b.x, b.y );
		debugLines[debugIndex * 2 + 1].position = Vector2f( a.x, a.y );
		debugIndex++;
	}

	for( int i = 0; i < 30; ++i )
	{
		debugLines[i*2+0].color = Color::Red;
		debugLines[i*2+1].color = Color::Blue;
	}

	//cout << "check: " << endl;
	//for( int i = 0; i < 13; ++i )
	//{
	//	cout << i << ": " << allNodes[i]->paths.size() << endl;
	//}
}

void Boss_Tiger::HandleRayCollision( Edge *edge, double equant, double rayPortion )
{
	//give another edge type so that you can differentiate openable
	//gates by unopenable gates
	if( edge->edgeType == Edge::OPEN_GATE )//|| ( edge->edgeType == Edge::CLOSED_GATE	
	{
		return;
	}

	if( rcEdge == NULL || length( edge->GetPoint( equant ) - rayStart ) < 
		length( rcEdge->GetPoint( rcQuantity ) - rayStart ) )
	{
		rcEdge = edge;
		rcQuantity = equant;
	}
}

void Boss_Tiger::UpdatePostPhysics()
{
	//launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
		
	}
	else
	{
		slowCounter++;
	}

	if( dead )
	{
		owner->RemoveEnemy( this );
	}

	FirePillar *fp = activePillars;
	while( fp != NULL )
	{
		fp->UpdatePostPhysics();
		fp = fp->next;
	}


	UpdateSprite();
	//launcher->UpdateSprites();
}

void Boss_Tiger::UpdateSprite()
{
	if( !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );

		

		if( mover != NULL && mover->ground != NULL )
		{
			V2d gn = mover->ground->Normal();
			double angle = atan2( gn.x, -gn.y );
			sprite.setRotation( angle * 180 / PI );
		}
		
		//sprite.setRotation( 
		//SetHoming( position, currHoming, 0 );
		//targeterSprite.setPosition( homingPos.x, homingPos.y );
	}
}

void Boss_Tiger::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{	
		//target->draw( homingVA, ts_homingRing->texture );
		target->draw( nextAttackOrb );
		target->draw( sprite );
		target->draw( nodeVA );
		target->draw( debugLines );
		if( lockPath != NULL )
			target->draw( testCS );

		CircleShape cs;
		cs.setFillColor( Color::Blue );
		cs.setRadius( 20 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		if( currNode != NULL )
		{
		for( list<NodePath*>::iterator it = currNode->paths.begin(); it != currNode->paths.end(); ++it )
		{
			V2d p = (*it)->pos;//(*it)->edge->GetPoint( (*it)->quant ) + (*it)->edge->Normal() * 64.0;
			cs.setPosition( p.x, p.y );
			target->draw( cs );
		}
		}

		target->draw( pillarVA, ts_pillar->texture );
		//punchPulse.Draw( target );
	}
}

void Boss_Tiger::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_BLUE );
		enemyCircle.setRadius( 50 );
		enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
		enemyCircle.setPosition( position.x, position.y );
		target->draw( enemyCircle );

		/*if( hasMonitor && !suppressMonitor )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}*/
	}
}

bool Boss_Tiger::IHitPlayer()
{

	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Tiger::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->player->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Boss_Tiger::PlayerHitMe()
{
	Actor *player = owner->player;
	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player->currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Boss_Tiger::PlayerSlowingMe()
{
	Actor *player = owner->player;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void Boss_Tiger::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Tiger::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Tiger::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}

void Boss_Tiger::DeactivatePillar( FirePillar *hr )
{
	//remove from active list

	assert( activePillar != NULL );

	if( hr->prev == NULL && hr->next == NULL )
	{
		activePillars = NULL;
	}
	else if( hr->prev == NULL )
	{
		activePillars = hr->next;
	}
	else if( hr->next == NULL )
	{
		hr->prev->next = NULL;
		hr->prev = NULL;
	}
	else
	{
		hr->prev->next = hr->next;
		hr->next->prev = hr->prev;
		hr->prev = NULL;
		hr->next = NULL;
	}


	//add to inactive list
	inactivePillars->prev = hr;
	hr->next = inactivePillars;
	inactivePillars = hr;

	hr->Clear();
}

Boss_Tiger::FirePillar * Boss_Tiger::ActivatePillar()
{
	if( inactivePillars == NULL )
		return NULL;
	else
	{
		FirePillar *temp = inactivePillars->next;
		FirePillar *newPillar = inactivePillars;
		inactivePillars = temp;
		inactivePillars->prev = NULL;

		newPillar->Reset( position );


		if( activePillars == NULL )
		{
			activePillars = newPillar;
		}
		else
		{
			activePillars->prev = newPillar;
			newPillar->next = activePillars;
			activePillars = newPillar;
		}

		return newPillar;
	}
}

void Boss_Tiger::AddPillar()
{
	if( inactivePillars == NULL )
	{
		inactivePillars = new FirePillar( this, 0 );
	}
	else
	{
		FirePillar *hr = inactivePillars;
		int numPillars = 0;
		while( hr != NULL )
		{
			numPillars += numPillarTiles;
			hr = hr->next;
		}

		//cout << "adding ring: " << numRings << endl;

		FirePillar *nhr = new FirePillar( this, numPillars );
		nhr->next = inactivePillars;
		inactivePillars->prev = nhr;
		inactivePillars = nhr;
	}
}

void Boss_Tiger::FirePillar::UpdatePrePhysics()
{
	if( (action == DISSIPATE && frame == 60) )
	{
		parent->DeactivatePillar( this );
		return;
	}
	if( action == ACTIVATE && frame == 60 )
	{
		action = ROTATE;
		frame = 0;
	}
	else if( action == ROTATE && frame == waveLengthFrames )
	{
		frame = 0;
	}

	switch( action )
	{
	case FIND:
		{
			cout << "ring find " << frame << endl;
			endRing = parent->owner->player->position;
			double a = (double)frame / 5;
			double f = a;//flyCurve.GetValue( a );
			position = startRing * ( 1.0 - f ) + endRing * ( f );
		}
		break;
	case LOCK:
		{
			position = parent->owner->player->position;
			cout << "ring lock " << frame << endl;
		}
		break;
	case FREEZE:
		break;
	case ACTIVATE:
		break;
	case DISSIPATE:
		break;
	}
}

Boss_Tiger::FirePillar::FirePillar( Boss_Tiger *p_parent, int p_vaIndex )
	:parent( p_parent ), frame( 0 ), next( NULL ), prev( NULL ),
	action( FIND ), vaIndex( p_vaIndex )
{
	hitbox.isCircle = true;
	hitbox.rw = 64;
	hitbox.rh = 64;
}

void Boss_Tiger::FirePillar::UpdatePostPhysics()
{
	IntRect ir = parent->ts_pillar->GetSubRect( 0 );
	int hw = parent->ts_pillar->tileWidth / 2;
	int hh = parent->ts_pillar->tileHeight / 2;
	//parent->ts_homingRing

	int numPillarTiles = parent->numPillarTiles;
	VertexArray &va = parent->pillarVA;

	t = t.Identity;
	t.rotate( currAngle_d );

	for( int i = 0; i < numPillarTiles; ++i )
	{
		Vector2f offset( 0, -(hh*2) * i );
		Vector2f indexPos = Vector2f( position.x, position.y ) 
			+ t.transformPoint( offset );
		int index = vaIndex + i;

		va[index*4+0].position = indexPos + Vector2f( -hw, -hh ); 
		va[index*4+1].position = indexPos + Vector2f( hw, -hh );
		va[index*4+2].position = indexPos + Vector2f( hw, hh );
		va[index*4+3].position = indexPos + Vector2f( -hw, hh );

		va[index*4+0].texCoords = Vector2f( ir.left, ir.top );
		va[index*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		va[index*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		va[index*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
	}
	

	/*parent->homingVA[vaIndex*4+0].color = Color::Green;
	parent->homingVA[vaIndex*4+1].color = Color::Green;
	parent->homingVA[vaIndex*4+2].color = Color::Green;
	parent->homingVA[vaIndex*4+3].color = Color::Green;*/

	++frame;
}

void Boss_Tiger::FirePillar::UpdatePhysics()
{
	Actor *player = parent->owner->player;
	if( player->hurtBody.Intersects( hitbox ) )
	{
		//parent->HomingRingTriggered( this );
	}
}

void Boss_Tiger::FirePillar::Clear()
{
	int pillarTiles = parent->numPillarTiles;
	
	for( int i = 0; i < pillarTiles; ++i )
	{
		int index = vaIndex + i;
		parent->pillarVA[index*4+0].position = Vector2f( 0, 0 );
		parent->pillarVA[index*4+1].position = Vector2f( 0, 0 );
		parent->pillarVA[index*4+2].position = Vector2f( 0, 0 );
		parent->pillarVA[index*4+3].position = Vector2f( 0, 0 );
	}
}

void Boss_Tiger::FirePillar::Reset( sf::Vector2<double> &pos,
	float angle, int p_waveLengthFrames )
{
	waveLengthFrames = p_waveLengthFrames;
	position = pos;

	prev = NULL;
	next = NULL;
}
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
	nodeVA( sf::Quads, 13 * 4 )
{
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
	
	
	

	//current num links is 248	
	//position.x = pos.x;
	//position.y = pos.y;

	ts_homingRing = owner->GetTileset( "bossbird_homing_100x100.png", 100, 100 );
	targeterSprite.setTexture( *ts_homingRing->texture );
	targeterSprite.setTextureRect( ts_homingRing->GetSubRect( 0 ) );
	targeterSprite.setOrigin( targeterSprite.getLocalBounds().width / 2,
		targeterSprite.getLocalBounds().height / 2 );


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
	hitBody.rw = 16;
	hitBody.rh = 16;

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
	
	activeHoming = NULL;
	inactiveHoming = NULL;

	for( int i = 0; i < MAX_HOMING; ++i )
	{
		AddHRing();
	}

	ResetEnemy();
	//UpdateHitboxes();
	
	
}



void Boss_Tiger::ClearHomingRings()
{
	HomingRing *active = activeHoming;
	while( active != NULL )
	{
		HomingRing *next = active->next;
		DeactivateHRing( active );
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
	
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->UpdateGroundPos();
	action = PLAN;

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

	ClearHomingRings();
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

	


	
	HomingRing *hr = activeHoming;
	while( hr != NULL )
	{
		hr->UpdatePrePhysics();
		hr = hr->next;
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
}

void Boss_Tiger::UpdatePhysics()
{	

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

	HomingRing *hr = activeHoming;
	while( hr != NULL )
	{
		hr->UpdatePhysics();
		hr = hr->next;
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
	nodeVA[0].position = op + ( -nodeSize.x / 2, -nodeSize.y / 2 );
	nodeVA[1].position = op + ( nodeSize.x / 2, -nodeSize.y / 2 );
	nodeVA[2].position = op + ( nodeSize.x / 2, nodeSize.y / 2 );
	nodeVA[3].position = op + ( -nodeSize.x / 2, nodeSize.y / 2 );

	nodeVA[0].color = Color::Red;
	nodeVA[1].color = Color::Red;
	nodeVA[2].color = Color::Red;
	nodeVA[3].color = Color::Red;

	Transform t;
	Vector2f offset( 0, -nodeRadius1 );
	for( int i = 0; i < 6; ++i )
	{
		Vector2f newP = t.transformPoint( offset );
		allNodes[i+1]->position = V2d( newP.x, newP.y );
		t.rotate( -360.f / 6.f );

		int j = i + 1;
		nodeVA[j*4+0].position = newP + ( -nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+1].position = newP + ( nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+2].position = newP + ( nodeSize.x / 2, nodeSize.y / 2 );
		nodeVA[j*4+3].position = newP + ( -nodeSize.x / 2, nodeSize.y / 2 );

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
		Vector2f newP = t2.transformPoint( offset2 );
		allNodes[i+7]->position = V2d( newP.x, newP.y );
		t.rotate( -360.f / 6.f );

		int j = i + 7;
		nodeVA[j*4+0].position = newP + ( -nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+1].position = newP + ( nodeSize.x / 2, -nodeSize.y / 2 );
		nodeVA[j*4+2].position = newP + ( nodeSize.x / 2, nodeSize.y / 2 );
		nodeVA[j*4+3].position = newP + ( -nodeSize.x / 2, nodeSize.y / 2 );

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

		rcEdge = NULL;
		rayStart = inBetween;
		rayEnd = thisPos;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		centerNode->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = inBetween;
		rayEnd = nextPos;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( centerNode, rcEdge, rcQuantity ) );
	}

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
		V2d midHG = (g+h)/2.0;

		rcEdge = NULL;
		rayStart = midAB;
		rayEnd = a;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[prev+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midAB;
		rayEnd = b;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[prev+1]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midCD;
		rayEnd = c;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[next+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midCD;
		rayEnd = d;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[next+1]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midEF;
		rayEnd = e;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[prev2+7], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midEF;
		rayEnd = f;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[prev2+7]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midGH;
		rayEnd = g;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[i+1]->paths.push_back( new NodePath( allNodes[next2+7], rcEdge, rcQuantity ) );

		rcEdge = NULL;
		rayStart = midGH;
		rayEnd = h;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		assert( rcEdge != NULL );

		allNodes[next2+7]->paths.push_back( new NodePath( allNodes[i+1], rcEdge, rcQuantity ) );
	}
}

void Boss_Tiger::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
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

	HomingRing *hr = activeHoming;
	while( hr != NULL )
	{
		hr->UpdatePostPhysics();
		hr = hr->next;
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

void Boss_Tiger::DeactivateHRing( HomingRing *hr )
{
	//remove from active list

	assert( activeHoming != NULL );

	if( hr->prev == NULL && hr->next == NULL )
	{
		activeHoming = NULL;
	}
	else if( hr->prev == NULL )
	{
		activeHoming = hr->next;
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
	inactiveHoming->prev = hr;
	hr->next = inactiveHoming;
	inactiveHoming = hr;

	hr->Clear();
}

Boss_Tiger::HomingRing * Boss_Tiger::ActivateHRing()
{
	if( inactiveHoming == NULL )
		return NULL;
	else
	{
		HomingRing *temp = inactiveHoming->next;
		HomingRing *newHoming = inactiveHoming;
		inactiveHoming = temp;
		inactiveHoming->prev = NULL;

		newHoming->Reset( position );


		if( activeHoming == NULL )
		{
			activeHoming = newHoming;
		}
		else
		{
			activeHoming->prev = newHoming;
			newHoming->next = activeHoming;
			activeHoming = newHoming;
		}

		return newHoming;
	}
}

void Boss_Tiger::AddHRing()
{
	if( inactiveHoming == NULL )
	{
		inactiveHoming = new HomingRing( this, 0 );
	}
	else
	{
		HomingRing *hr = inactiveHoming;
		int numRings = 0;
		while( hr != NULL )
		{
			numRings++;
			hr = hr->next;
		}

		//cout << "adding ring: " << numRings << endl;

		HomingRing *nhr = new HomingRing( this, numRings );
		nhr->next = inactiveHoming;
		inactiveHoming->prev = nhr;
		inactiveHoming = nhr;
	}
}

void Boss_Tiger::HomingRing::UpdatePrePhysics()
{
	if( (action == DISSIPATE && frame == 60) )
	{
		parent->DeactivateHRing( this );
		return;
	}
	if( action == FIND && frame == 5 + 1 )
	{
		action = LOCK;
		frame = 0;
	}
	else if( action == LOCK && frame == 60 )
	{
		action = FREEZE;
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

Boss_Tiger::HomingRing::HomingRing( Boss_Tiger *p_parent, int p_vaIndex )
	:parent( p_parent ), frame( 0 ), next( NULL ), prev( NULL ),
	action( FIND ), vaIndex( p_vaIndex )
{
	hitbox.isCircle = true;
	hitbox.rw = 64;
	hitbox.rh = 64;
}

void Boss_Tiger::HomingRing::UpdatePostPhysics()
{
	IntRect ir = parent->ts_homingRing->GetSubRect( 0 );
	int hw = parent->ts_homingRing->tileWidth / 2;
	int hh = parent->ts_homingRing->tileHeight / 2;
	//parent->ts_homingRing
	/*parent->homingVA[vaIndex*4+0].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, -hh ); 
	parent->homingVA[vaIndex*4+1].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, -hh );
	parent->homingVA[vaIndex*4+2].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, hh );
	parent->homingVA[vaIndex*4+3].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, hh );*/

	//parent->homingVA[vaIndex*4+0].texCoords = Vector2f( ir.left, ir.top );
	//parent->homingVA[vaIndex*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	//parent->homingVA[vaIndex*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	//parent->homingVA[vaIndex*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );

	/*parent->homingVA[vaIndex*4+0].color = Color::Green;
	parent->homingVA[vaIndex*4+1].color = Color::Green;
	parent->homingVA[vaIndex*4+2].color = Color::Green;
	parent->homingVA[vaIndex*4+3].color = Color::Green;*/

	++frame;
}

void Boss_Tiger::HomingRing::UpdatePhysics()
{
	Actor *player = parent->owner->player;
	if( player->hurtBody.Intersects( hitbox ) )
	{
		parent->HomingRingTriggered( this );
	}
}

void Boss_Tiger::HomingRingTriggered( HomingRing *hr )
{
	//if( action != KICK )
	//{
	//	action = KICK;
	//	frame = 0;
	//	//kickTargetPos = hr->position;
	//}
}

void Boss_Tiger::HomingRing::Clear()
{
	/*parent->homingVA[vaIndex*4+0].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+1].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+2].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+3].position = Vector2f( 0, 0 );*/
}

void Boss_Tiger::HomingRing::Reset( sf::Vector2<double> &pos )
{
	position = pos;
	startRing = pos;

	prev = NULL;
	next = NULL;
}
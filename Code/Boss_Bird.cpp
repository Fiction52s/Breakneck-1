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



Boss_Bird::Boss_Bird( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::BOSS_BIRD, false, 6 ), deathFrame( 0 ),
	flyCurve( 0, 0, 1, 1 ), punchPulse( owner ),  homingVA( sf::Quads, MAX_HOMING * 4 ),
	debugLines( sf::Lines, GRID_SIZE_X * 2 + GRID_SIZE_Y * 2 )
{
	bulletSpeed = 1;

	originalPos = pos;
	
	gridCenter.x = originalPos.x;
	gridCenter.y = originalPos.y;

	currIndex.x = GRID_SIZE_X / 2;
	currIndex.y = GRID_SIZE_Y / 2;

	gridCellSize.x = 160;
	gridCellSize.y = 80;

	gridOrigin = gridCenter + V2d( -currIndex.x * gridCellSize.x, -currIndex.y * gridCellSize.y );

	actionLength[FLY] = 10;
	actionLength[PUNCHPREPOUT] = 60;
	actionLength[PUNCHPREPIN] = 60;
	actionLength[PUNCH] = 30;
	actionLength[THROWHOMING] = 60;
	actionLength[THROWCURVE] = 30;
	actionLength[KICK] = 30;

	animFactor[FLY] = 1;
	animFactor[PUNCHPREPOUT] = 1;
	animFactor[PUNCHPREPIN] = 1;
	animFactor[PUNCHPREPOUT] = 1;
	animFactor[PUNCH] = 1;
	animFactor[THROWHOMING] = 1;
	animFactor[THROWCURVE] = 1;
	animFactor[KICK] = 1;

	//current num links is 248	
	//position.x = pos.x;
	//position.y = pos.y;

	ts_homingRing = owner->GetTileset( "bossbird_homing_100x100.png", 100, 100 );
	targeterSprite.setTexture( *ts_homingRing->texture );
	targeterSprite.setTextureRect( ts_homingRing->GetSubRect( 0 ) );
	targeterSprite.setOrigin( targeterSprite.getLocalBounds().width / 2,
		targeterSprite.getLocalBounds().height / 2 );


	bulletSpeed = 0;

	receivedHit = NULL;
	
	
	flyDuration = 60;
	

	deathFrame = 0;
	
	launcher = new Launcher( this, BasicBullet::BType::BOSS_BIRD, owner, 64, 1, position, V2d( 1, 0 ), 0, 180, false );
	
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	action = FLY;
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
	int curr = 0;
	for( int i = 0; i < GRID_SIZE_X; ++i )
	{
		debugLines[curr*2+0].position = GetGridPosF( i, 0 );
		debugLines[curr*2+1].position = GetGridPosF( i, GRID_SIZE_Y - 1 );
		++curr;
	}

	for( int i = 0; i < GRID_SIZE_Y; ++i )
	{
		debugLines[curr*2+0].position = GetGridPosF( 0, i );
		debugLines[curr*2+1].position = GetGridPosF( GRID_SIZE_X-1, i );
		++curr;
	}

	int totalLineVertices = GRID_SIZE_X * 2 + GRID_SIZE_Y * 2;
	for( int i = 0; i < totalLineVertices; ++i )
	{
		debugLines[i].color = Color::Green;
	}
}

void Boss_Bird::SetNextAttack()
{
	int r = rand() % 3;

	int xOptions[GRID_SIZE_X-1];
	for( int i = 0; i < GRID_SIZE_X; ++i )
	{
		if( i < currIndex.x )
		{
			xOptions[i] = i;	
		}
		else if( i > currIndex.x )
		{
			xOptions[i-1] = i;
		}
	}

	int yOptions[GRID_SIZE_Y-1];
	for( int i = 0; i < GRID_SIZE_Y; ++i )
	{
		if( i < currIndex.y )
		{
			yOptions[i] = i;	
		}
		else if( i > currIndex.y )
		{
			yOptions[i-1] = i;
		}
	}

	int rx = rand() % (GRID_SIZE_X-1);
	int ry = rand() % (GRID_SIZE_Y-1);

	nextAttackIndex = Vector2i( xOptions[rx], yOptions[ry] );

	nextAttackType = (AttackOrb)r;

	nextAttackOrb.setTextureRect( ts_nextAttackOrb->GetSubRect( nextAttackType ) );
	nextAttackOrb.setPosition( GetGridPosF( nextAttackIndex ) );

	startFly = position;
	endFly = GetGridPosD( nextAttackIndex );

	cout << "chosen index: " << nextAttackIndex.x << ", " << nextAttackIndex.y << endl;
}

void Boss_Bird::ClearHomingRings()
{
	HomingRing *active = activeHoming;
	while( active != NULL )
	{
		HomingRing *next = active->next;
		DeactivateHRing( active );
		active = next;
	}
	activeHoming = NULL;
	/*for( int i = 0; i < MAX_HOMING; ++i )
	{
		homingVA[i*4+0].position = Vector2f( 0, 0 );
		homingVA[i*4+1].position = Vector2f( 0, 0 );
		homingVA[i*4+2].position = Vector2f( 0, 0 );
		homingVA[i*4+3].position = Vector2f( 0, 0 );
	}*/
}

sf::Vector2f Boss_Bird::GetGridPosF( sf::Vector2i &index )
{
	return Vector2f( gridOrigin.x + index.x * gridCellSize.x, 
		gridOrigin.y + index.y * gridCellSize.y );
}

sf::Vector2<double> Boss_Bird::GetGridPosD( sf::Vector2i &index )
{
	return V2d( gridOrigin.x + index.x * gridCellSize.x, 
		gridOrigin.y + index.y * gridCellSize.y );
}

sf::Vector2f Boss_Bird::GetGridPosF( int x, int y )
{
	return Vector2f( gridOrigin.x + x * gridCellSize.x, 
		gridOrigin.y + y * gridCellSize.y );
}

sf::Vector2<double> Boss_Bird::GetGridPosD( int x, int y )
{
	return V2d( gridOrigin.x + x * gridCellSize.x, 
		gridOrigin.y + y * gridCellSize.y );
}

void Boss_Bird::ResetEnemy()
{	
	
	action = FLY;

	currIndex.x = GRID_SIZE_X / 2;
	currIndex.y = GRID_SIZE_Y / 2;

	flyFrame = 0;
	//fireCounter = 0;
	//launcher->Reset();

	dead = false;
	//dying = false;
	deathFrame = 0;
	frame = 0;

	launcher->Reset();



	position = GetGridPosD( currIndex );
	//position.x = originalPos.x;
	//position.y = originalPos.y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	SetNextAttack();
	ClearHomingRings();
}

void Boss_Bird::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Bird::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//b->launcher->DeactivateBullet( b );
}

void Boss_Bird::BulletHitPlayer(BasicBullet *b )
{
	owner->player->ApplyHit( b->launcher->hitboxInfo );
}

void Boss_Bird::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case FLY:
			frame = 0;
			break;
		case PUNCHPREPOUT:
			punchPulse.PulseIn( Vector2i( position.x, position.y ) );
			action = PUNCHPREPIN;
			frame = 0;
			break;
		case THROWHOMING:
		case PUNCHPREPIN:
			SetNextAttack();
			action = FLY;
			flyFrame = 0;
			frame = 0;
			break;
		case PUNCH:
			break;
		case THROWCURVE:
			break;
		case KICK:
			break;
		}
	}
}

void Boss_Bird::UpdatePrePhysics()
{
	ActionEnded();

	V2d playerPos = owner->player->position;
	
	
	
	launcher->maxBulletSpeed = 15;
	launcher->UpdatePrePhysics();

	switch( action )
	{
	case THROWCURVE:
	case FLY:
		if( flyFrame == flyDuration + 1 )
		{
			currIndex.x = nextAttackIndex.x;
			currIndex.y = nextAttackIndex.y;
			switch( nextAttackType )
			{
			case ORB_PUNCH:
				punchPulse.PulseOut( Vector2i( position.x, position.y ) );
				action = PUNCHPREPOUT;
				frame = 0;
				break;
			case ORB_KICK:
				action = THROWHOMING;
				frame = 0;
				break;
			case ORB_THROW:
				SetNextAttack();
				action = THROWCURVE;
				frame = 0;
				flyFrame = 0;
				break;
			}
		}
		break;
	case PUNCHPREPOUT:
		break;
	case PUNCHPREPIN:
		break;
	case PUNCH:
		break;
	case THROWHOMING:
		break;
	
	case KICK:
		break;
	}

	switch( action )
	{
	case FLY:
		cout << "FLY" << endl;
		break;
	case PUNCHPREPOUT:
		cout << "PUNCHPREPOUT" << endl;
		break;
	case PUNCHPREPIN:
		cout << "PUNCHPREPIN" << endl;
		break;
	case PUNCH:
		cout << "PUNCH" << endl;
		break;
	case THROWHOMING:
		if( frame == 1 )
		{
			HomingRing *hr = ActivateHRing();
		}
		cout << "THROWHOMING" << endl;
		break;
	case THROWCURVE:
		{
			cout << "THROWCURVE" << endl;
			if( flyFrame % 20 == 0 )
			{
				cout << "firing launcher!: " << flyFrame << endl;
				launcher->facingDir = normalize( owner->player->position - position );
				launcher->position = position;
				launcher->Fire();
			}
			break;
		}
	case KICK:
		cout << "KICK" << endl;
		break;
	}
	
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

void Boss_Bird::UpdatePhysics()
{	

	if( action == FLY || action == THROWCURVE )
	{
		double a = (double)flyFrame / flyDuration;
		double f = flyCurve.GetValue( a );
		position = startFly * ( 1.0 - f ) + endFly * ( f );
	}
	else if( action == THROWHOMING && frame <= 5 )
	{
		endRing = owner->player->position;
		double a = (double)frame / 5;
		double f = a;//flyCurve.GetValue( a );
		homingPos = startRing * ( 1.0 - f ) + endRing * ( f );
	}
	
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

	launcher->UpdatePhysics();

	if( !dead )
	{
		PhysicsResponse();
	}
	return;
}

void Boss_Bird::PhysicsResponse()
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
		//	cout << "Boss_Bird just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Bird::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
		punchPulse.Update();
		++homingFrame;

		switch( action )
		{	
		case THROWHOMING:	
		case FLY:
			++flyFrame;
			break;
		case PUNCHPREPOUT:
			break;
		case PUNCHPREPIN:
			break;
		case PUNCH:
			break;
		
		case THROWCURVE:
			++flyFrame;
			break;
		case KICK:
			break;
		}
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


	punchPulse.UpdatePoints();
	UpdateSprite();
	//launcher->UpdateSprites();
}

void Boss_Bird::UpdateSprite()
{
	if( !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );
		launcher->UpdateSprites();
		//SetHoming( position, currHoming, 0 );
		//targeterSprite.setPosition( homingPos.x, homingPos.y );
	}
}

void Boss_Bird::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{	
		target->draw( homingVA, ts_homingRing->texture );
		target->draw( nextAttackOrb );
		target->draw( sprite );
		punchPulse.Draw( target );
		target->draw( debugLines );
	}
}

void Boss_Bird::DrawMinimap( sf::RenderTarget *target )
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

bool Boss_Bird::IHitPlayer( int index )
{

	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Bird::UpdateHitboxes()
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
pair<bool,bool> Boss_Bird::PlayerHitMe( int index )
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

bool Boss_Bird::PlayerSlowingMe()
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

void Boss_Bird::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Bird::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Bird::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}

void Boss_Bird::DeactivateHRing( HomingRing *hr )
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

Boss_Bird::HomingRing * Boss_Bird::ActivateHRing()
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

void Boss_Bird::AddHRing()
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

Boss_Bird::PunchPulse::PunchPulse( GameSession *p_owner )
	:owner( p_owner ), circleVA( sf::Quads, CIRCLEPOINTS * 4 ), innerRadius( 10 ), 
	outerRadius( 50 ), show( false ), frame( 0 ), pulseLength( 60 ),
	maxOuterRadius( 200 ), maxInnerRadius( 160 )
{
	minInnerRadius = innerRadius;
	minOuterRadius = outerRadius;
	UpdatePoints();	
}

void Boss_Bird::PunchPulse::Reset()
{
	frame = 0;
	show = false;
}

void Boss_Bird::PunchPulse::Update()
{
	if( show )
	{
		if( frame == pulseLength + 1 )
		{
			frame = 0;
			show = false;
		}
		else
		{
			CubicBezier innerBez( 0, 0, 1, 1 );
			CubicBezier outerBez( 0, 0, 1, 1 );

			double innerQ = innerBez.GetValue( frame / (double)pulseLength );
			double outerQ = outerBez.GetValue( frame / (double)pulseLength );

			if( !out )
			{
				innerQ = 1.0 - innerQ;
				outerQ = 1.0 - outerQ;
			}

			innerRadius = minInnerRadius * (1.0 - innerQ) + maxInnerRadius * innerQ;
			outerRadius = minOuterRadius * (1.0 - outerQ) + maxOuterRadius * outerQ;

			++frame;
		}

		UpdatePoints();
	}
}

void Boss_Bird::PunchPulse::UpdatePoints()
{
	/*Transform t;
	float angle = 0;
	for( int i = 0; i < circlePoints; ++i )
	{
		circleVA[i*4+0].position = 
	}*/
	//cout <<  CIRCLEPOINTS << endl;
	if( show )
	{
	Transform tr;
	Vector2f offsetInner( 0, -innerRadius );
	Vector2f offsetOuter( 0, -outerRadius );
	for( int i = 0; i < CIRCLEPOINTS; ++i )
	{ 
		circleVA[i*4+0].position = position + tr.transformPoint( offsetInner );  
		circleVA[i*4+1].position = position + tr.transformPoint( offsetOuter ); 
		
		tr.rotate( 360.f / CIRCLEPOINTS );

		circleVA[i*4+2].position = position + tr.transformPoint( offsetOuter ); 
		circleVA[i*4+3].position = position + tr.transformPoint( offsetInner );

		circleVA[i*4+0].color = Color::Cyan;
		circleVA[i*4+1].color = Color::Cyan;
		circleVA[i*4+2].color = Color::Cyan;
		circleVA[i*4+3].color = Color::Cyan;
	}
	}

	
}

void Boss_Bird::PunchPulse::Draw( sf::RenderTarget *target )
{
	if( show )
	{
		target->draw( circleVA );
	}
	
}

void Boss_Bird::PunchPulse::PulseOut( sf::Vector2i &pos )
{
	show = true;
	out = true;
	frame = 0;
	position.x = pos.x;
	position.y = pos.y;
}

void Boss_Bird::PunchPulse::PulseIn( sf::Vector2i &pos )
{
	show = true;
	out = false;
	frame = 0;
	position.x = pos.x;
	position.y = pos.y;
}

void Boss_Bird::HomingRing::UpdatePrePhysics()
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

Boss_Bird::HomingRing::HomingRing( Boss_Bird *p_parent, int p_vaIndex )
	:parent( p_parent ), frame( 0 ), next( NULL ), prev( NULL ),
	action( FIND ), vaIndex( p_vaIndex )
{
	hitbox.isCircle = true;
	hitbox.rw = 64;
	hitbox.rh = 64;
}

void Boss_Bird::HomingRing::UpdatePostPhysics()
{
	IntRect ir = parent->ts_homingRing->GetSubRect( 0 );
	int hw = parent->ts_homingRing->tileWidth / 2;
	int hh = parent->ts_homingRing->tileHeight / 2;
	//parent->ts_homingRing
	parent->homingVA[vaIndex*4+0].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, -hh ); 
	parent->homingVA[vaIndex*4+1].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, -hh );
	parent->homingVA[vaIndex*4+2].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, hh );
	parent->homingVA[vaIndex*4+3].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, hh );

	parent->homingVA[vaIndex*4+0].texCoords = Vector2f( ir.left, ir.top );
	parent->homingVA[vaIndex*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	parent->homingVA[vaIndex*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	parent->homingVA[vaIndex*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );

	/*parent->homingVA[vaIndex*4+0].color = Color::Green;
	parent->homingVA[vaIndex*4+1].color = Color::Green;
	parent->homingVA[vaIndex*4+2].color = Color::Green;
	parent->homingVA[vaIndex*4+3].color = Color::Green;*/

	++frame;
}

void Boss_Bird::HomingRing::UpdatePhysics()
{
	Actor *player = parent->owner->player;
	if( player->hurtBody.Intersects( hitbox ) )
	{
		parent->HomingRingTriggered( this );
	}
}

void Boss_Bird::HomingRingTriggered( HomingRing *hr )
{
	if( action != KICK )
	{
		action = KICK;
		frame = 0;
		kickTargetPos = hr->position;
	}
}

void Boss_Bird::HomingRing::Clear()
{
	parent->homingVA[vaIndex*4+0].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+1].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+2].position = Vector2f( 0, 0 );
	parent->homingVA[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void Boss_Bird::HomingRing::Reset( sf::Vector2<double> &pos )
{
	position = pos;
	startRing = pos;

	prev = NULL;
	next = NULL;
}
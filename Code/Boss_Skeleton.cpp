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



Boss_Skeleton::Boss_Skeleton( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::TURTLE, false, 6 ), deathFrame( 0 ),
	DOWN( 0, 1 ), LEFT( -1, 0 ), RIGHT( 1, 0 ), UP( 0, -1 ), pathVA( sf::Quads, MAX_PATH_SIZE * 4 ),
	flowerVA( sf::Quads, 200 * 4 ), linkVA( sf::Quads, 248 * 4 ),
	swingNodeVA( sf::Quads, 3 * 4 ), holdNodeVA( sf::Quads, 6 * 4 )
{

	

	actionLength[PLANSWING] = 10;
	actionLength[SWING] = 1;
	actionLength[PAUSE_SWING] = 30;

	//current num links is 248	
	position.x = pos.x;
	position.y = pos.y;
	for( int i = 0; i < 200 * 4; ++i )
	{
		flowerVA[i].position = Vector2f( 0, 0 );
		flowerVA[i].color = Color::Red;

		
	}

	for( int i = 0; i < 248 * 4; ++i )
	{
		linkVA[i].position = Vector2f( 0, 0 );
		linkVA[i].color = Color::Green;
	}

	testNumLinks = 0;
	testIndex = 0;
	testLength = 32;
	CreateQuadrant();

	do
	{
	//	cout << "START ITERATION----- " << i << endl;
		ClearPathVA();
		FlowerNode *fn = CreatePath();
	//	cout << "FINISHED ITERATION----- " << i << endl;
		if( fn != NULL )
			break;
	}
	while( true );

	bulletSpeed = 5;

	
	//action = PAT_PLANMOVE;

	skeletonFightSeq = new SkeletonFightSeq( owner );

	fireCounter = 0;
	receivedHit = NULL;
	

	originalPos = pos;

	deathFrame = 0;
	
	//launcher = new Launcher( this, owner, 12, 12, position, V2d( 1, 0 ), 2 * PI, 90, true );
	//launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	action = PLANSWING;
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Boss_Skeleton.png", 80, 80 );
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );


	//position.x = 0;
	//position.y = 0;
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
	dying = false;


	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	maxSwingPlanSize = 10;
	swingPlanLength = 5;
	numSwingNodes = 3;
	numHoldNodes = 6;
	nodeSpread.x = 300;
	nodeSpread.y = 400;

	swingNodeRadius = 32;
	holdNodeRadius = 32;

	SetupMovementNodes();

	UpdateHitboxes();

	currHoldIndex = 0;
	planIndex = 0;
	//nextHoldIndex = GetNextHoldIndex( currHoldIndex, swingPlan[planIndex] );
	//cout << "finish init" << endl;
}

void Boss_Skeleton::GeneratePlan()
{
	for( int i = 0; i < swingPlanLength; ++i )
	{
		swingPlan[i] = rand() % 3;
	}
}

int Boss_Skeleton::GetNextHoldIndex( int currI,
	int swingI )
{
	int plan = swingI;
	if( currI == 0 )
	{
		if( swingI == 0 )
		{
			return 2;
		}
		else if( swingI == 1 )
		{
			return 4;
		}
		else if( swingI == 2 )
		{
			return 5;
		}
	}
	else if( currI == 1 )
	{
		if( swingI == 0 )
		{
			return 3;
		}
		else if( swingI == 1 )
		{
			return 5;
		}
		else if( swingI == 2 )
		{
			return 2;
		}
	}
	else if( currI == 2 )
	{
		if( swingI == 0 )
		{
			return 1;
		}
		else if( swingI == 1 )
		{
			return 3;
		}
		else if( swingI == 2 )
		{
			return 5;
		}
	}
	else if( currI == 3 )
	{
		if( swingI == 0 )
		{
			return 0;
		}
		else if( swingI == 1 )
		{
			return 2;
		}
		else if( swingI == 2 )
		{
			return 4;
		}
	}
	else if( currI == 4 )
	{
		if( swingI == 0 )
		{
			return 1;
		}
		else if( swingI == 1 )
		{
			return 0;
		}
		else if( swingI == 2 )
		{
			return 2;
		}
	}
	else if( currI == 5 )
	{
		if( swingI == 0 )
		{
			return 2;
		}
		else if( swingI == 1 )
		{
			return 1;
		}
		else if( swingI == 2 )
		{
			return 3;
		}
	}
}

void Boss_Skeleton::SetupMovementNodes()
{
	V2d topMiddlePos = position;

	double left = position.x - nodeSpread.x * 2;
	double top = position.y;

	holdNodePos[0] = V2d( left, top + nodeSpread.y );
	holdNodePos[1] = V2d( left, top + 2 * nodeSpread.y );

	holdNodePos[2] = V2d( left + 2 * nodeSpread.x , top + nodeSpread.y );
	holdNodePos[3] = V2d( left + 2 * nodeSpread.x, top + 2 * nodeSpread.y );
	holdNodePos[4] = V2d( left + 4 * nodeSpread.x, top + nodeSpread.y );
	holdNodePos[5] = V2d( left + 4 * nodeSpread.x, top + 2 * nodeSpread.y );

	swingNodePos[0] = V2d( left + nodeSpread.x, top );
	swingNodePos[1] = V2d( left + 2 * nodeSpread.x, top );
	swingNodePos[2] = V2d( left + 3 * nodeSpread.x, top );

	for( int i = 0; i < swingPlanLength; ++i )
	{
		swingPlan[i] = rand() % numSwingNodes;
	}

	for( int i = 0; i < 3; ++i )
	{
		V2d sp = swingNodePos[i];
		swingNodeVA[i*4+0].position = Vector2f( sp.x - swingNodeRadius,
			sp.y - swingNodeRadius );
		swingNodeVA[i*4+1].position = Vector2f( sp.x + swingNodeRadius,
			sp.y - swingNodeRadius );
		swingNodeVA[i*4+2].position = Vector2f( sp.x + swingNodeRadius,
			sp.y + swingNodeRadius );
		swingNodeVA[i*4+3].position = Vector2f( sp.x - swingNodeRadius,
			sp.y + swingNodeRadius );
	}

	for( int i = 0; i < 6; ++i )
	{
		V2d hp = holdNodePos[i];
		holdNodeVA[i*4+0].position = Vector2f( hp.x - holdNodeRadius,
			hp.y - holdNodeRadius );
		holdNodeVA[i*4+1].position = Vector2f( hp.x + holdNodeRadius,
			hp.y - holdNodeRadius );
		holdNodeVA[i*4+2].position = Vector2f( hp.x + holdNodeRadius,
			hp.y + holdNodeRadius );
		holdNodeVA[i*4+3].position = Vector2f( hp.x - holdNodeRadius,
			hp.y + holdNodeRadius );
	}

	swingTopSideDurations[0] = 60; //A to C
	swingTopSideDurations[1] = 60; //A to E
	swingTopSideDurations[2] = 60; //A to F

	swingBotSideDurations[0] = 60;
	swingBotSideDurations[1] = 60;
	swingBotSideDurations[2] = 60;

	swingTopMidDurations[0] = 60;
	swingTopMidDurations[1] = 60;
	swingTopMidDurations[2] = 60;

	swingBotMidDurations[0] = 60;
	swingBotMidDurations[1] = 60;
	swingBotMidDurations[2] = 60;

	seq2To3.AddLineMovement( holdNodePos[2], holdNodePos[3], CubicBezier( .24,.96,.63,.91 ), swingTopMidDurations[1] );
	seq3To2.AddLineMovement( holdNodePos[3], holdNodePos[2], CubicBezier( .24,.96,.63,.91 ), swingBotMidDurations[1] );

	//from 0-------------------


	//0 to 2
	V2d dir = holdNodePos[0] - swingNodePos[0];
	double radius = length( dir );
	dir = normalize( dir );
	V2d dirDest = normalize( holdNodePos[2] - swingNodePos[0] );
	
	double angleStart = atan2( dir.y, dir.x );
	double angleEnd = atan2( dirDest.y, dirDest.x );
	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq0To2.AddRadialMovement( swingNodePos[0], radius, angleStart, angleEnd, false, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingTopSideDurations[0] );

	//0 to 4
	dir =  holdNodePos[0] - swingNodePos[1];
	radius = length( dir );
	dir = normalize( dir );

	dirDest = normalize( holdNodePos[4] - swingNodePos[1] );

	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq0To4.AddRadialMovement( swingNodePos[1], radius, angleStart, angleEnd,false, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//0 to 5 missing
	seq0To5.AddLineMovement( holdNodePos[0], holdNodePos[5], CubicBezier( 0, 0, 1, 1 ), swingTopMidDurations[1] );

	//from 1 ------------------------
	
	//1 to 2 missing
	seq1To2.AddLineMovement( holdNodePos[1], holdNodePos[2], CubicBezier( 0, 0, 1, 1 ), swingTopMidDurations[1] );

	//1 to 3
	dir = holdNodePos[1] - swingNodePos[0];
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[3] - swingNodePos[0] );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq1To3.AddRadialMovement( swingNodePos[0], radius, angleStart, angleEnd, false,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//1 to 5
	dir =  holdNodePos[1] - swingNodePos[1];
	radius = length( dir );
	dir = normalize( dir );

	dirDest = normalize( holdNodePos[5] - swingNodePos[1] );

	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq1To5.AddRadialMovement( swingNodePos[1], radius, angleStart, angleEnd,false, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//from 2--------------

	//2 to 1
	V2d mid = ( holdNodePos[1] + holdNodePos[2] ) / 2.0;
	V2d along = normalize( holdNodePos[2] - holdNodePos[1] );
	V2d other( along.y, -along.x );
	LineIntersection li = lineIntersection( holdNodePos[1], V2d( holdNodePos[1].x, holdNodePos[1].y - 1.0 ), 
		mid, mid + other );//SegmentIntersect( topLeft, topRight, edges[i]->v0, edges[i]->v1 );

	V2d center = li.position;
	//LineIntersection li = 
	dir = holdNodePos[2] - center;
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[1] - center );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq2To1.AddRadialMovement( center, radius, angleStart, angleEnd, true,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//2 to 3 already done

	//2 to 5
	mid = ( holdNodePos[5] + holdNodePos[2] ) / 2.0;
	along = normalize( holdNodePos[2] - holdNodePos[5] );
	other = V2d( along.y, -along.x );
	li = lineIntersection( holdNodePos[5], V2d( holdNodePos[5].x, holdNodePos[5].y - 1.0 ), 
		mid, mid + other );//SegmentIntersect( topLeft, topRight, edges[i]->v0, edges[i]->v1 );

	center = li.position;
	//LineIntersection li = 
	dir = holdNodePos[2] - center;
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[5] - center );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq2To5.AddRadialMovement( center, radius, angleStart, angleEnd, false,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//from 3-----------------

	//3 to 0
	mid = ( holdNodePos[0] + holdNodePos[3] ) / 2.0;
	along = normalize( holdNodePos[3] - holdNodePos[0] );
	other = V2d( along.y, -along.x );
	li = lineIntersection( holdNodePos[3], V2d( holdNodePos[3].x, holdNodePos[3].y - 1.0 ), 
		mid, mid + other );//SegmentIntersect( topLeft, topRight, edges[i]->v0, edges[i]->v1 );

	center = li.position;
	//LineIntersection li = 
	dir = holdNodePos[3] - center;
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[0] - center );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq3To0.AddRadialMovement( center, radius, angleStart, angleEnd, true,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//3 to 2 already done

	//3 to 4
	mid = ( holdNodePos[4] + holdNodePos[3] ) / 2.0;
	along = normalize( holdNodePos[4] - holdNodePos[3] );
	other = V2d( along.y, -along.x );
	li = lineIntersection( holdNodePos[3], V2d( holdNodePos[3].x, holdNodePos[3].y - 1.0 ), 
		mid, mid + other );//SegmentIntersect( topLeft, topRight, edges[i]->v0, edges[i]->v1 );

	center = li.position;
	//LineIntersection li = 
	dir = holdNodePos[3] - center;
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[4] - center );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq3To4.AddRadialMovement( center, radius, angleStart, angleEnd, false,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//from 4------------------

	//4 to 0
	dir =  holdNodePos[4] - swingNodePos[1];
	radius = length( dir );
	dir = normalize( dir );

	dirDest = normalize( holdNodePos[0] - swingNodePos[1] );

	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq4To0.AddRadialMovement( swingNodePos[1], radius, angleStart, angleEnd,true, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//4 to 1 missing. similar to 0 to 5
	seq4To1.AddLineMovement( holdNodePos[4], holdNodePos[1], CubicBezier( 0, 0, 1, 1 ), swingTopMidDurations[1] );

	//4 to 2
	dir =  holdNodePos[4] - swingNodePos[2];
	radius = length( dir );
	dir = normalize( dir );

	dirDest = normalize( holdNodePos[2] - swingNodePos[2] );

	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq4To2.AddRadialMovement( swingNodePos[2], radius, angleStart, angleEnd,true, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingTopSideDurations[2] );


	//5 to 1
	dir =  holdNodePos[5] - swingNodePos[1];
	radius = length( dir );
	dir = normalize( dir );

	dirDest = normalize( holdNodePos[1] - swingNodePos[1] );

	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq5To1.AddRadialMovement( swingNodePos[1], radius, angleStart, angleEnd,true, 
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	//5 to 2 missing
	seq5To2.AddLineMovement( holdNodePos[5], holdNodePos[2], CubicBezier( 0, 0, 1, 1 ), swingTopMidDurations[1] );

	//5 to 3
	dir = holdNodePos[5] - swingNodePos[2];
	radius = length( dir );
	dir = normalize( dir );
	dirDest = normalize( holdNodePos[3] - swingNodePos[2] );
	
	angleStart = atan2( dir.y, dir.x );
	angleEnd = atan2( dirDest.y, dirDest.x );

	if( angleStart < 0 )
		angleStart += 2 * PI;
	if( angleEnd < 0 )
		angleEnd += 2 * PI;

	seq5To3.AddRadialMovement( swingNodePos[2], radius, angleStart, angleEnd, true,
		V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1 ), swingBotSideDurations[1] );

	

	

	
}

void Boss_Skeleton::ResetEnemy()
{
	do
	{
	//	cout << "START ITERATION----- " << i << endl;
		ClearPathVA();
		FlowerNode *fn = CreatePath();
	//	cout << "FINISHED ITERATION----- " << i << endl;
		if( fn != NULL )
			break;
	}
	while( true );

	testIndex = 0;
	//CreateQuadrant();

	action = PLANSWING;
	//action = PAT_PLANMOVE;
	//ClearPathVA();

	

	fireCounter = 0;
	//launcher->Reset();

	dead = false;
	dying = false;
	deathFrame = 0;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Boss_Skeleton::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Skeleton::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//b->launcher->DeactivateBullet( b );
}

void Boss_Skeleton::BulletHitPlayer(BasicBullet *b )
{
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
}

int Boss_Skeleton::GetSwingDuration( int currI, int swingI )
{
	if( currI == 0 || currI == 4 )
	{
		return swingTopSideDurations[swingI];
	}
	else if( currI == 1 || currI == 5 )
	{
		return swingBotSideDurations[swingI];
	}
	else if( currI == 2 )
	{
		return swingTopMidDurations[swingI];
	}
	else if( currI == 3 )
	{
		return swingBotMidDurations[swingI];
	}
}

void Boss_Skeleton::ActionEnded()
{
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case PLANSWING:
			{
			action = SWING;

			frame = 0;
			planIndex = 0;

			

			GeneratePlan();
			
			nextHoldIndex = GetNextHoldIndex( currHoldIndex, swingPlan[planIndex] );

			currSwingDuration = GetSwingDuration( currHoldIndex, swingPlan[planIndex] );

			SetMovementSeq();

			++planIndex;
			break;
			}
		case SWING:
		//	frame = 0;
			break;
		case PAUSE_SWING:
			action = SWING;
			frame = 0;
			break;
		}
	}
}

void Boss_Skeleton::UpdatePrePhysics()
{
	ActionEnded();

	//launcher->UpdatePrePhysics();

	if( action == SWING && frame == currSwingDuration )
	{
		action = PAUSE_SWING;
		frame = 0;
		position = holdNodePos[nextHoldIndex];
		currHoldIndex = nextHoldIndex;
	}

	switch( action )
	{
	case PAT_PLANMOVE:
		break;
	case PAT_MOVE:
		break;
	case PAT_SHOOT:
		break;
	case PLANSWING:
		break;
	case PAUSE_SWING:
		{
			if( frame == 0 )
			{
				if( planIndex == swingPlanLength )
				{
					action = PLANSWING;
					frame = 0;
				}
				else
				{
					nextHoldIndex = GetNextHoldIndex( currHoldIndex, swingPlan[planIndex] );

					SetMovementSeq();

					++planIndex;
				}
			}
			break;
		}
	case SWING:
		{
			
			break;
		}
		
	}

	switch( action )
	{
	case PAT_PLANMOVE:
		break;
	case PAT_MOVE:
		break;
	case PAT_SHOOT:
		break;
	case PLANSWING:
		{
			
		}
		break;
	case PAUSE_SWING:
		{
			
		}
		break;
	case SWING:
		{
			break;
		}
		
	}
	

	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dying = true;
			//cout << "dying" << endl;
		}

		receivedHit = NULL;
	}
}

void Boss_Skeleton::SetMovementSeq()
{
	switch( currHoldIndex )
	{
	case 0:
		if( nextHoldIndex == 2 )
		{
			currSequence = &seq0To2;
		}
		else if( nextHoldIndex == 4 )
		{
			currSequence = &seq0To4;
		}
		else if( nextHoldIndex == 5 )
		{
			currSequence = &seq0To5;
		}
		currSequence->Reset();
		break;
	case 1:
		if( nextHoldIndex == 2 )
		{
			currSequence = &seq1To2;
		}
		else if( nextHoldIndex == 3 )
		{
			currSequence = &seq1To3;
		}
		else if( nextHoldIndex == 5 )
		{
			currSequence = &seq1To5;
		}
		currSequence->Reset();
		break;
	case 2:
		if( nextHoldIndex == 1 )
		{
			currSequence = &seq2To1;
		}
		else if( nextHoldIndex == 3 )
		{
			currSequence = &seq2To3;
		}
		else if( nextHoldIndex == 5 )
		{
			currSequence = &seq2To5;
		}
		currSequence->Reset();
		break;
	case 3:
		if( nextHoldIndex == 0 )
		{
			currSequence = &seq3To0;
		}
		else if( nextHoldIndex == 2 )
		{
			currSequence = &seq3To2;
		}
		else if( nextHoldIndex == 4 )
		{
			currSequence = &seq3To4;
		}
		currSequence->Reset();
		break;
	case 4:
		if( nextHoldIndex == 0 )
		{
			currSequence = &seq4To0;
		}
		else if( nextHoldIndex == 1 )
		{
			currSequence = &seq4To1;
		}
		else if( nextHoldIndex == 2 )
		{
			currSequence = &seq4To2;
		}
		currSequence->Reset();
		break;
	case 5:
		if( nextHoldIndex == 1 )
		{
			currSequence = &seq5To1;
		}
		else if( nextHoldIndex == 2 )
		{
			currSequence = &seq5To2;
		}
		else if( nextHoldIndex == 3 )
		{
			currSequence = &seq5To3;
		}
		currSequence->Reset();
		break;
	}

	cout << "seq: " << currHoldIndex << ", " << nextHoldIndex << endl;
	/*else
	{
		currSequence = NULL;
	}*/
}

void Boss_Skeleton::UpdatePhysics()
{	
	if( action == SWING )
	{
		if( currSequence == NULL )
		{
			V2d startPos = holdNodePos[currHoldIndex];
			V2d endPos = holdNodePos[nextHoldIndex];

			//cout << "curr: " << currHoldIndex << ", next: " << nextHoldIndex << endl;

			int extDur = currSwingDuration * NUM_STEPS * 5;
			int currTime = frame * 5 * NUM_STEPS + owner->substep;
			double a = (double)currTime / extDur;

			CubicBezier b( 0, 0, 1, 1 );
			double z = b.GetValue( a );
			position = startPos * ( 1.0 - z ) + endPos * ( z );
		}
		else
		{
			currSequence->Update( slowMultiple );
			//if( currSequence-
			position = currSequence->position;
		}
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



	//launcher->UpdatePhysics();

	if( !dead && !dying )
	{
		PhysicsResponse();
	}
	return;
}

void Boss_Skeleton::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			owner->GetPlayer( 0 )->ConfirmHit( 6, 5, .8, 6 );

			if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
			{
				owner->GetPlayer( 0 )->velocity.y = 4;//.5;
			}

//			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
		}

		if( IHitPlayer() )
		{
		//	cout << "Boss_Skeleton just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Skeleton::ClearPathVA()
{
	for( int i = 0; i < MAX_PATH_SIZE * 4; ++i )
	{
		pathVA[i].color = Color::Blue;
		pathVA[i].position = Vector2f( 0, 0 );
	}
}

void Boss_Skeleton::UpdatePostPhysics()
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
		++fireCounter;
	
		if( dying )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( deathFrame == 60 && dying )
	{
		dying = false;
		dead = true;
	}

	if( dead )//&& launcher->GetActiveCount() == 0 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	//launcher->UpdateSprites();
}

void Boss_Skeleton::UpdateSprite()
{
	if( !dying && !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );
	}
	if( dying )
	{

		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 1 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	}
}

void Boss_Skeleton::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );

			cs.setFillColor( Color::Black );

			//cs.setFillColor( monitor-> );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( swingNodeVA );
		target->draw( holdNodeVA );
		
		target->draw( sprite );

		
		if( action == PAT_PLANMOVE ||
			action == PAT_MOVE || action == PAT_SHOOT )
		{
			target->draw( flowerVA );
			target->draw( linkVA );
			target->draw( pathVA );
		}
		
		//target->draw( pathVA );

		/*if( action == PAT_PLANMOVE )
		{
			target->draw( testFinalCircle );
			target->draw( testCircle );
			
		}*/
	}
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );*/
		}
		
		target->draw( topDeathSprite );
	}

	

}

void Boss_Skeleton::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
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

bool Boss_Skeleton::IHitPlayer( int index )
{

	Actor *player = owner->GetPlayer( 0 );
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Skeleton::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Boss_Skeleton::PlayerHitMe( int index )
{
	Actor *player = owner->GetPlayer( 0 );
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

bool Boss_Skeleton::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
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

void Boss_Skeleton::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Skeleton::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Skeleton::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}

bool Boss_Skeleton::DirIsValid( sf::Vector2i &testIndex,
	sf::Vector2i &testDir )
{
	Vector2i temp = testIndex + testDir;
	if( temp.x >= 0 && temp.x < GRID_SIZE && temp.y >= 0 && temp.y < GRID_SIZE )
	{
		return true;
	}
	else
	{
		return false;
	}
}

Boss_Skeleton::FlowerNode * Boss_Skeleton::CreatePath()
{
	int numNodes = 0;
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++ j )
		{
			
			//cout << "testing : " << i << ", " << j << endl;
			FlowerNode *fn = nodes[i][j];

			if( fn == NULL )
				continue; 

			for( int k = 0; k < 3; ++k )
			{
				fn->connectsOpen[k] = true;
			}

			//cout << "making not part of path: " << fn->position.x << ", " << fn->position.y << endl;
			fn->partOfPath = false;
			

			if( fn->numConnects == 3 )
			{
				int r = rand() % 3;
				fn->connectsOpen[r] = false;
			}
			else if( fn->numConnects == 2 )
			{
				//entrance or exit node
				//int r = rand() % 2;
				//fn->connectsOpen[r] = false;
			}
			else
			{
				continue;
				//assert( 0 );
			}
		}
	}

	FlowerNode *prev = NULL;
	FlowerNode *curr = nodes[1 + HALF_GRID][0 + HALF_GRID];

	for( int i = 0; i < MAX_PATH_SIZE; ++i )
	{
		//cout << "start wat--------------------------------" << endl;
		
		if( curr->partOfPath )
		{
			cout << i << " part of path already?: curr: " << curr->position.x << ", " << curr->position.y << endl;
			//return NULL;
			break;
		}

		//cout << "adding to path: " << curr->position.x << ", " << curr->position.y << endl;
		path[numNodes] = curr;
		curr->partOfPath = true;


		if( prev == NULL )
		{
		}
		else
		{
			//cout << "thing: " << i << endl;
			Vector2i blah = curr->position - prev->position;
			V2d along = normalize( V2d( blah.x, blah.y ) );
			V2d other( along.y, -along.x );
			V2d start = V2d( prev->position.x, prev->position.y );
			V2d end = V2d( curr->position.x, curr->position.y );
			double w = 5;
			V2d sl = start + other * w;
			V2d sr = start - other * w;
			V2d el = end + other * w;
			V2d er = end - other * w;

			pathVA[(i-1) * 4 + 0].position = Vector2f( sl.x, sl.y );
			pathVA[(i-1) * 4 + 1].position = Vector2f( el.x, el.y );
			pathVA[(i-1) * 4 + 2].position = Vector2f( er.x, er.y );
			pathVA[(i-1) * 4 + 3].position = Vector2f( sr.x, sr.y );
		}
		
		if( curr->dest )
		{
			//cout << "dest reached: " << i << endl;
			if( i < 15 )
				return NULL;
			else
				return curr;
		}
		else
		{
			if( prev == NULL )
			{
				//assert( numConnects == 2 );
				//start

				//itll always take the first option for now. gotta fix later
				prev = curr;
				if( curr->connectsOpen[0] )
				{
					curr = curr->connects[0];
				}
				else //1 is open
				{
					curr = curr->connects[1];
				}
			}
			else
			{
				prev->partOfPath = true;
				if( curr->numConnects == 2 )
				{
					bool p0 = curr->connects[0]->partOfPath;
					bool p1 = curr->connects[1]->partOfPath;
					//cout << "0: " << curr->connects[0] << ", 1: " << curr->connects[1] << ", prev: " << prev << endl;
					//cout << "prevstuff: " << (int)prev->partOfPath << endl;
					if( p0 && p1 )
					{
						cout << "this thing" << endl;
						//return NULL;
						break;
						//return;
					}
					else if( p0 )
					{
						prev = curr;
						curr = curr->connects[1];
					}
					else if( p1 )
					{
						prev = curr;
						curr = curr->connects[0];
					}
					else
					{
						assert( false );
					}

					//cout << "this thing" << endl;
					//return;
				}
				else
				{
					for( int n = 0; n < 3; ++n )
					{	
						if( curr->connectsOpen[n] && !curr->connects[n]->partOfPath )
						{
							prev = curr;
							curr = curr->connects[n];
							break;
						}
					}
				}
			}
			//prev = curr;
		}
	}

	return NULL;
}

void Boss_Skeleton::UpdatePathVA()
{
}

void Boss_Skeleton::CreateNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex)
{
	FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
		testLength * yIndex ) );
	assert( nodes[xIndex][yIndex] == NULL );
	nodes[xIndex][yIndex] = fn;
	
	int blah = 10;
	//cout << "node " << testIndex << ": " << xIndex << ", " << yIndex << endl;
	//cout << "position: " << fn->position.x << ", " << fn->position.y << endl;
	flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
	flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
	flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
	flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
	testIndex++;
}

Boss_Skeleton::FlowerNode * Boss_Skeleton::CreateFlowerNode( Vector2i &basePos, int xIndex, int yIndex )
{
	//bounds: xIndex [-8,8], yIndex [-8,8]

	int blah = 10;

	//int realXIndex = xIndex + 8;
	//int realYIndex = yIndex + 8;

	FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
		testLength * yIndex ) );
	//cout << "node " << testIndex << ": " << xIndex << ", " << yIndex << endl;

	flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
	flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
	flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
	flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
	testIndex++;
	
	return fn;
}

void Boss_Skeleton::CreateAxisNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex, int yIndex )
{
	FlowerNode *fn;
	int realX = xIndex + HALF_GRID;
	int realY = yIndex + HALF_GRID;
	fn = CreateFlowerNode( basePos, xIndex, yIndex );
	nodes[realX][realY] = fn;

	if( xIndex != 0 )
	{
		realX = -xIndex + HALF_GRID;
		realY = yIndex + HALF_GRID;
		fn = CreateFlowerNode( basePos, -xIndex, yIndex );
		nodes[realX][realY] = fn;

		if( yIndex != 0 )
		{
			realX = -xIndex + HALF_GRID;
			realY = -yIndex + HALF_GRID;
			fn = CreateFlowerNode( basePos, -xIndex, -yIndex );
			nodes[realX][realY] = fn;
		}
	}

	if( yIndex != 0 )
	{
		realX = xIndex + HALF_GRID;
		realY = -yIndex + HALF_GRID;
		fn = CreateFlowerNode( basePos, xIndex, -yIndex );
		nodes[realX][realY] = fn;
	}
}

void Boss_Skeleton::CreateMirrorNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex)
{
	int blah = 10;
	if( xIndex == yIndex )
	{
		CreateAxisNode( basePos, nodes, xIndex, yIndex );
	}
	else
	{
		CreateAxisNode( basePos, nodes, xIndex, yIndex );
		CreateAxisNode( basePos, nodes, yIndex, xIndex);

		/*FlowerNode *fn;
		int realX = xIndex + 8;
		int realY = yIndex + 8;
		fn = CreateFlowerNode( basePos, xIndex, yIndex );
		nodes[realX][realY] = fn;

		realX = -xIndex + 8;
		realY = yIndex + 8;
		fn = CreateFlowerNode( basePos, -xIndex, yIndex );
		nodes[realX][realY] = fn;

		realX = -xIndex + 8;
		realY = -yIndex + 8;
		fn = CreateFlowerNode( basePos, -xIndex, -yIndex );
		nodes[realX][realY] = fn;

		realX = xIndex + 8;
		realY = -yIndex + 8;
		fn = CreateFlowerNode( basePos, xIndex, -yIndex );
		nodes[realX][realY] = fn;


		FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
			testLength * yIndex ) );
		nodes[xIndex][yIndex] = fn;
		flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
		flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
		flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
		flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
		testIndex++;
		
		fn = new FlowerNode( basePos + Vector2i( testLength * yIndex, 
		testLength * xIndex ) );
		nodes[yIndex][xIndex] = fn;
		flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
		flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
		flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
		flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
		testIndex++;*/
	}
	
}

void Boss_Skeleton::CreateLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex0,
		int yIndex0, int xIndex1, int yIndex1 )
{
	//cout << "create link. fn0: " << xIndex0 << ", " << yIndex0 << ", fn1: " << xIndex1 << ", " << yIndex1 << endl;
	FlowerNode *fn0 = nodes[xIndex0][yIndex0];
	FlowerNode *fn1 = nodes[xIndex1][yIndex1];

	assert( fn0 != NULL );
	assert( fn1 != NULL );

	if( fn0->numConnects >= 3 )
	{
		cout << "fn0: " << fn0->numConnects << endl;
		cout << "numlinks: " << testNumLinks << endl;
		cout << "xindex0: " << xIndex0-HALF_GRID << ", yindex0: " << yIndex0-HALF_GRID << endl;
		cout << "xindex1: " << xIndex1-HALF_GRID << ", yindex1: " << yIndex1-HALF_GRID << endl;
		cout << "fn0pos: " << fn0->position.x << ", " << fn0->position.y << endl;
		assert( fn0->numConnects < 3 );
	}
	else if( fn1->numConnects >= 3 )
	{
		cout << "fn1: " << fn1->numConnects << endl;
		cout << "numlinks: " << testNumLinks << endl;
		cout << "xindex0: " << xIndex0-HALF_GRID << ", yindex0: " << yIndex0-HALF_GRID << endl;
		cout << "xindex1: " << xIndex1-HALF_GRID << ", yindex1: " << yIndex1-HALF_GRID << endl;
		cout << "fn1pos: " << fn1->position.x << ", " << fn1->position.y << endl;
		assert( fn1->numConnects < 3 );
	}
	
	

	fn0->connects[fn0->numConnects] = fn1;
	fn1->connects[fn1->numConnects] = fn0;

	//setup visuals

	Vector2i sub = fn1->position - fn0->position;
	V2d along = V2d( sub.x, sub.y );
	double len = length( along );
	double w = 5;
	along = normalize( along );

	V2d other = V2d( along.y, -along.x );
	V2d start( fn0->position.x, fn0->position.y );
	V2d end( fn1->position.x, fn1->position.y );

	V2d sl = start + other * w;
	V2d sr = start - other * w;
	V2d el = end + other * w;
	V2d er = end - other * w;

	linkVA[testNumLinks*4+0].position = Vector2f( sl.x, sl.y );
	linkVA[testNumLinks*4+1].position = Vector2f( el.x, el.y );
	linkVA[testNumLinks*4+2].position = Vector2f( er.x, er.y );
	linkVA[testNumLinks*4+3].position = Vector2f( sr.x, sr.y );


	fn0->numConnects++;
	fn1->numConnects++;

	++testNumLinks;

	//cout << "end of link creation" << endl;
}

void Boss_Skeleton::CreateMirrorLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE], 
	int xIndex0, int yIndex0, 
	int xIndex1, int yIndex1 )
{
	bool a = xIndex0 == yIndex0;
	bool b = xIndex1 == yIndex1;
	if( a && b )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
	}
	else if( a )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, xIndex0, yIndex0, yIndex1, xIndex1 );
	}
	else if( b )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, yIndex0, xIndex0, xIndex1, yIndex1 );
	}
	//else if( yIndex0 == 0 || yIndex1 == 0 )
	//{
	//	cout << "creating single link" << endl;
	//	CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
	//	//CreateAxisLink( nodes, yIndex0, xIndex0, yIndex1, xIndex1 );
	//}
	else
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, yIndex0, xIndex0, yIndex1, xIndex1 );
	}

}

void Boss_Skeleton::CreateAxisLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1 )
{
	int realX0 = xIndex0 + HALF_GRID;
	int realY0 = yIndex0 + HALF_GRID;
	int realX1 = xIndex1 + HALF_GRID;
	int realY1 = yIndex1 + HALF_GRID;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );

	if( xIndex0 != 0 || xIndex1 != 0 )
	{
		realX0 = -xIndex0 + HALF_GRID;
		realY0 = yIndex0 + HALF_GRID;
		realX1 = -xIndex1 + HALF_GRID;
		realY1 = yIndex1 + HALF_GRID;
		CreateLink( nodes, realX0, realY0, realX1, realY1 );

		if( yIndex0 != 0 || yIndex1 != 0 )
		{
			realX0 = -xIndex0 + HALF_GRID;
			realY0 = -yIndex0 + HALF_GRID;
			realX1 = -xIndex1 + HALF_GRID;
			realY1 = -yIndex1 + HALF_GRID;
			CreateLink( nodes, realX0, realY0, realX1, realY1 );
		}

	}

	if( yIndex0 != 0 || yIndex1 != 0 )
	{
		realX0 = xIndex0 + HALF_GRID;
		realY0 = -yIndex0 + HALF_GRID;
		realX1 = xIndex1 + HALF_GRID;
		realY1 = -yIndex1 + HALF_GRID;
		CreateLink( nodes, realX0, realY0, realX1, realY1 );
	}
	//if( xIndex0 != 0 && xIndex1 != 0 )
	//{
	//	realX0 = -xIndex0 + 8;
	//	realY0 = yIndex0 + 8;
	//	realX1 = -xIndex1 + 8;
	//	realY1 = yIndex1 + 8;
	//	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	//}
	//else if( xIndex0 == 0 )
	//{
	//	
	//}
	/*if( xIndex0 != 0 || xIndex1 != 0 )
	{*/
		
	//}
	
	/*if( xIndex0 != 0 || xIndex1 != 0 || yIndex0 != 0 || yIndex1 != 0 )
	{*/
		
	//}
	
	/*if( yIndex0 != 0 || yIndex1 != 0 )
	{*/
		
	//}

	/*int realX0 = xIndex0 + 8;
	int realY0 = yIndex0 + 8;
	int realX1 = xIndex1 + 8;
	int realY1 = yIndex1 + 8;

	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = -xIndex0 + 8;
	realY0 = yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = -xIndex0 + 8;
	realY0 = -yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = xIndex0 + 8;
	realY0 = -yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );*/
}

void Boss_Skeleton::CreateZeroLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1, int dir )
{
	
}

void Boss_Skeleton::CreateQuadrant()
{
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			nodes[i][j] = NULL;
			
		}
	}

	int LEFT = 0;
	int RIGHT = 1;
	int UP = 2;
	int DOWN = 2;
	
	int length = 128;
	Vector2i basePos( position.x, position.y );
	//8x8
	//make multiple center nodes
	
	{
	CreateMirrorNode( basePos, nodes, 1, 0 );
	CreateMirrorNode( basePos, nodes, 4, 0 );
	CreateMirrorNode( basePos, nodes, 5, 0 );
	CreateMirrorNode( basePos, nodes, 8, 0 );

	CreateMirrorNode( basePos, nodes, 2, 1 );
	CreateMirrorNode( basePos, nodes, 3, 1 );
	CreateMirrorNode( basePos, nodes, 6, 1 );
	CreateMirrorNode( basePos, nodes, 7, 1 );


	CreateMirrorNode( basePos, nodes, 4, 2 );
	CreateMirrorNode( basePos, nodes, 5, 2 );
	CreateMirrorNode( basePos, nodes, 4, 3 );
	CreateMirrorNode( basePos, nodes, 5, 4 );
	CreateMirrorNode( basePos, nodes, 6, 3 );
	CreateMirrorNode( basePos, nodes, 6, 4 );
	CreateMirrorNode( basePos, nodes, 7, 5 );
	CreateMirrorNode( basePos, nodes, 7, 6 );
	CreateMirrorNode( basePos, nodes, 8, 7 );

	CreateMirrorNode( basePos, nodes, 2, 2 );
	CreateMirrorNode( basePos, nodes, 3, 3 );
	CreateMirrorNode( basePos, nodes, 5, 5 );
	CreateMirrorNode( basePos, nodes, 6, 6 );
	CreateMirrorNode( basePos, nodes, 8, 8 );

	//new
	CreateMirrorNode( basePos, nodes, 7, 3 );
	CreateMirrorNode( basePos, nodes, 8, 2 );
	CreateMirrorNode( basePos, nodes, 9, 2 );
	CreateMirrorNode( basePos, nodes, 10, 1 );
	CreateMirrorNode( basePos, nodes, 9, 0 );
	CreateMirrorNode( basePos, nodes, 11, 0 );


	}


	//CreateMirrorLink( nodes, 6, 6, 7, 6 );

	CreateMirrorLink( nodes, 2, 2, 2, 1 );
	CreateMirrorLink( nodes, 2, 2, 3, 3 );

	CreateMirrorLink( nodes, 3, 3, 4, 3 );

	CreateMirrorLink( nodes, 5, 5, 5, 4 );
	CreateMirrorLink( nodes, 5, 5, 6, 6 );

	CreateMirrorLink( nodes, 6, 6, 7, 6 );

	
	CreateMirrorLink( nodes, 2, 1, 3, 1 );
	CreateMirrorLink( nodes, 3, 1, 4, 2 );
	CreateMirrorLink( nodes, 3, 1, 4, 0 );
	
	CreateMirrorLink( nodes, 6, 1, 7, 1 );
	CreateMirrorLink( nodes, 6, 1, 5, 2 );
	CreateMirrorLink( nodes, 6, 3, 5, 2 );
	CreateMirrorLink( nodes, 6, 3, 6, 4 );
	CreateMirrorLink( nodes, 6, 4, 5, 4 );
	CreateMirrorLink( nodes, 6, 4, 7, 5 );
	

	CreateMirrorLink( nodes, 5, 4, 4, 3 );
	CreateMirrorLink( nodes, 4, 3, 4, 2 );

	CreateMirrorLink( nodes, 5, 2, 4, 2 );
	

	CreateMirrorLink( nodes, 1, 0, 2, 1 );
	//CreateAxisLink( nodes, 1, 0, 0, 1 );
	//CreateMirrorLink( nodes, 1, 1, 1, 1 );

	
	CreateMirrorLink( nodes, 5, 0, 4, 0 );
	CreateMirrorLink( nodes, 7, 1, 8, 0 );
	CreateMirrorLink( nodes, 5, 0, 6, 1 );

	CreateMirrorLink( nodes, 7, 5, 7, 6 );
	CreateMirrorLink( nodes, 8, 7, 8, 8 );
	CreateMirrorLink( nodes, 8, 7, 7, 6 );

	//new
	CreateMirrorLink( nodes, 6, 3, 7, 3 );
	CreateMirrorLink( nodes, 7, 3, 8, 2 );
	CreateMirrorLink( nodes, 7, 1, 8, 2 );
	CreateMirrorLink( nodes, 8, 2, 9, 2 );
	CreateMirrorLink( nodes, 9, 2, 10, 1 );
	CreateMirrorLink( nodes, 9, 0, 10, 1 );
	CreateMirrorLink( nodes, 11, 0, 10, 1 );
	CreateMirrorLink( nodes, 8, 0, 9, 0 );

	
	
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			if( nodes[i][j] != NULL )
				nodes[i][j]->dest = false;
			
		}
	}

	nodes[11+HALF_GRID][0+HALF_GRID]->dest = true;
	nodes[-11+HALF_GRID][0+HALF_GRID]->dest = true;
	nodes[0+HALF_GRID][11+HALF_GRID]->dest = true;
	nodes[0+HALF_GRID][-11+HALF_GRID]->dest = true;
	nodes[8+HALF_GRID][8+HALF_GRID]->dest = true;
	nodes[-8+HALF_GRID][8+HALF_GRID]->dest = true;
	nodes[-8+HALF_GRID][-8+HALF_GRID]->dest = true;
	nodes[8+HALF_GRID][-8+HALF_GRID]->dest = true;


	//cout << "testNumLInks: " << testNumLinks << endl;
	//nodes[][0] = new FlowerNode( basePos + Vector2i( length, 0 ) );
	//int blah = 0;
	//bool mode = false;
	//while( blah < 8 )
	//{
	//	if( !mode )
	//	{
	//		//double leap

	//	}
	//}
	


	//nodes[1][0]->connects[0] = nodes[0][0];
	//nodes[1][0]->connects[0] = nodes[0][0];
	//for( int i = 0; i < 8; ++i )
	//{
	//	for( int j = 0; j < 8; ++j )
	//	{
	//		blah[i][j] = new FlowerNode( basePos + Vector2i( length * i, length * j ) );
	//	}
	//}
	
	
}
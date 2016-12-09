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

void Boss_Coyote::CreateNodes()
{
	partying = true;
	//V2d testPos = ground->GetPoint( quantity );

	//V2d truePos = position;
	arenaCenter = V2d( originalPos.x, originalPos.y );//owner->poiMap["coyotecenter"]->pos;
	//V2d testPos = 
	//testPos += V2d( -200, -200 );

	double radius = 600;
	double angle = PI / 2;
	V2d curr,next;
	V2d dir;
	int lineIndex = 0;

	//create corners
	for( int i = 0; i < 6; ++i )
	{
		dir = V2d( cos( angle - PI / 2 ), sin( angle - PI / 2 ) );
		points[i] = new ScorpionNode( arenaCenter + dir * radius );
		//edges[i] = new ScorpionNode;
		angle += 2 * PI / 6;
	}

	//create edges
	for( int i = 0; i < 6; ++i )
	{
		curr = points[i]->position;
		if( i == 5 )
		{
			next = points[0]->position;
		}
		else
		{
			next = points[i+1]->position;
		}
		edges[i] = new ScorpionNode( ( curr + next ) / 2.0 );
	}

	for( int i = 0; i < pathSize; ++i )
	{
		testPaths[i].color = Color::Red;
		testPaths[i].position = Vector2f( 0, 0 );
	}

	//connect corners to corners
	for( int i = 0; i < 6; ++i )
	{
		int prevI = i - 1;
		if( prevI < 0 ) prevI = 5;

		int nextI = i + 1;
		if( nextI == 6 ) nextI = 0;

		int index = 0;
		for( int j = 0; j < 6; ++j )
		{
			if( i != j )//&& j != prevI && j != nextI )
			{
				points[i]->neighbors[index] = points[j];
				++index;
			}
		}
		//points[i]->neighbors[3] = edges[prevI];
		//points[i]->neighbors[4] = edges[nextI];

		//connect points to edges
		//cout << "index: " << index << endl;

		for( int blah = 0; blah < 3; ++blah )
		{
			//cout << "line index: " << lineIndex << endl;
			
			Vector2f startPos( points[i]->position.x, points[i]->position.y );
			//cout << "i: " << i << ", blah: " << blah << endl;
			//cout << "blahzzz: " << points[i]->neighbors[blah] << endl;
			Vector2f endPos( points[i]->neighbors[blah]->position.x, points[i]->neighbors[blah]->position.y );
			
			//cout << "test: " << testPaths[lineIndex*2].position.x << endl;
			testPaths[lineIndex * 2 + 0].position = startPos;
			testPaths[lineIndex * 2 + 1].position = endPos;
			++lineIndex;
		}
	}
	
	if( false )
	{
	//connect points to edges
	for( int i = 0; i < 6; ++i )
	{
		int prevI = i - 1;
		if( prevI < 0 ) prevI = 5;

		int nextI = i;

		points[i]->neighbors[3] = edges[prevI];
		points[i]->neighbors[4] = edges[nextI];

		for( int blah = 3; blah < 5; ++blah )
		{
			Vector2f startPos( points[i]->position.x, points[i]->position.y );
			Vector2f endPos( points[i]->neighbors[blah]->position.x, points[i]->neighbors[blah]->position.y );
			testPaths[lineIndex * 2 + 0].position = startPos;
			testPaths[lineIndex * 2 + 1].position = endPos;
			++lineIndex;
		}
	}

	noPartyCutoff = lineIndex * 2;//12 * 5 * 2;

	//////connect edges to edges
	for( int i = 0; i < 6; ++i )
	{
		int prevI = i - 2;
		if( prevI == -1 ) prevI = 5;
		else if( prevI == -2 ) prevI = 4;

		int nextI = i + 2;
		if( nextI == 6 ) nextI = 0;
		else if( nextI == 7 ) nextI = 1;

		int index = 0;
		for( int j = 0; j < 6; ++j )
		{
			if( i != j && j != prevI && j != nextI )
			{
				edges[i]->neighbors[index] = edges[j];
				++index;
			}
		}

		for( int blah = 0; blah < 3; ++blah )
		{
			Vector2f startPos( edges[i]->position.x, edges[i]->position.y );
			Vector2f endPos( edges[i]->neighbors[blah]->position.x, edges[i]->neighbors[blah]->position.y );
			testPaths[lineIndex * 2 + 0].position = startPos;
			testPaths[lineIndex * 2 + 1].position = endPos;
			++lineIndex;
		}
	}

	

	//connect edges to points
	for( int i = 0; i < 6; ++i )
	{
		int prevI = i;

		int nextI = i;
		if( nextI == 6 ) nextI = 0;

		edges[i]->neighbors[3] = points[prevI];
		edges[i]->neighbors[4] = points[nextI];
	}

	}
}

void Boss_Coyote::SetPartyMode( bool party )
{
	if( !partying && party )
	{
		pathCutoff = pathSize;

		for( int i = 0; i < 6; ++i )
		{
			int prevI = i - 1;
			if( prevI < 0 ) prevI = 5;

			int nextI = i;

			points[i]->neighbors[3] = edges[prevI];
			points[i]->neighbors[4] = edges[nextI];
		}

		partying = party;

	}
	else if( partying && !party )
	{
		
		for( int i = 0; i < 6; ++i )
		{
			int prevI = i - 1;
			if( prevI < 0 ) prevI = 5;

			int nextI = i + 1;
			if( nextI == 6 ) nextI = 0;

			points[i]->neighbors[3] = points[prevI];
			points[i]->neighbors[4] = points[nextI];
		}

		pathCutoff = noPartyCutoff;

		partying = party;
	}
}

Boss_Coyote::ScorpionNode::ScorpionNode( sf::Vector2<double> &pos )
	:position( pos ), facingIndex( -1 ), nType( DIRECTION )
{
	//index as -1 means you stop
	for( int i = 0; i < 5; ++i )
	{
		neighbors[i] = NULL;
	}
}

void Boss_Coyote::ScorpionNode::SetNewDirection( bool onlyMovement )
{
	int r = rand() % 4;
	if( true )//r < 2 || onlyMovement )
	{
		nType = DIRECTION;
		int index = 0;
		int possibles[4];
		for( int i = 0; i < 5; ++i )
		{
			if( i != facingIndex )
			{
				possibles[index] = i;
				++index;
			}
		}
		int test = rand() % 4;
		facingIndex = possibles[test];
	}
	else
	{
		if( r == 2 )
		{
			nType = SHOTGUN;
		}
		else
		{
			nType = REVERSE_SHOTGUN;
		}
		
	}
}

void Boss_Coyote::RandomizeDirections()
{
	for( int i = 0; i < 6; ++i )
	{
		points[i]->facingIndex = rand() % 5;
		edges[i]->facingIndex = rand() % 5;
	}
}

Boss_Coyote::Boss_Coyote( GameSession *owner, sf::Vector2i &pos )
	:Enemy( owner, EnemyType::STAGBEETLE, false, 3 ),//, facingRight( cw ),
	moveBezTest( 0,0,1,1 ), bigBounceBullet( this ),
	dialogue( owner, DialogueBox::BIRD )//, testPaths( sf::Lines, 12 * 5 * 2 )
{
	ts_afterImage = owner->GetTileset( "bosscoyote_afterimage_128x128.png", 128, 128 );

	ts_face = owner->GetTileset( "Bosses/Coyote/03_coyote_face_02_384x384.png", 384, 384 );

	ts_symbols0 = owner->GetTileset( "Bosses/Dialogue/Symbols/02_Symbols_256x256.png", 256, 256 );

	fi0.push_back( SymbolInfo() );
	SymbolInfo *si = &fi0.back();
	si->ts = ts_symbols0;
	si->frame = 0;
	si->framesHold = 20;
	/*fi0.push_back( SymbolInfo() );
	si = &fi0.back();
	si->ts = ts_symbols0;
	si->frame = 1;
	si->framesHold = 20;*/

	dextra0 = Vector2f( -200, 0 );

	originalPos = pos;

	coyoteFightSeq = new CoyoteFightSeq( owner );
	meetCoyoteSeq = new MeetCoyoteSeq( owner );
	coyoteTalkSeq = new CoyoteTalkSeq( owner );
	//skeletonAttackCoyoteSeq = new SkeletonAttackCoyoteSeq( owner );


	action = SEQ_SLEEP;
	frame = 0;
	pathSize = 120;
	pathCutoff = pathSize;// 12 * 5 * 2
	testPaths = new Vertex[pathSize];
	speed = 15;//25;
	testCircle.setRadius( 30 );
	testCircle.setFillColor( Color::Magenta );
	testCircle.setOrigin( testCircle.getLocalBounds().width / 2, 
		testCircle.getLocalBounds().height / 2 );

	travelFrame = 0;
	facingRight = false;
	gravity = V2d( 0, .6 );
	//maxGroundSpeed = s;
	//action = RUN;
	initHealth = 60;
	health = initHealth;
	dead = false;
	deathFrame = 0;

	maxFallSpeed = 25;

	//ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	//ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );

	attackFrame = -1;
	attackMult = 10;

	double height = 128;
	double width = 128;

	originalPos = pos;
	//startGround = g;
	//startQuant = q;
	frame = 0;

	testMover = new GroundMover( owner, NULL, 0, 32, true, this );
	//testMover->gravity = V2d( 0, .5 );
	testMover->SetSpeed( 0 );
	//testMover->groundSpeed = s;
	/*if( !facingRight )
	{
		testMover->groundSpeed = -testMover->groundSpeed;
	}*/

	ts = owner->GetTileset( "crawler_128x128.png", width, height );
	

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	//V2d gPoint = g->GetPoint( q );
	sprite.setPosition( testMover->physBody.globalPosition.x,
		testMover->physBody.globalPosition.y );
	position = V2d( pos.x, pos.y );//testMover->physBody.globalPosition;
	//roll = false;
	//position = gPoint + ground->Normal() * height / 2.0;
	

	receivedHit = NULL;

	double size = 32;//max( width, height );
	spawnRect = sf::Rect<double>( pos.x - size, pos.y - size, size * 2, size * 2 );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	
	afterImageHitboxInfo = new HitboxInfo;
	afterImageHitboxInfo->damage = 18;
	afterImageHitboxInfo->drainX = 0;
	afterImageHitboxInfo->drainY = 0;
	afterImageHitboxInfo->hitlagFrames = 0;
	afterImageHitboxInfo->hitstunFrames = 15;
	afterImageHitboxInfo->knockback = 0;

	/*testLaunch = new Launcher( this, owner, 10, 1,
		testMover->physBody.globalPosition, g->Normal(), 0 );*/
	/*physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 32;
	physBody.rh = 32;
	physBody.type = CollisionBox::BoxType::Physics;*/

	

	deathPartingSpeed = .4;

	

	bezFrame = 0;
	bezLength = 60 * NUM_STEPS;

	testMover->SetSpeed( 0 );

	CreateNodes();

	currNode = points[0];

	RandomizeDirections();

	int bulletTTL = 500;
	launcher = new Launcher( this, BasicBullet::BOSS_COYOTE, owner, 144, 12, position, V2d( 1, 0 ), PI * 2.0 / 3.0, bulletTTL, true );
	launcher->SetBulletSpeed( 3 );	
	
	//SetPartyMode( false );
	//SetPartyMode( true );

	//testMover->Move( slowMultiple );

	//ground = testMover->ground;
	//edgeQuantity = testMover->edgeQuantity;
	//position = testMover->physBody.globalPosition;
}

Boss_Coyote::~Boss_Coyote()
{
	delete [] testPaths;
}

void Boss_Coyote::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
{
	
}

void Boss_Coyote::BulletHitPlayer( BasicBullet *b )
{
}

void Boss_Coyote::ResetEnemy()
{
	activeImages = NULL;
	bigBounceBullet.Reset( position );
	launcher->Reset();
	
	action = SEQ_SLEEP;//MOVE;
	RandomizeDirections();
	currNode = points[0];
	//ResetDirections();
	travelFrame = 0;

	//testMover->ground = startGround;
	//testMover->edgeQuantity = startQuant;
	testMover->physBody.globalPosition = V2d( originalPos.x, originalPos.y );
	testMover->ground = NULL;
	//testMover->roll = false;
	//testMover->UpdateGroundPos();
	//testMover->SetSpeed( 0 );

	position = testMover->physBody.globalPosition;
	
	//testMover->UpdateGroundPos();

	//testLaunch->Reset();
	//testLaunch->position = testMover->physBody.globalPosition;
	//testLaunch->facingDir = startGround->Normal();

	bezFrame = 0;
	health = initHealth;
	attackFrame = -1;
	//lastReverser = false;
	//roll = false;
	//ground = startGround;
	//edgeQuantity = startQuant;
	//V2d gPoint = testMover->ground->GetPoint( testMover->edgeQuantity );
	//sprite.setPosition( testMover->physBody.globalPosition.x,
	//	testMover->physBody.globalPosition.y );
	frame = 0;

	//V2d gn = testMover->ground->Normal();
	//testMover->physBody.globalPosition = gPoint + testMover->ground->Normal() * 64.0 / 2.0;

	/*V2d gn = ground->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;*/

	//position = gPoint + offset;

	deathFrame = 0;
	dead = false;

	//----update the sprite
	double angle = 0;
	////position = gPoint + gn * 32.0;
	angle = 0;//atan2( gn.x, -gn.y );
	//	
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setRotation( angle );
	//sprite.setTextureRect( ts->GetSubRect( frame / crawlAnimationFactor ) );
	//sprite.setPosition( 
	//V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( position.x, position.y );
	//----

	UpdateHitboxes();
	ClearAfterImages();
}

void Boss_Coyote::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	//might need for other queries but def not for physics
}

void Boss_Coyote::UpdateHitboxes()
{
	Edge *ground = testMover->ground;
	if( ground != NULL )
	{
		//V2d gn = ground->Normal();
		//double angle = 0;
		
		
		//angle = atan2( gn.x, -gn.y );
		
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( testMover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody.globalPosition = testMover->physBody.globalPosition;
	hurtBody.globalPosition = testMover->physBody.globalPosition;
	//physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
}

void Boss_Coyote::ActionEnded()
{
	switch( action )
	{
	case MOVE:
		frame = 0;
		break;
	case MOVE_WAIT:
		if( frame == 20 )
		{
			action = MOVE;
			frame = 0;
		}
		break;
	case SHOTGUN:
		if( frame == 60 )
		{
			action = MOVE;
			frame = 0;
			travelFrame = 0;

			//travelIndex = 0;
		}
		break;
	case REVERSE_SHOTGUN:
		{
			if( frame == 80 )
			{
				action = MOVE;
				frame = 0;
				travelFrame = 0;
			}
		}
	case SEQ_SLEEP:
		if( frame == 60 )
			frame = 0;
		break;
	case SEQ_ILL_TEST_YOU:
		if( frame == 30 )
			frame = 0;
		break;
	case SEQ_RUN:
		//if( frame == 30 )
		//	frame = 0;
		break;
	}	
}

void Boss_Coyote::UpdatePrePhysics()
{
	ActionEnded();

	switch( action )
	{
	case MOVE:
		{
			ScorpionNode *nextNode = currNode->neighbors[currNode->facingIndex];
			double diff = length( nextNode->position - currNode->position );

			int cap = diff / speed + .5;

			if( travelFrame == cap )
			{
				travelFrame = 0;
				currNode->SetNewDirection( false );
				currNode = nextNode;
				action = MOVE_WAIT;
				frame = 0;

				switch( currNode->nType )
				{
				case ScorpionNode::DIRECTION:
					break;
				case ScorpionNode::SHOTGUN:
					action = SHOTGUN;
					frame = 0;
					break;
				case ScorpionNode::REVERSE_SHOTGUN:
					action = REVERSE_SHOTGUN;
					frame = 0;
					bigBounceBullet.revNode = currNode;
					break;
				}
			}

			
		}
		break;
	case SHOTGUN:
		break;
	case REVERSE_SHOTGUN:
		break;
	}

	switch( action )
	{
	case MOVE:
		{
			ScorpionNode *nextNode = currNode->neighbors[currNode->facingIndex];
			double diff = length( nextNode->position - currNode->position );

			int cap = diff / speed + .5;

			double r = moveBezTest.GetValue( travelFrame / (double)cap );
			position = currNode->position * (1 - r) + nextNode->position * r;
			travelFrame++;
		}
		break;
	case SHOTGUN:
		if( frame == 0 )
		{
			currNode->SetNewDirection( true );
			launcher->facingDir = normalize( arenaCenter - currNode->position );
			launcher->position = currNode->position;
			launcher->Fire();
			cout << "fire" << endl;
		}
		break;
	case REVERSE_SHOTGUN:
		if( frame == 0 )
		{
			currNode->SetNewDirection( true );
			V2d shootDir = normalize( arenaCenter - currNode->position );
			double bSpeed = 25;
			bigBounceBullet.position = currNode->position;
			bigBounceBullet.Fire( shootDir * bSpeed );
		}
		break;
	case SEQ_SLEEP:
		break;
	case SEQ_ILL_TEST_YOU:
		
		break;
	case SEQ_RUN:
		if( frame == 0 )
		{
			portrait.Close();
			dialogue.Close();
			//owner->activeDialogue = NULL;
		}
		else if( frame == 60 )
		{
			testMover->SetSpeed( 10 );
			//testMover->groundSpeed = 10;
		}
		else if( frame == 120 )
		{
			testMover->SetSpeed( 0 );
		}
		//else if( frame == 40 )
		//{
		//	testMover->SetSpeed( 0 );
			//testMover->groundSpeed = 0;
		//}
		else if( frame == 121 )
		{
			//frame = 122;

			action = MOVE;

			testMover->ground = NULL;
			//testMover->ground = startGround;
			//testMover->edgeQuantity = startQuant;
			//testMover->UpdateGroundPos();
			facingRight = false;

			meetCoyoteSeq->CoyoteGone();

		}

		break;
	}

	portrait.Update();
	dialogue.Update();



	launcher->UpdatePrePhysics();
	bigBounceBullet.UpdatePrePhysics();

	return;
	//testLaunch->UpdatePrePhysics();
	Actor *player = owner->player;

	if( dead )
		return;

	



	//bool roll = testMover->roll;

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;

			
		}

		receivedHit = NULL;
	}


	
	if( attackFrame == 2 * attackMult )
	{
		attackFrame = -1;
	}
	//if( attacking )
	//{
	//}
	//else
	//{

		
	
		//cout << "groundspeed: " << testMover->groundSpeed << endl;
	//}

	/*if( bezFrame == 0 )
	{
		testLaunch->position = position;
		if( testMover->ground != NULL )
		{
			testLaunch->facingDir = testMover->ground->Normal();
		}
		else
		{
			testLaunch->facingDir = V2d( 0, -1 );
		}
		
		testLaunch->Fire();
	}*/
}

void Boss_Coyote::UpdatePhysics()
{
	if( action == SEQ_RUN || action == SEQ_ILL_TEST_YOU
		|| action == SEQ_SLEEP )
	{
		testMover->Move( slowMultiple );

		position = testMover->physBody.globalPosition;
		//testMover->Update();
	}
	launcher->UpdatePhysics();
	bigBounceBullet.UpdatePhysics();
	return;
	//testLaunch->UpdatePhysics();
	specterProtected = false;

	if( dead )
	{
		return;
	}



	double f = moveBezTest.GetValue( bezFrame / (double)bezLength );
	//testMover->groundSpeed = groundSpeed;// * f;
	if( !facingRight )
	{
	//	testMover->groundSpeed = groundSpeed;// * f;
	}
	bezFrame++;

	if( bezFrame == bezLength )
	{
		bezFrame = 0;
		

	}

	if( testMover->ground != NULL )
	{
	}
	else
	{
		testMover->velocity += gravity / (NUM_STEPS * slowMultiple);

		if( testMover->velocity.y >= maxFallSpeed )
		{
			testMover->velocity.y = maxFallSpeed;
		}
	}

	
	//testMover->groundSpeed = 5;
	testMover->Move( slowMultiple );

	position = testMover->physBody.globalPosition;
	
	PhysicsResponse();
}

bool Boss_Coyote::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	double rw = testMover->physBody.rw;
	double rh = testMover->physBody.rh;

	Rect<double> oldR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
	position += vel;
	
	Rect<double> newR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
	//minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	Rect<double> r( minLeft , minTop, maxRight - minLeft, maxBottom - minTop );

	
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

void Boss_Coyote::PhysicsResponse()
{
	if( !dead  )
	{
		
		
		

		//sprite.setPosition( position.x, position.y );

		UpdateHitboxes();

		if( PlayerSlowingMe() )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = 5;
			//	cout << "yes slow" << endl;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		//	cout << "no slow" << endl;
		}

		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
				//cout << "hit here!" << endl;
				//triggers multiple times per frame? bad?
				owner->player->ConfirmHit( 3, 5, .8, 6 );

				if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
				{
					owner->player->velocity.y = 4;//.5;
				}

															//cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );
		//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			
			/*if( !result.second )
			{
				owner->Pause( 6 );
			}*/
			
			//dead = true;
			//receivedHit = NULL;
			}
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		//gotta get the correct angle upon death
		Transform t;
		//t.rotate( angle / PI * 180 );
		Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
		deathVector = V2d( newPoint.x, newPoint.y );

		queryMode = "reverse";

		//physbody is a circle
		//Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		//owner->crawlerReverserTree->Query( this, r );
	}
}

void Boss_Coyote::UpdatePostPhysics()
{
	/*if( action == SEQ_RUN )
	{
		position = testMover->physBody.globalPosition;
	}*/
	bigBounceBullet.UpdatePostPhysics();
	launcher->UpdatePostPhysics();
	launcher->UpdateSprites();
	UpdateSprite();
	++frame;
	return;
	if( receivedHit != NULL )
		owner->Pause( 5 );

	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
	}

	UpdateSprite();
	//testLaunch->UpdateSprites();

	if( slowCounter == slowMultiple )
	{
		
		++frame;
		slowCounter = 1;
		
		if( dead )
		{
			deathFrame++;
		}
		else
		{
			if( attackFrame >= 0 )
				++attackFrame;
		}
	}
	else
	{
		slowCounter++;
	}

	//cout << "position: " << position.x << ", " << position.y << endl;
	//need to calculate frames in here!!!!

	//sprite.setPosition( position );
	//UpdateHitboxes();
}

bool Boss_Coyote::PlayerSlowingMe()
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

void Boss_Coyote::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		portrait.Draw( target );
		dialogue.Draw( target );
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 55 );
			cs.setFillColor( Color::Black );
			
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );
		target->draw( testPaths, pathCutoff, sf::Lines );
		testCircle.setPosition( position.x, position.y );
		target->draw( testCircle );
		bigBounceBullet.Draw( target );
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
		}
		
		target->draw( topDeathSprite );
	}
}

void Boss_Coyote::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	/*if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool Boss_Coyote::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( player->invincibleFrames == 0 && hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir.x = -abs( hitboxInfo->kbDir.x );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir.x = abs( hitboxInfo->kbDir.x );
		}
		else
		{
			//dont change it
		}
		attackFrame = 0;
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> Boss_Coyote::PlayerHitMe()
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

void Boss_Coyote::UpdateSprite()
{
	if( dead )
	{
		//cout << "deathVector: " << deathVector.x << ", " << deathVector.y << endl;
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 31 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 30 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{

		V2d p = testMover->physBody.globalPosition;

		sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height * 3.0/4.0);
		sprite.setPosition( p.x, p.y );
		sprite.setRotation( 0 );


		sprite.setPosition( position.x, position.y );
		/*if( attackFrame >= 0 )
		{
			IntRect r = ts->GetSubRect( 28 + attackFrame / attackMult );
			if( !facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
		}*/
	}
}

void Boss_Coyote::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		//if( ground != NULL )
		{
		/*CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		V2d g = ground->GetPoint( edgeQuantity );
		cs.setPosition( g.x, g.y );*/
		}
		//owner->window->draw( cs );
		//UpdateHitboxes();
		//physBody.DebugDraw( target );
		testMover->physBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Boss_Coyote::SaveEnemyState()
{
}

void Boss_Coyote::LoadEnemyState()
{
}

void Boss_Coyote::HitTerrain( double &q )
{
	
}

bool Boss_Coyote::StartRoll()
{

}

void Boss_Coyote::FinishedRoll()
{

}

void Boss_Coyote::HitOther()
{
	V2d v;
	if( facingRight && testMover->groundSpeed > 0 )
	{
		v = V2d( 10, -10 );
		testMover->Jump( v );
	}
	else if( !facingRight && testMover->groundSpeed < 0 )
	{
		v = V2d( -10, -10 );
		testMover->Jump( v );
	}
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Boss_Coyote::ReachCliff()
{
	if( facingRight && testMover->groundSpeed < 0 
		|| !facingRight && testMover->groundSpeed > 0 )
	{
		testMover->SetSpeed( 0 );
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if( facingRight )
	{
		v = V2d( 10, -10 );
	}
	else
	{
		v = V2d( -10, -10 );
	}

	testMover->Jump( v );
	//testMover->groundSpeed = -testMover->groundSpeed;
	//facingRight = !facingRight;
}

void Boss_Coyote::HitOtherAerial( Edge *e )
{
	//cout << "hit edge" << endl;
}

void Boss_Coyote::Land()
{
	frame = 0;
	//cout << "land" << endl;
}

Boss_Coyote::BigBounceBullet::BigBounceBullet( Boss_Coyote *p_parent )
	:parent( p_parent )
{
	int rad = 32;
	ts = owner->GetTileset( "Bosses/Coyote/bigbouncebullet_64x64.png", 64, 64 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2 );	
	frame = 0;

	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = rad;
	hurtBody.rh = rad;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = rad;
	hitBody.rh = rad;

	physBody.type = CollisionBox::Physics;
	physBody.isCircle = true;
	physBody.globalAngle = 0;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = rad;
	physBody.rh = rad;

	//collideWithTerrain = true;
	collideWithPlayer = true;

	active = false;
}

void Boss_Coyote::BigBounceBullet::UpdatePrePhysics()
{
	Movable::UpdatePrePhysics();
	if( framesToLive == 0 )
	{
		active = false;

		V2d diff = parent->arenaCenter - revNode->position;
		parent->launcher->facingDir = normalize( -diff );
		parent->launcher->position = parent->arenaCenter + diff;
		parent->launcher->Fire();
	}
}

void Boss_Coyote::BigBounceBullet::Fire( sf::Vector2<double> vel )
{
	int dist = length( parent->arenaCenter - position ) * 2;
	cout << "BIG bounce fire: " << vel.x << ", " << vel.y << endl;
	velocity = vel;
	active = true;
	frame = 0;
	framesToLive = dist / (int)length( vel );
}

void Boss_Coyote::BigBounceBullet::Reset(
	sf::Vector2<double> &pos )
{
	active = false;
	frame = 0;
	Movable::Reset( pos );
}

void Boss_Coyote::BigBounceBullet::UpdatePostPhysics()
{
	Movable::UpdatePostPhysics();
	sprite.setTextureRect( ts->GetSubRect( frame % 4 ) );
	sprite.setPosition( position.x, position.y );
}


void Boss_Coyote::BigBounceBullet::HitPlayer()
{
}
void Boss_Coyote::BigBounceBullet::IncrementFrame()
{
	++frame;
	--framesToLive;
}

void Boss_Coyote::BigBounceBullet::Draw( sf::RenderTarget *target )
{
	if( active )
	{
		target->draw( sprite );
	}
}

void Boss_Coyote::SetRelFacePos( Vector2f &pos )
{
	portrait.sprite.setPosition( pos + Vector2f( position.x, position.y ) );
	dialogue.SetPosition( portrait.sprite.getPosition() + dextra0 );
}

void Boss_Coyote::Start_IllTestYou()
{
	action = SEQ_ILL_TEST_YOU;
	frame = 0;
	SetRelFacePos( Vector2f( 0, -200 ) );
	portrait.Open();
	portrait.SetSprite( ts_face,
		0 );
	portrait.scaleMultiple = .5;

	dialogue.SetSymbols( &fi0 );
	dialogue.Open();
	owner->activeDialogue = this;
}

bool Boss_Coyote::ConfirmDialogue()
{
	if( portrait.state != PortraitBox::OPEN ||
		dialogue.state != DialogueBox::OPEN )
	{
		return false;
	}

	switch( action )
	{
	case SEQ_ILL_TEST_YOU:
		action = SEQ_RUN;
		frame = 0;
		break;
	}

	return true;
}


void Boss_Coyote::ClearAfterImages()
{
	AfterImage *active = activeImages;
	while( active != NULL )
	{
		AfterImage *next = active->next;
		DeactivateAfterImage( active );
		active = next;
	}
	activeImages = NULL;
}

void Boss_Coyote::DeactivateAfterImage( AfterImage *afterImage )
{
	//remove from active list
	assert( activeImages != NULL );

	if( afterImage->prev == NULL && afterImage->next == NULL )
	{
		activeImages = NULL;
	}
	else if( afterImage->prev == NULL )
	{
		activeImages = afterImage->next;
	}
	else if( afterImage->next == NULL )
	{
		afterImage->prev->next = NULL;
		afterImage->prev = NULL;
	}
	else
	{
		afterImage->prev->next = afterImage->next;
		afterImage->next->prev = afterImage->prev;
		afterImage->prev = NULL;
		afterImage->next = NULL;
	}


	//add to inactive list
	inactiveImages->prev = afterImage;
	afterImage->next = inactiveImages;
	inactiveImages = afterImage;

	afterImage->Clear();
}

Boss_Coyote::AfterImage * Boss_Coyote::ActivateAfterImage()
{
	if( inactiveImages == NULL )
		return NULL;
	else
	{
		AfterImage *temp = inactiveImages->next;
		AfterImage *newImage = inactiveImages;
		inactiveImages = temp;
		inactiveImages->prev = NULL;

		newImage->Reset( position );


		if( activeImages == NULL )
		{
			activeImages = newImage;
		}
		else
		{
			activeImages->prev = newImage;
			newImage->next = activeImages;
			activeImages = newImage;
		}

		return newImage;
	}
}

void Boss_Coyote::AddAfterImage()
{
	if( inactiveImages == NULL )
	{
		inactiveImages = new AfterImage( this, 0 );
	}
	else
	{
		AfterImage *ai = inactiveImages;
		int numImages = 0;
		while( ai != NULL )
		{
			numImages++;
			ai = ai->next;
		}

		//cout << "adding ring: " << numRings << endl;

		AfterImage *nai = new AfterImage( this, numImages );
		nai->next = inactiveImages;
		inactiveImages->prev = nai;
		inactiveImages = nai;
	}
}



void Boss_Coyote::AfterImage::UpdatePrePhysics()
{
	if( action == STAY && frame == 60 )
	{
		action = DISSIPATE;
		frame = 0;
	}
	else if( (action == DISSIPATE && frame == 60) )
	{
		parent->DeactivateAfterImage( this );
		return;
	}

	switch( action )
	{
	case STAY:
		{	
		}
		break;
	case DISSIPATE:
		break;
	}
}

Boss_Coyote::AfterImage::AfterImage( Boss_Coyote *p_parent, int p_vaIndex )
	:parent( p_parent ), frame( 0 ), next( NULL ), prev( NULL ),
	action( STAY ), vaIndex( p_vaIndex )
{
	hitbox.isCircle = true;
	hitbox.rw = 64;
	hitbox.rh = 64;
}

void Boss_Coyote::AfterImage::UpdatePostPhysics()
{
	IntRect ir = parent->ts_afterImage->GetSubRect( 0 );
	int hw = parent->ts_afterImage->tileWidth / 2;
	int hh = parent->ts_afterImage->tileHeight / 2;
	//parent->ts_homingRing
	parent->afterImageVA[vaIndex*4+0].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, -hh ); 
	parent->afterImageVA[vaIndex*4+1].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, -hh );
	parent->afterImageVA[vaIndex*4+2].position = Vector2f( position.x, position.y ) 
		+ Vector2f( hw, hh );
	parent->afterImageVA[vaIndex*4+3].position = Vector2f( position.x, position.y ) 
		+ Vector2f( -hw, hh );

	parent->afterImageVA[vaIndex*4+0].texCoords = Vector2f( ir.left, ir.top );
	parent->afterImageVA[vaIndex*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
	parent->afterImageVA[vaIndex*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	parent->afterImageVA[vaIndex*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );

	/*parent->homingVA[vaIndex*4+0].color = Color::Green;
	parent->homingVA[vaIndex*4+1].color = Color::Green;
	parent->homingVA[vaIndex*4+2].color = Color::Green;
	parent->homingVA[vaIndex*4+3].color = Color::Green;*/

	++frame;
}

void Boss_Coyote::AfterImage::UpdatePhysics()
{
	Actor *player = parent->owner->player;
	if( player->hurtBody.Intersects( hitbox ) )
	{
		parent->owner->player->ApplyHit( parent->afterImageHitboxInfo );
	}
}

void Boss_Coyote::AfterImage::Clear()
{
	parent->afterImageVA[vaIndex*4+0].position = Vector2f( 0, 0 );
	parent->afterImageVA[vaIndex*4+1].position = Vector2f( 0, 0 );
	parent->afterImageVA[vaIndex*4+2].position = Vector2f( 0, 0 );
	parent->afterImageVA[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void Boss_Coyote::AfterImage::Reset( sf::Vector2<double> &pos )
{
	action = STAY;
	frame = 0;
	position = pos;
	prev = NULL;
	next = NULL;
}
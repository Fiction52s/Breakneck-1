#include "Boss.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

Boss_Bird::Projectile::Projectile( Boss_Bird *b, bool p_large )
{
	slowMultiple = 1;
	slowCounter = 1;
	velocity = V2d( 0, 0 );

	boss = b;

	sprite.setTexture( *b->ts_projectile->texture );
	sprite.setTextureRect( b->ts_projectile->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );

	state = NO_EXIST;
	frame = 0;
	actionLength[NO_EXIST] = 1;
	actionLength[STILL] = 1;
	actionLength[HOME] = 1;
	actionLength[FALL_AT_PLAYER] = 1;
	actionLength[RETRACT] = 1;

	large = p_large;
	double rad = 32;
	if( large )
	{
		rad = 64;
	}


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

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	
	hitboxInfo->knockback = 20;
	hitboxInfo->hitstunFrames = 1;
	hitboxInfo->damage = 0;
	hitboxInfo->hitlagFrames = 0;
}

void Boss_Bird::Projectile::Reset()
{
	slowMultiple = 1;
	slowCounter = 1;
	velocity = V2d( 0, 0 );

	//sprite.setTexture( *b->ts_projectile->texture );
	//sprite.setTextureRect( b->ts_projectile->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	frame = 0;
	state = NO_EXIST;
}

void Boss_Bird::Projectile::Draw( RenderTarget *target )
{
	if( state != NO_EXIST )
	{
		target->draw( sprite );
	}
}

void Boss_Bird::Projectile::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	Contact *c = boss->owner->coll.collideEdge( position + tempVel, physBody, e, tempVel, V2d( 0, 0 ) );

	if( c != NULL )
	{
		if( !col )
		{
			minContact = *c;
			col = true;
		}
		else if( c->collisionPriority < minContact.collisionPriority )
		{
			minContact = *c;
		}
	}
}

bool Boss_Bird::Projectile::ResolvePhysics( 
		sf::Vector2<double> vel )
{
	Rect<double> oldR( position.x - physBody.rw, position.y - physBody.rw, 
		2 * physBody.rw, 2 * physBody.rw );

	//cout << "Resolving: " << vel.x << ", " << vel.y << endl;
	position += vel;

	Rect<double> newR( position.x - physBody.rw, position.y - physBody.rw, 
		2 * physBody.rw, 2 * physBody.rw );

	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );


	double ex = 1;
	Rect<double> r( minLeft - ex, minTop - ex, 
		(maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );
	//Rect<double> realRect( min( oldR.left, r.left ),
	//	min( oldR.top, r.top ));

	minContact.collisionPriority = 1000000;

	col = false;

	tempVel = vel;
	minContact.edge = NULL;

	//queryMode = "resolve";
//	Query( this, owner->testTree, r );
	//queryBullet = bullet;


	
	boss->owner->terrainTree->Query( this, r );

	return col;
}

void Boss_Bird::Projectile::UpdatePrePhysics()
{
	Actor *player = boss->owner->player;

	if( frame == actionLength[state] )
	{
		switch( state )
		{
		case NO_EXIST:
			frame = 0;
			break;
		case STILL:
			frame = 0;
			break;
		case HOME:
			frame = 0;
			break;
		case FALL_AT_PLAYER:
			frame = 0;
			break;
		case RETRACT:
			//state = NO_EXIST;
			frame = 0;
			break;
		}
	}
	
	switch( state )
	{
	case NO_EXIST:
		
		break;
	case STILL:
		
		break;
	case HOME:
		
		break;
	case FALL_AT_PLAYER:
		break;
	case RETRACT:
		V2d diff = boss->position - position;
		V2d bDir = normalize( diff );
		V2d newVel = bDir * ( frame * .4 );

		if( length( newVel ) > 20 )
		{
			V2d dir = normalize( newVel );
			newVel = dir * 20.0;
		}
		if( length( diff ) < length( newVel ) )
		{
			state = NO_EXIST;
			frame = 0;
		}
		break;
	}
		
	switch( state )
	{
	case NO_EXIST:
		
		break;
	case STILL:
		
		break;
	case HOME:
		
		break;
	case FALL_AT_PLAYER:
		{
			double fac = .25;
			double max = 7;
			if( large )
			{
				max = 5;
				fac = .25;
			}
			V2d playerDir = normalize( player->position - position );
			velocity += playerDir * fac;
			if( length( velocity ) > max )
			{
				V2d dir = normalize( velocity );
				velocity = dir * max;
			}
			//position = player->position;
			//cout << "adjust: " << position.x << ", " << position.y << endl;
		
		}
		break;
	case FALL_AT_SELF:
		{
			double fac = .4;	
			double max = 20;

			if( large )
			{
				fac = .1;
				max = 5;
			}
			
			V2d bDir = normalize( boss->position - position );
			velocity += bDir * fac;
			if( length( velocity ) > max )
			{
				V2d dir = normalize( velocity );
				velocity = dir * max;
			}
		}
		break;
	case RETRACT:
		{
			V2d diff = boss->position - position;
			V2d bDir = normalize( diff );
			velocity = bDir * ( frame * .4 );

			if( length( velocity ) > 40 )
			{
				V2d dir = normalize( velocity );
				velocity = dir * 20.0;
			}
		}
		break;
	}
	
}

void Boss_Bird::Projectile::UpdatePhysics()
{
	V2d movement = velocity / NUM_STEPS / (double)slowMultiple;

	double movementLen = length( movement );
	V2d moveDir = normalize( movement );
	double move = 0;

	do
	{
		//cout << "loop: " << movementLen << endl;
		if( movementLen > physBody.rw )
		{
			movementLen -= physBody.rw;
			move = physBody.rw;
		}
		else
		{
			move = movementLen;
			movementLen = 0;
		}

		if( move != 0 )
		{
			bool hit = ResolvePhysics( moveDir * move );
			if( hit )
			{
				HitTerrain();
				break;
			}
		}

		hitBody.globalPosition = position;

		Actor *player = boss->owner->player;
		if( player->hurtBody.Intersects( hitBody ) )
		{	
			
			//cout << "hit??" << endl;
			HitPlayer();
			break;
		}
	}
	while( movementLen > 0 );

	//IHitPlayer();
}

void Boss_Bird::Projectile::UpdatePostPhysics()
{	
	int f = 0;
	switch( state )
	{
	case NO_EXIST:
		f = 0;
		break;
	case STILL:
		f = 1;
		break;
	case HOME:
		f = 2;
		break;
	case FALL_AT_PLAYER:
		f = 2;
		break;
	case FALL_AT_SELF:
		break;
	}

	sprite.setTextureRect( boss->ts_projectile->GetSubRect( f ) );
	sprite.setPosition( position.x, position.y );

	if( large )
	{
		sprite.setScale( 2, 2 );
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
}

void Boss_Bird::Projectile::HitPlayer()
{
	hitboxInfo->kbDir = normalize( boss->owner->player->position - position );
	boss->owner->player->ApplyHit( hitboxInfo );
}

void Boss_Bird::Projectile::HitTerrain()
{
	//hitboxInfo->kbDir = normalize( boss->owner->player->position - position );
	//owner->player->ApplyHit( hitboxInfo );
}

Boss_Bird::Boss_Bird( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::BOSS_BIRD, false, 2 ), deathFrame( 0 ), moveBez( 0, 0, 1, 1 ),
	DOWN( 0, 1 ), LEFT( -1, 0 ), RIGHT( 1, 0 ), UP( 0, -1 ), pathVA( sf::Quads, MAX_PATH_SIZE * 4 ),
	attackMarkerVA( sf::Quads, 17 * 4 ), dialogue( owner, DialogueBox::BIRD ),
	gridbgVA( sf::Quads, (GRID_SIZE) * 2 * 4 )
{
	
	


	//owner->cam.Update( &owner->player );
	
	SetupCinemTiming();
	//spawned = true;
	//owner->AddEnemy( this );
	

	//assert( pathParam.size() == 3 );
	V2d ceilingExtra( 0, 96 );
	V2d groundExtra( 0, -64 );
	//startPos = Vector2f( pos.x, pos.y );
	V2d bStart = owner->poiMap["birdstart"]->pos + ceilingExtra;
	V2d dSpot = owner->poiMap["dropspot"]->pos + ceilingExtra;
	V2d lSpot = owner->poiMap["landspot"]->pos + groundExtra;
	V2d aSpot = owner->poiMap["airspot"]->pos;

	startPos = Vector2f( bStart.x, bStart.y );
	//list<Vector2i>::iterator pit = pathParam.begin();
	dropSpot = Vector2f( dSpot.x, dSpot.y );//startPos + Vector2f( (*pit).x, (*pit).y );
	//pit++;
	landSpot = Vector2f( lSpot.x, lSpot.y );//startPos + Vector2f( (*pit).x, (*pit).y );
	//pit++;
	diamondCenter = Vector2f( aSpot.x, aSpot.y );//startPos + Vector2f( (*pit).x, (*pit).y );

	position.x = dropSpot.x;
	position.y = dropSpot.y;
	//position.x = diamondCenter.x;//pos.x;
	//position.y = diamondCenter.y;

	cinemFrames[FIGHT_INTRO] = 5;
	showFace = true;
	cinemFrame = 0;
	cinem = NOCINEM;//FIGHT_INTRO;
	fightIntroAction = FI_WALK;
	
	

	throwHoldFrames = 20;
	currentAttack = NOATTACK;
	//attackFrame = 0;
	gridRatio = 1;
	gridSizeRatio = 64 * 2.5;

	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );

	position.x = diamondCenter.x;
	position.y = diamondCenter.y;

	moveIndex.x = GRID_SIZE / 2;
	moveIndex.y = GRID_SIZE / 2;

	V2d gridIndexPos = trueRight * (double)moveIndex.x + trueDown * (double)moveIndex.y;
	gridIndexPos *= gridSizeRatio;

	//cout << "grid index pos: " << gridIndexPos.x << ", " << gridIndexPos.y << endl;
	gridOriginPos = position - gridIndexPos;//V2d( pos.x, pos.y );

	int gridNumVertices = gridbgVA.getVertexCount();
	int numGridQuads = gridNumVertices / 4;
	
	V2d start = gridOriginPos;
	V2d end = gridOriginPos + trueRight * (double)(GRID_SIZE-1) * gridSizeRatio;
	V2d other = normalize( end - start );
	other = V2d( other.y, -other.x );
	double gridbgWidth = 10;
	Color gridCol = Color::Black;



	for( int i = 0; i < GRID_SIZE; ++i )
	{
		//cout << "row" << endl;
		Vector2f startLeft( (start + other * gridbgWidth ).x, (start + other * gridbgWidth ).y );
		Vector2f startRight( (start - other * gridbgWidth ).x, (start - other * gridbgWidth ).y );
		Vector2f endLeft( (end + other * gridbgWidth ).x, (end + other * gridbgWidth ).y );
		Vector2f endRight( (end - other * gridbgWidth ).x, (end - other * gridbgWidth ).y );

		gridbgVA[i * 4 + 0].position = startLeft;
		gridbgVA[i * 4 + 1].position = startRight;
		gridbgVA[i * 4 + 2].position = endRight;
		gridbgVA[i * 4 + 3].position = endLeft;


		gridbgVA[i * 4 + 0].color = gridCol;
		gridbgVA[i * 4 + 1].color = gridCol;
		gridbgVA[i * 4 + 2].color = gridCol;
		gridbgVA[i * 4 + 3].color = gridCol;

		start += trueDown * (double)gridSizeRatio;
		end += trueDown * (double)gridSizeRatio;
	}

	start = gridOriginPos;
	end = gridOriginPos + trueDown * (double)(GRID_SIZE-1) * gridSizeRatio;
	other = normalize( end - start );
	other = V2d( other.y, -other.x );
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		Vector2f startLeft( (start + other * gridbgWidth ).x, (start + other * gridbgWidth ).y );
		Vector2f startRight( (start - other * gridbgWidth ).x, (start - other * gridbgWidth ).y );
		Vector2f endLeft( (end + other * gridbgWidth ).x, (end + other * gridbgWidth ).y );
		Vector2f endRight( (end - other * gridbgWidth ).x, (end - other * gridbgWidth ).y );

		gridbgVA[i * 4 + 0 + (GRID_SIZE) * 4].position = startLeft;
		gridbgVA[i * 4 + 1 + (GRID_SIZE) * 4].position = startRight;
		gridbgVA[i * 4 + 2 + (GRID_SIZE) * 4].position = endRight;
		gridbgVA[i * 4 + 3 + (GRID_SIZE) * 4].position = endLeft;


		gridbgVA[i * 4 + 0 + (GRID_SIZE) * 4].color = gridCol;
		gridbgVA[i * 4 + 1 + (GRID_SIZE) * 4].color = gridCol;
		gridbgVA[i * 4 + 2 + (GRID_SIZE) * 4].color = gridCol;
		gridbgVA[i * 4 + 3 + (GRID_SIZE) * 4].color = gridCol;

		start += trueRight * (double)gridSizeRatio;
		end += trueRight * (double)gridSizeRatio;
	}
	

	


	//diamondCenter = diamondCenter + gridIndexPos;

	

	ts_attackIcons = owner->GetTileset( "Bosses/Bird/attackicons_32x32.png", 32, 32 );

	ts_projectile = owner->GetTileset( "Bosses/Bird/projectile_64x64.png", 64, 64 );

	ts_glide = owner->GetTileset( "Bosses/Bird/glide_256x256.png", 256, 256 );
	ts_wing = owner->GetTileset( "Bosses/Bird/wing_256x256.png", 256, 256 );
	ts_kick = owner->GetTileset( "Bosses/Bird/kick_256x256.png", 256, 256 );
	ts_intro = owner->GetTileset( "Bosses/Bird/intro_256x256.png", 256, 256 );
	ts_birdFace = owner->GetTileset( "Bosses/Bird/bird_face_384x384.png", 384, 384 );

	ts_talk = owner->GetTileset( "Bosses/Bird/talk_256x256.png", 256, 256 );
	ts_symbols0 = owner->GetTileset( "Bosses/Dialogue/Symbols/02_Symbols_256x256.png", 256, 256 );

	ts_smallRingThrow = owner->GetTileset( "Bosses/Bird/smallringthrow_256x256.png", 256, 256 );
	ts_bigRingThrow = owner->GetTileset( "Bosses/Bird/bigringthrow_256x256.png", 256, 256 );
	ts_spinStart = owner->GetTileset( "Bosses/Bird/spinstart_256x256.png", 256, 256 );
	ts_spin = owner->GetTileset( "Bosses/Bird/spin_256x256.png", 256, 256 );
	ts_escape = owner->GetTileset( "Bosses/Bird/escape_256x256.png", 256, 256 );
	//ts_dialogueBox = owner->GetTileset( "Bosses/Bird/dialoguebox_192x192.png", 192, 192 );

	ts_c01_walk = owner->GetTileset( "Bosses/Bird/c01_walk_256x256.png", 256, 256 );
	ts_c02_foottap = owner->GetTileset( "Bosses/Bird/c02_foottap_256x256.png", 256, 256 );
	ts_c03_cross = owner->GetTileset( "Bosses/Bird/c03_cross_256x256.png", 256, 256 );
	ts_c04_laugh = owner->GetTileset( "Bosses/Bird/c04_laugh_256x256.png", 256, 256 );
	ts_c05_fall = owner->GetTileset( "Bosses/Bird/c05_fall_256x256.png", 256, 256 );

	//dialogueSprite.setTexture( *ts_dialogueBox->texture );
	//dialogueSprite.setTextureRect( ts_dialogueBox->GetSubRect( 0 ) );
	showDialogue = false;
	dialogueFrame = 0;

	portrait.SetSprite( ts_birdFace, 0 );
	portrait.scaleMultiple = .5;
	
	
	//SetRelFacePos( Vector2f( 0, 0 ) );

	Vector2i blah( 0, 0 );

	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			attackNodes[i][j] = NOATTACK;
		}
	}

	

	/*attackNodes[1][1] = KICK;
	attackNodes[5][1] = WING;
	attackNodes[1][5] = WING;
	attackNodes[5][5] = SPIN;
	attackNodes[3][3] = KICK;
	attackNodes[4][4] = WING;
	attackNodes[2][2] = WING;
	attackNodes[6][6] = KICK;
	attackNodes[3][5] = SPIN;
	attackNodes[5][3] = SPIN;*/

	/*attackNodes[1][1] = SPIN;
	attackNodes[0][4] = BIG_BULLET;
	attackNodes[4][0] = SMALL_BULLET;
	attackNodes[2][3] = PUNCH;
	attackNodes[3][2] = BIG_BULLET;
	attackNodes[4][4] = SPIN;
	attackNodes[2][5] = SMALL_BULLET;
	attackNodes[5][2] = KICK;
	attackNodes[3][6] = KICK;
	attackNodes[6][3] = SMALL_BULLET;
	attackNodes[7][1] = PUNCH;
	attackNodes[1][7] = PUNCH;
	attackNodes[8][4] = BIG_BULLET;
	attackNodes[4][8] = SMALL_BULLET;
	attackNodes[7][7] = SPIN;
	attackNodes[5][6] = BIG_BULLET;
	attackNodes[6][5] = PUNCH;*/
















	/*attackNodes[1][1] = KICK;

	attackNodes[0][2] = BIG_BULLET;
	attackNodes[2][0] = SMALL_BULLET;

	attackNodes[1][3] = PUNCH;
	attackNodes[3][1] = PUNCH;

	attackNodes[2][2] = SPIN;

	attackNodes[4][2] = BIG_BULLET;
	attackNodes[2][4] = SMALL_BULLET;

	attackNodes[3][3] = KICK;*/

	attackNodes[1][1] = SPIN;

	attackNodes[0][2] = SPIN;
	attackNodes[2][0] = SPIN;

	attackNodes[1][3] = SPIN;
	attackNodes[3][1] = SPIN;

	attackNodes[2][2] = SPIN;

	attackNodes[4][2] = SPIN;
	attackNodes[2][4] = SPIN;

	attackNodes[3][3] = SPIN;
	

	SetupAttackMarkers();
	/*for( int i = 0; i < 4 * 4; ++i )
	{
		attackMarkerVA[i].position = Vector2f( 0, 0 );
	}*/
	
	

	//attackNodes[Vector2i(0, 0)] = WINGATTACK;
	//attackNodes[Vector2i(4, 0)] = KICKATTACK;
	//attackNodes[Vector2i(4, 4)] = LUNGEATTACK;
	//attackNodes[Vector2i(0, 4)] = SPINATTACK;


	testCircle.setRadius( 30 );
	testCircle.setFillColor( Color::Red );
	testCircle.setOrigin( testCircle.getLocalBounds().width / 2, 
		testCircle.getLocalBounds().height / 2 );

	testFinalCircle.setRadius( 30 );
	testFinalCircle.setFillColor( Color::Magenta );
	testFinalCircle.setOrigin( testFinalCircle.getLocalBounds().width / 2, 
		testFinalCircle.getLocalBounds().height / 2 );
	ClearPathVA();

	nodeTravelFrames = 30;
	action = PLANMOVE;
	travelFrame = 0;
	travelIndex = 0;
	testFrame = 0;
	
	pathSize = 16;//MAX_PATH_SIZE;
	moveX = false;
	//xIndexMove = 0;
	///yIndexMove = 0;
	//loop = false; //no looping on Boss_Bird for now

	bulletSpeed = 5;

	

	/*animFactor[NEUTRAL] = 1;
	animFactor[FIRE] = 1;
	animFactor[FADEIN] = 1;
	animFactor[FADEOUT] = 1;
	animFactor[INVISIBLE] = 1;

	actionLength[NEUTRAL] = 3;
	actionLength[FIRE] = 20;
	actionLength[FADEIN] = 60;
	actionLength[FADEOUT] = 90;
	actionLength[INVISIBLE] = 30;*/

	fireCounter = 0;
	receivedHit = NULL;

	
	

	originalPos = Vector2i(position.x, position.y );

	deathFrame = 0;
	
	//launcher = new Launcher( this, owner, 32, 1, position, V2d( 1, 0 ), 0, 900, true );
	//launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 1000;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Boss_Bird.png", 80, 80 );
	//ts = //owner->GetTileset( "bat_48x48.png", 48, 48 );
	//sprite.setTexture( *ts->texture );
	//sprite.setTextureRect( ts->GetSubRect( frame ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	UpdateSprite();
	//sprite.setPosition( pos.x, pos.y );


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
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	//hitboxInfo->kbDir;

	dialogueFrame = 0;

	dead = false;
	dying = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	numSmallRings = 6;
	numBigRings = 2;
	smallRings = new Projectile*[numSmallRings];
	bigRings = new Projectile*[numBigRings];
	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i] = new Projectile( this, false );
	}
	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i] = new Projectile( this, true );
	}

	UpdateHitboxes();

	fi0.push_back( SymbolInfo() );
	SymbolInfo *si = &fi0.back();
	si->ts = ts_symbols0;
	si->frame = 0;
	si->framesHold = 20;
	fi0.push_back( SymbolInfo() );
	si = &fi0.back();
	si->ts = ts_symbols0;
	si->frame = 1;
	si->framesHold = 20;

	fi1.push_back( SymbolInfo() );
	si = &fi1.back();
	si->ts = ts_symbols0;
	si->frame = 3;
	si->framesHold = 60;
	fi1.push_back( SymbolInfo() );
	si = &fi1.back();
	si->ts = ts_symbols0;
	si->frame = 4;
	si->framesHold = 60;

	fi2.push_back( SymbolInfo() );
	si = &fi2.back();
	si->ts = ts_symbols0;
	si->frame = 2;
	si->framesHold = 60;
	
	fi3.push_back( SymbolInfo() );
	si = &fi3.back();
	si->ts = ts_symbols0;
	si->frame = 5;
	si->framesHold = 60;

	fi3.push_back( SymbolInfo() );
	si = &fi3.back();
	si->ts = ts_symbols0;
	si->frame = 5;
	si->framesHold = 60;

	fi3.push_back( SymbolInfo() );
	si = &fi3.back();
	si->ts = ts_symbols0;
	si->frame = 6;
	si->framesHold = 60;

	fi3.push_back( SymbolInfo() );
	si = &fi3.back();
	si->ts = ts_symbols0;
	si->frame = 7;
	si->framesHold = 60;
	
	fi3.push_back( SymbolInfo() );
	si = &fi3.back();
	si->ts = ts_symbols0;
	si->frame = 8;
	si->framesHold = 120;

	//cout << "finish init" << endl;
}


void Boss_Bird::ResetEnemy()
{
	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i]->Reset();
	}

	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i]->Reset();
	}
	dialogueFrame = 0;
	showFace = true;
	cinemFrame = 0;

	travelFrame = 0;
	travelIndex = 0;
	action = PLANMOVE;
	ClearPathVA();
	testFrame = 0;
	moveX = false;
	//moveIndex = Vector2i( 0, 0 );
	moveIndex.x = GRID_SIZE / 2;
	moveIndex.y = GRID_SIZE / 2;

	fireCounter = 0;
	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
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
	b->launcher->DeactivateBullet( b );
}

void Boss_Bird::SetupAttackMarkers()
{
	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );

	int size = 32;
	int index = 0;
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			if( attackNodes[i][j] == NOATTACK )
			{
				continue;
			}

			V2d gridIndexPos = trueRight * (double)i + trueDown * (double)j;
			gridIndexPos *= gridSizeRatio;

			//cout << "filling attack markers : " << index << endl;
			Vector2f gip( gridIndexPos.x + gridOriginPos.x, gridIndexPos.y + gridOriginPos.y );
			//cout << "grid index pos: " << gridIndexPos.x << ", " << gridIndexPos.y << endl;
			//cout << "i: " << i << ", j: " << j << ", pos: " << gip.x << ", " << gip.y << endl;
			attackMarkerVA[index * 4 + 0].position = gip + Vector2f( -size, -size );
			attackMarkerVA[index * 4 + 1].position = gip + Vector2f( size, -size );
			attackMarkerVA[index * 4 + 2].position = gip + Vector2f( size, size );
			attackMarkerVA[index * 4 + 3].position = gip + Vector2f( -size, size );

			int realFrame = 0;
			switch( attackNodes[i][j] )
			{
			case PUNCH:
				realFrame = 0;
				break;
			case KICK:
				realFrame = 1;
				break;
			case BIG_BULLET:
				realFrame = 2;
				break;
			case SMALL_BULLET:
				realFrame = 3;
				break;
			case SPIN:
				realFrame = 4;
				break;
			}

			IntRect ir = ts_attackIcons->GetSubRect( realFrame );

			attackMarkerVA[index * 4 + 0].texCoords = Vector2f( ir.left, ir.top );
			attackMarkerVA[index * 4 + 1].texCoords = Vector2f( ir.left + ir.width, ir.top );
			attackMarkerVA[index * 4 + 2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
			attackMarkerVA[index * 4 + 3].texCoords = Vector2f( ir.left, ir.top + ir.height );

			/*attackMarkerVA[index * 4 + 0].color = Color::Red;
			attackMarkerVA[index * 4 + 1].color = Color::Red;
			attackMarkerVA[index * 4 + 2].color = Color::Red;
			attackMarkerVA[index * 4 + 3].color = Color::Red;*/

			++index;
		}
	}
	
}

void Boss_Bird::BulletHitPlayer(BasicBullet *b )
{
	b->launcher->hitboxInfo->kbDir = normalize( owner->player->position - b->position );
	b->launcher->hitboxInfo->knockback = 20;
	b->launcher->hitboxInfo->hitstunFrames = 1;
	b->launcher->hitboxInfo->damage = 0;
	b->launcher->hitboxInfo->hitlagFrames = 0;
	//cout << "hit player" << endl;
	owner->player->ApplyHit( b->launcher->hitboxInfo );

	//launcher->DeactivateBullet( this );
}

void Boss_Bird::ActionEnded()
{
	if( frame == actionLength[action] )
	{
	switch( action )
	{
	
	}
	}
}

void Boss_Bird::UpdateMovement()
{
	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );

	Vector2i currIndex = moveIndex;
	Vector2i nextIndex = moveIndex + path[travelIndex];

	V2d along;
	Vector2i dir( path[travelIndex].x, path[travelIndex].y );
	if( dir == LEFT )
	{
		along = trueLeft;
	}
	else if( dir == DOWN ) 
	{
		along  = trueDown;
	}
	else if( dir == UP )
	{
		along  = trueUp;
	}
	else if( dir == RIGHT )
	{
		along  = trueRight;
	}

	//cout << "moveindex: " << moveIndex.x << ", " << moveIndex.y << endl;
	V2d gridIndexPos = trueRight * (double)moveIndex.x + trueDown * (double)moveIndex.y;
	gridIndexPos *= gridSizeRatio;

	double val = moveBez.GetValue( (double)travelFrame / nodeTravelFrames );
	V2d curr = gridIndexPos + gridOriginPos;

	curr += along * gridSizeRatio * val;
	
	position = curr;


	//++travelFrame;
}

void Boss_Bird::SetupCinemTiming()
{
	fallTiming[0] = 4;
	fallTiming[1] = 2;
	fallTiming[2] = 8;
	fallTiming[3] = 4;
	fallTiming[4] = 2;
	fallTiming[5] = 2;
	fallTiming[6] = 2;
	fallTiming[7] = 1;
	fallTiming[8] = 3;
	fallTiming[9] = 2;
	fallTiming[10] = 8;
	fallTiming[11] = 3;
	fallTiming[12] = 1;
	fallTiming[13] = 1;
	fallTiming[14] = 2;
	fallTiming[15] = 3;
	fallTiming[16] = 2;
	fallTiming[17] = 1;
	fallTiming[18] = 3;
	fallTiming[19] = 8;

	int total = 0;
	for( int i = 0; i < 20; ++i )
	{
		fallTiming[i] *= 4;
		total += fallTiming[i];
		fallTiming[i] = total;
	}

	//cout << "TOTAL: " << total << endl;
	//124
}

void Boss_Bird::Init()
{
	
}

bool Boss_Bird::UpdateCinematic()
{

	 Vector2f extra0( -250, 100 );
	 Vector2f extra1( -250, -100 );

	 Vector2f pextra0( 0, 200 );
	 Vector2f pextra1( 0, -200 );

	 Vector2f dextra0( -200, 0 );
	 
	//if( cinem != NOCINEM )
	//{
	//	switch( cinem )
	//	{
	//	case FIGHT_INTRO:
	//		
	//		sprite.setTexture( *ts_intro->texture );
	//		sprite.setTextureRect( ts_intro->GetSubRect( 0 ) );

	//		//faceSprite.setTexture( *ts_birdFace->texture );
	//		
	//		break;
	//	}

	//	return;
	//}

	//cutPlayerInput = true;

	assert( cinem != NOCINEM );
	switch( cinem )
	{
	case FIGHT_INTRO:
		{
		switch( fightIntroAction )
		{
		case FI_WALK:
			showFace = false;
			if( cinemFrame == 240 + 30 )
			{
				portrait.Open();
				cinemFrame = 0;
				fightIntroAction = FI_FOOTTAP;
				portrait.SetPosition( dropSpot + pextra0 );
				dialogue.SetPosition( dropSpot + pextra0 + dextra0 );
				portrait.SetSprite( ts_birdFace, 1 );
			}
			break;
		case FI_FOOTTAP:
			showFace = false;
			if( cinemFrame == 60 )
			{
				
				cinemFrame = 0;
				fightIntroAction = FI_CROSS;
			}
			break;
		case FI_CROSS:
			showFace = false;
			if(  (cinemFrame / 8) == 3 )
			{
				
				cinemFrame = 0;
				fightIntroAction = FI_LAUGH;
				portrait.SetSprite( ts_birdFace, 1 );
			}
			break;
		case FI_LAUGH:
			showFace = true;
			if( cinemFrame == 60 )
			{
				
				cinemFrame = 0;
				fightIntroAction = FI_EXPLAIN0;
				portrait.SetSprite( ts_birdFace, 1 );
				dialogue.SetSymbols( &fi0 );
				dialogue.Open();
			}
			break;
		case FI_EXPLAIN0:
			//cout << "0 " << endl;
			showFace = true;
			if( cinemFrame > 60 && owner->currInput.start && !owner->prevInput.start )
			{
				cinemFrame = 0;
				fightIntroAction = FI_EXPLAIN1;
				dialogue.SetSymbols( &fi1 );
				portrait.SetSprite( ts_birdFace, 2 );
			}
			break;
		case FI_EXPLAIN1:
			//cout << "1 " << endl;
			showFace = true;
			if( cinemFrame > 60 && owner->currInput.start && !owner->prevInput.start )
			{
				cinemFrame = 0;
				fightIntroAction = FI_EXPLAIN2;
				dialogue.SetSymbols( &fi2 );
				portrait.SetSprite( ts_birdFace, 2 );
			}
			break;
		case FI_EXPLAIN2:
		//	cout << "2 " << endl;
			showFace = true;
			if( cinemFrame > 60 && owner->currInput.start && !owner->prevInput.start )
			{
				cinemFrame = 0;
				fightIntroAction = FI_EXPLAIN3;
				dialogue.SetSymbols( &fi3 );
				portrait.SetSprite( ts_birdFace, 3 );
			}
			break;
		case FI_EXPLAIN3:
		//	cout << "3 " << endl;
			showFace = true;
			if( cinemFrame > 60 && owner->currInput.start && !owner->prevInput.start )
			{
				cinemFrame = 0;
				fightIntroAction = FI_FALL;
				position.x = landSpot.x;
				position.y = landSpot.y;
				portrait.Close();
				dialogue.Close();
				
			}
			break;
		case FI_FALL:
			showFace = false;
			if( cinemFrame == 249 )//125 ) //125?
			{
				
				
				cinemFrame = 0;
				fightIntroAction = FI_GROUNDWAIT;
				
			}
			else if( cinemFrame == 150 )
			{
				portrait.SetPosition( landSpot + pextra1 );
				portrait.Open();
				portrait.SetSprite( ts_birdFace, 14 );
			}
			break;
		case FI_GROUNDWAIT:
			showFace = true;
			if( cinemFrame == 60 )
			{
				portrait.Close();
				cinemFrame = 0;
				fightIntroAction = FI_FLY;
				position.x = diamondCenter.x;
				position.y = diamondCenter.y;
			}
			break;
		case FI_FLY:
			showFace = false;
			if( cinemFrame == 60 )
			{
				cinem = NOCINEM;
				owner->cam.EaseOutOfManual( 120 );

				owner->currMusic->stop();
				owner->currMusic = owner->soundManager->GetMusic( "Audio/Music/02_bird_fight.ogg" );
				owner->currMusic->setLoop( true );
				owner->currMusic->play();
				
				//owner->cam.SetManual( false );
				//owner->cutPlayerInput = false;
				return false;
			}
			break;
		}

		switch( fightIntroAction )
		{
		case FI_WALK:
			{
				CubicBezier bez( 0, 0, 1, 1 );

				if( cinemFrame >= 30 )
				{
					if( cinemFrame == 30 )
					{
						owner->cam.SetManual( true );
					}
				int c = cinemFrame - 30;
				//cout << "returning pre:" << cinemFrame << endl;
				float z = bez.GetValue( (double)c / 240 );

				Vector2f po = startPos * ( 1 - z ) + dropSpot * z;
				Vector2f trueSpot = dropSpot + extra0;
				owner->cam.Set( ( trueSpot * 1.f/60.f + owner->cam.pos * 59.f/60.f ),
					1, 0 );

				sprite.setPosition( po.x, po.y );

				sprite.setTexture( *ts_c01_walk->texture );
				int f = (c / 8) % 4;
				//cout << "f: " << f << endl;
				sprite.setTextureRect( ts_c01_walk->GetSubRect( f ) );
				}
			}
			break;
		case FI_FOOTTAP:
			{
				sprite.setTexture( *ts_c02_foottap->texture );

				//10 frames then 6 frames
				int c = cinemFrame % 16;
				if( c >= 8 )
				{
					c = 1;
				}
				else
				{
					c = 0;
				}
				int f = c;//(cinemFrame / 8) % 2;
				sprite.setTextureRect( ts_c02_foottap->GetSubRect( f ) );			
			}
			break;
		case FI_CROSS:
			{
				//4, 4, 8
				int c = cinemFrame;
				if( c < 4 )
				{
					c = 0;
				}
				else if( c < 8 )
				{
					c = 1;
				}
				else
				{
					c = 2;
				}

				sprite.setTexture( *ts_c03_cross->texture );
				int f = c;//(cinemFrame / 8);
				sprite.setTextureRect( ts_c03_cross->GetSubRect( f ) );
			}
			break;
		case FI_LAUGH:
			{
				sprite.setTexture( *ts_c04_laugh->texture );
				int f = (cinemFrame / 6) % 2;
				sprite.setTextureRect( ts_c04_laugh->GetSubRect( f ) );		
			}
			break;
		case FI_EXPLAIN0:
			{
				sprite.setTexture( *ts_talk->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 1 ) );			
			}
			break;
		case FI_EXPLAIN1:
			{
				sprite.setTexture( *ts_talk->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 2 ) );	
			}
			break;
		case FI_EXPLAIN2:
			{
				sprite.setTexture( *ts_talk->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 0 ) );			
			}
			break;
		case FI_EXPLAIN3:
			{
				sprite.setTexture( *ts_talk->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 0 ) );	
			}
			break;
		case FI_FALL:
			{
				int ind = 0;
				for( int i = 0; i < 20; ++i )
				{
					if( cinemFrame < fallTiming[i] )
					{
						ind = i;
						break;
					}
				}

				sprite.setTexture( *ts_c05_fall->texture );
				sprite.setTextureRect( ts_c05_fall->GetSubRect( ind ) );

				int len = fallTiming[6] - fallTiming[2];
				int c = cinemFrame - fallTiming[2];
				if( ind > 2 && c <= len && c >= 0 )
				{
				CubicBezier bez( 0, 0, 1, 1 );
				float z = bez.GetValue( (double)c / len );
			
				Vector2f po =  dropSpot * ( 1 - z ) + landSpot * z;
			
				//owner->cam.manual = false;
				Vector2f camPo = ( dropSpot + extra0 ) * ( 1 - z ) + ( landSpot + extra1 ) * z;
				owner->cam.Set( camPo,
					1, 0 );

				sprite.setPosition( po.x, po.y );
				}
				

				
				
				
				//sprite.setPosition( po.x, po.y );
			}
			break;
		case FI_GROUNDWAIT:
			{
				//CubicBezier bez( 0, 0, 1, 1 );
				//float z = bez.GetValue( (double)cinemFrame / 60 );
			
				//owner->cam.Set( dropSpot * ( 1 - z ) + landSpot * z,
				//	1, 0 );

				sprite.setTexture( *ts_intro->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 2 ) );
			}
			break;
		case FI_FLY:
			{
				CubicBezier bez( 0, 0, 1, 1 );
				float z = bez.GetValue( (double)cinemFrame / 60 );
			
//
				Vector2f po = landSpot * ( 1 - z ) + diamondCenter * z;
				Vector2f camPo = (landSpot + extra1 ) * (1 -z) + diamondCenter * z;
				owner->cam.Set( po,
					1, 0 );

				sprite.setTexture( *ts_intro->texture );
				sprite.setTextureRect( ts_intro->GetSubRect( 2 ) );
				sprite.setPosition( po.x, po.y );
			}
			break;
		}

		sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	}
	}


	portrait.Update();
	dialogue.Update();

	++cinemFrame;

	return true;
}

void Boss_Bird::UpdatePrePhysics()
{
	if( cinem != NOCINEM )
	{
		bool stillCinematic = UpdateCinematic();
		if( stillCinematic )
			return;
	}

	V2d playerPos = owner->player->position;

	ActionEnded();

	launcher->UpdatePrePhysics();

	switch( action )
	{
	case PLANMOVE:
		break;
	case MOVE:
		break;
	case ATTACK_SMALL_BULLET:
		if( frame == 18 * 2 )
		{
			action = MOVE;
		}
		break;
	case ATTACK_KICK:
		if( frame == 10 ) //back on the grid
		{
			action = ATTACK_KICKRETREAT;
		}
		break;
	case ATTACK_KICKRETREAT:
		if( frame == 30 + 30 )
		{
			action = MOVE;
		}
		break;
	case ATTACK_SPIN:
		if( frame == 60 ) //not sure if variable or gets harder as you speed up
		{
			action = MOVE;
		}
		break;
	case ATTACK_BIG_BULLET:
		if( frame == 20 * 2 )
		{
			action = MOVE;
		}
		break;
	case ATTACK_PUNCH:
		if( frame == 20 )
		{
			action = ATTACK_PUNCHRETREAT;
		}
		break;
	case ATTACK_PUNCHPLAN:
		if( frame == 60 )
		{
			action = MOVE;
		}
		break;
	case ATTACK_PUNCHRETREAT:
		if( frame == 30 + 30 )
		{
			action = MOVE;
		}
		break;

	}

	
	if ( action != ATTACK_KICK && action != ATTACK_KICKRETREAT && action != ATTACK_PUNCH &&
		action != ATTACK_PUNCHRETREAT && action != ATTACK_PUNCHPLAN )
	{
		++travelFrame;
		if( travelFrame == nodeTravelFrames )	
		{
			if( action == MOVE || action == ATTACK_SMALL_BULLET 
				|| action == ATTACK_SPIN
				|| action == ATTACK_BIG_BULLET )
			{
				//cout << "moving moveIndex w/ travelIndex : " << travelIndex << endl;
				//cout << "move index was: " << moveIndex.x << ", " << moveIndex.y << endl;
				moveIndex += path[travelIndex];
			}
		
			travelFrame = 0;
			travelIndex++;
			if( travelIndex == pathSize )
			{
				frame = 0;
				if( action == PLANMOVE )
				{
					action = MOVE;
				}
				else if( action == MOVE 
					|| action == ATTACK_SMALL_BULLET
					|| action == ATTACK_BIG_BULLET
					|| action == ATTACK_SPIN 
					|| action == ATTACK_KICK )
				{
					if( action == ATTACK_BIG_BULLET )
					{
						sprite.setRotation( 0 );
					}
					action = PLANMOVE;
				}
				else
				{
					assert( 0 );
				}
			}
		}
	}
	

	if( (action == MOVE || action == ATTACK_SMALL_BULLET || action == ATTACK_BIG_BULLET
		|| action == ATTACK_SPIN )
		&& ( travelIndex > 0 && travelFrame == 0 && travelIndex != pathSize ) )
	{
		//cout << "moveIndex: " << moveIndex.x << ", " << moveIndex.y << endl;
		AttackType at = attackNodes[moveIndex.x][moveIndex.y];
		if( at != NOATTACK )
		{
			switch( at )
			{
			case SMALL_BULLET:
				action = ATTACK_SMALL_BULLET;
				break;
			case BIG_BULLET:
				action = ATTACK_BIG_BULLET;
				//cout << "kick: " << travelIndex << endl;
				break;
			case KICK:
				action = ATTACK_KICK;
				//frame = 0;
				break;
			case SPIN:
				action = ATTACK_SPIN;
				break;
			case PUNCH:
				action = ATTACK_PUNCHPLAN;
				break;
			}

			//V2d dir = GetLungeDir();

			//currentAttack = at;
			frame = 0;
			//attackFrame = 0;
			cout << "at: " << (int)at << " x: " << moveIndex.x << ", " << moveIndex.y << endl;
		}
	}
		
	
	switch( action )
	{
	case PLANMOVE:
		if( frame == 0 )
		{
			CreatePath();
			travelFrame = 0;
			travelIndex = 0;		

			

			
			//projectiles[0]->position = owner->poiMap["bullettest"]->pos;
			//projectiles[0]->state = Projectile::State::FALL_AT_PLAYER;
			//launcher->position = owner->poiMap["bullettest"]->pos;
			//launcher->bulletSpeed = 0;
			//launcher->Fire();
		}

		UpdatePathVA();
		break;
	case MOVE:
		if( frame == 0 )
		{
			travelFrame = 0;
			travelIndex = 0;
			//projectiles[0]->state = Projectile::State::RETRACT;
		}
		UpdateMovement();
		break;
	case ATTACK_SMALL_BULLET:
		if( frame == 0 )
		{
			ringThrowRight = (playerPos.x >= position.x);
			ringThrowDown = (playerPos.y >= position.y);
		}
		else if( frame == 9 * 2 )
		{
			if( smallRings[0]->state != Projectile::State::NO_EXIST )
			{
			}

			smallRings[0]->position = position;//owner->poiMap["bullettest"]->pos;
			smallRings[0]->state = Projectile::State::FALL_AT_PLAYER;
			smallRings[0]->frame = 0;
			smallRings[0]->velocity = V2d( 0, 0 );
		}
		UpdateMovement();
		break;
	case ATTACK_KICK:
		{
		//cout << "kicking" << endl;
		if( frame == 0 )
		{
			lungeStart = position;

			
			V2d lungeDir = GetLungeDir();
			rayStart = position;
			rayEnd = position + lungeDir * 2000.0; //lol cant wait for this to break
			rcEdge = NULL;
			RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );
			assert( rcEdge != NULL );

			
			lungeEnd = rcEdge->GetPoint( rcQuantity );
			testFinalCircle.setPosition( lungeEnd.x, lungeEnd.y );
		}
		
		double lungeLength = 9;
				

		position = lungeStart * (1.0 - frame / lungeLength ) + lungeEnd *( frame / lungeLength );

		break;
		}
	case ATTACK_KICKRETREAT:
		{
			
			if( frame < 30 )
			{
				//pause for dramatic effect
			}
			else
			{
				double retreatLength = 29;
				int f = frame - 30;
				cout << "f: " << f << endl;
				position = lungeStart * ((f) / retreatLength ) 
				+ lungeEnd *(1.0 - (f) / retreatLength );
			}
			
			//--travelFrame;
			
			break;
		}
	case ATTACK_SPIN:
		if( frame == 0 )
		{
			if( bigRings[0]->state != Projectile::State::NO_EXIST )
			{
				bigRings[0]->state = Projectile::State::FALL_AT_SELF;
				bigRings[0]->frame = 0;
			}
			if( smallRings[0]->state != Projectile::State::NO_EXIST )
			{
				smallRings[0]->state = Projectile::State::FALL_AT_SELF;
				smallRings[0]->frame = 0;
			}
		}
		UpdateMovement();
		break;
	case ATTACK_BIG_BULLET:
		if( frame == 0 )
		{
			ringThrowRight = (playerPos.x >= position.x);
			ringThrowDown = (playerPos.y >= position.y);
		}
		else if( frame == 9 * 2 )
		{
			bigRings[0]->position = position;//owner->poiMap["bullettestlarge"]->pos;
			bigRings[0]->state = Projectile::State::FALL_AT_PLAYER;
			bigRings[0]->frame = 0;
			bigRings[0]->velocity = V2d( 0, 0 );
		}
		UpdateMovement();
		break;
	case ATTACK_PUNCH:
		break;
	case ATTACK_PUNCHRETREAT:
		break;
	case ATTACK_PUNCHPLAN:
		//if( frame == 0 )
		//{
		//	lungeStart = position;
		//}

		//if( frame < 30 )
		//{
		//	//pause for dramatic effect
		//}
		//else
		//{
		//	double retreatLength = 29;
		//	int f = frame - 30;
		//	cout << "f: " << f << endl;
		//	position = lungeStart * ((f) / retreatLength ) 
		//	+ lungeEnd *(1.0 - (f) / retreatLength );
		//}
		break;
	}

	
	

	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			//AttemptSpawnMonitor();
			dying = true;
			cout << "dying true what" << endl;
		}

		receivedHit = NULL;
	}


	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i]->UpdatePrePhysics();
	}

	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i]->UpdatePrePhysics();
	}
	//if( !dying && !dead && action == FIRE && frame == actionLength[FIRE] - 1 )// frame == 0 && slowCounter == 1 )
	//{
	//	//cout << "firing" << endl;
	//	launcher->position = position;
	//	launcher->facingDir = normalize( owner->player->position - position );
	//	//cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
	//	//	launcher->facingDir.y << endl;
	
	//	fireCounter = 0;
	//	//testLauncher->Fire();
	//}

	/*if( latchedOn )
	{
		basePos = owner->player->position + offsetPlayer;
	}*/
}

void Boss_Bird::UpdatePhysics()
{
	if( cinem != NOCINEM )
		return;

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

	launcher->UpdatePhysics();

	if( !dead && !dying )
	{
		/*if( action == NEUTRAL )
		{
			Actor *player = owner->player;
			if( length( player->position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}*/
		PhysicsResponse();
	}
	//return;

	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i]->UpdatePhysics();
	}

	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i]->UpdatePhysics();
	}
}

void Boss_Bird::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_GREEN, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Boss_Bird received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 8 );
			}
		
			health -= 20;

			if( health <= 0 )
				dead = true;

			receivedHit = NULL;*/
			//dead = true;
			//receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
		//	cout << "Boss_Bird just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Bird::ClearPathVA()
{
	for( int i = 0; i < MAX_PATH_SIZE * 4; ++i )
	{
		pathVA[i].position = Vector2f( 0, 0 );
		pathVA[i].color = COLOR_GREEN;
	}
}

sf::Vector2<double> Boss_Bird::GetLungeDir()
{
	V2d playerPos = owner->player->position;
	V2d playerDir = normalize( playerPos - position );
	

	/*double angle = atan2( playerDir.y, playerDir.x );
	int rot = 60;
	angle += (((rand() % rot) - rot / 2 ) / 180.0 * PI);
	V2d newDir( cos( angle ), sin( angle ) );

	V2d spot = position + newDir * 300.0;*/



	//testFinalCircle.setPosition( spot.x, spot.y );

	//cout << "angle: " << angle << endl;
	//return newDir;
	return playerDir;
}

void Boss_Bird::SetRelFacePos( sf::Vector2f &pos )
{
	portrait.sprite.setPosition( pos + Vector2f( position.x, position.y ) );
}

void Boss_Bird::UpdatePostPhysics()
{
	if( cinem != NOCINEM )
	{
		//UpdateSprite();
		//++cinemFrame;
		
		return;
	}
		

	launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

	

	if( deathFrame == 60 && dying )
	{
		//cout << "switching dead" << endl;
		dying = false;
		dead = true;
		//cout << "REMOVING" << endl;
		//testLauncher->Reset();
		//owner->RemoveEnemy( this );
		//return;
	}

	if( dead && launcher->GetActiveCount() == 0 )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	launcher->UpdateSprites();

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		//++attackFrame;
		slowCounter = 1;
		++fireCounter;
	
		if( dying )
		{
			//cout << "deathFrame: " << deathFrame << endl;
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}


	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i]->UpdatePostPhysics();
	}

	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i]->UpdatePostPhysics();
	}
}

void Boss_Bird::UpdateSprite()
{
	if( !dying && !dead )
	{
		sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
		Vector2i dir( path[travelIndex].x, path[travelIndex].y );
		switch( action )
		{
		case MOVE:
			sprite.setTexture( *ts_glide->texture );

			if( dir.x > 0 )
			{
				//down right
				sprite.setTextureRect( ts_glide->GetSubRect( 2) );
			}
			else if( dir.y > 0 )
			{
				//down left
				sprite.setTextureRect( ts_glide->GetSubRect( 0 ) );
			}
			else if( dir.x < 0 )
			{
				//up left
				sprite.setTextureRect( ts_glide->GetSubRect( 1 ) );
			}
			else if( dir.y < 0 )
			{
				//up right
				sprite.setTextureRect( ts_glide->GetSubRect( 3 ) );
			}
			else
			{
				cout << "travelIndex: " << travelIndex << endl;
				cout << "dir: " << dir.x << ", " << dir.y << endl;
				assert( false );
			}
			break;
		case PLANMOVE:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 0 ) );
			break;
		case ATTACK_SMALL_BULLET:
			{
			sprite.setTexture( *ts_smallRingThrow->texture );
			IntRect ir = ts_smallRingThrow->GetSubRect( frame / 2 );

			if( !ringThrowRight )
			{
				ir.left += ir.width;
				ir.width = -ir.width;
			}
			if( !ringThrowDown )
			{
				ir.top += ir.height;
				ir.height = -ir.height;
			}

			sprite.setTextureRect( ir );
			
			}
			break;
		case ATTACK_KICK:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 1 ) );
			break;
		case ATTACK_SPIN:
			sprite.setTexture( *ts_spin->texture );
			sprite.setTextureRect( ts_spin->GetSubRect( (frame / 2) % 5 ) );
			break;
		case ATTACK_BIG_BULLET:
			{
				sprite.setTexture( *ts_bigRingThrow->texture );
				IntRect ir = ts_bigRingThrow->GetSubRect( frame / 2 );

				if( !ringThrowRight )
				{
					ir.left += ir.width;
					ir.width = -ir.width;
				}
				if( !ringThrowDown )
				{
					ir.top += ir.height;
					ir.height = -ir.height;
				}

				sprite.setTextureRect( ir );
				

				//sf::Vector2i p = path[travelIndex-1];

				////cout << "Start once" << endl;
				//if( p.x > 0 )
				//{
				//	//cout << "270" << endl;
				//	sprite.setRotation( -270 );
				//}
				//else if( p.x < 0 )
				//{
				//	//cout << "90" << endl;
				//	sprite.setRotation( -90 );
				//}
				//else if( p.y > 0 )
				//{
				//	//cout << "180" << endl;
				//	sprite.setRotation( -180 );
				//}
				//else if( p.y < 0 )
				//{
				//	//cout << "dont rotate" << endl;
				//	sprite.setRotation( 0 );
				//	//leave it
				//}
				//cout << "end" << endl;

				break;
			}
		case ATTACK_PUNCHPLAN:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 0 ) );
			break;
		case ATTACK_PUNCH:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 0 ) );
			break;
		case ATTACK_PUNCHRETREAT:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 0 ) );
			break;
		case ATTACK_KICKRETREAT:
			sprite.setTexture( *ts_wing->texture );
			sprite.setTextureRect( ts_wing->GetSubRect( 0 ) );
			break;
			
			
		}

		sprite.setPosition( position.x, position.y );
		
	}
	else if( false )
	{
		//cout << "dying" << endl;
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

void Boss_Bird::Draw( sf::RenderTarget *target )
{
	//if( showFace )
	{
		//cout << "drawing faceSprite " << endl;
		portrait.Draw( target );
		//target->draw( faceSprite );
	}
	
	dialogue.Draw( target );
	if( showDialogue )
	{
		//target->draw( dialogueSprite );
	}
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
		
		target->draw( gridbgVA );
		
		target->draw( pathVA );
		target->draw( attackMarkerVA, ts_attackIcons->texture );
		target->draw( sprite );

		if( action == PLANMOVE )
		{
			
			target->draw( testCircle );
			
		}
		//else if( action == ATTACK_LUNGESTART )
		//{
		//	//testFinalCircle.setPosition( position.x, position.y );
		//	target->draw( testFinalCircle );
		//}
	}
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}

	for( int i = 0; i < numSmallRings; ++i )
	{
		smallRings[i]->Draw( target );
	}

	for( int i = 0; i < numBigRings; ++i )
	{
		bigRings[i]->Draw( target );
	}
}

void Boss_Bird::DrawMinimap( sf::RenderTarget *target )
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

bool Boss_Bird::IHitPlayer()
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
pair<bool,bool> Boss_Bird::PlayerHitMe()
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

bool Boss_Bird::DirIsValid( sf::Vector2i &testIndex,
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

void Boss_Bird::CreatePath()
{
	//cout << "CREATE PATH START" << endl;
	ClearPathVA();
	sf::Vector2i testIndex = moveIndex;

	if( testIndex.x == 0 && testIndex.y == 0 )	
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else
		{
			path[0] = RIGHT;
		}
	}
	else if( testIndex.x == GRID_SIZE-1 && testIndex.y == GRID_SIZE-1 )
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = UP;
		}
		else
		{
			path[0] = LEFT;
		}
	}
	else if( testIndex.x == 0 && testIndex.y == GRID_SIZE - 1 )
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = UP;
		}
		else
		{
			path[0] = RIGHT;
		}
	}
	else if( testIndex.x == GRID_SIZE - 1 && testIndex.y == 0 )
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = LEFT;
		}
		else
		{
			path[0] = DOWN;
		}
	}
	else if( testIndex.x == 0 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = UP;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else
		{
			path[0] = DOWN;
		}
	}
	else if( testIndex.y == 0 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else if( r == 1 )
		{
			path[0] = LEFT;
		}
		else
		{
			path[0] = RIGHT;
		}
	}
	else if( testIndex.x == GRID_SIZE-1 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else if( r == 1 )
		{
			path[0] = LEFT;
		}
		else
		{
			path[0] = UP;
		}
	}
	else if( testIndex.y == GRID_SIZE-1 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = LEFT;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else
		{
			path[0] = UP;
		}
	}
	else
	{
		int r = rand() % 4;
		if( r == 0 )
		{
			path[0] = LEFT;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else if( r == 2 )
		{
			path[0] = DOWN;
		}
		else
		{
			path[0] = UP;
		}
	}

	testIndex += path[0];
	for( int i = 1; i < pathSize; ++i )
	{
		int numOptions = 0;
		
		sf::Vector2i leftTurn( path[i-1].y, -path[i-1].x );
		bool validLeftTurn = DirIsValid( testIndex, leftTurn );
		sf::Vector2i rightTurn( -path[i-1].y, path[i-1].x );
		bool validRightTurn = DirIsValid( testIndex, rightTurn );
		sf::Vector2i forward = path[i-1];
		bool validForward = DirIsValid( testIndex, forward );
		
		if( validLeftTurn && validRightTurn && validForward )
		{
			//cout << "a" << endl;
			int r = rand() % 3;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else if( r == 1 )
			{
				path[i] = rightTurn;
			}
			else
			{
				path[i] = forward;
			}
		}
		else if( validLeftTurn && validRightTurn )
		{
			//cout << "b" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else
			{
				path[i] = rightTurn;
			}
		}
		else if( validForward && validRightTurn )
		{
			//cout << "c" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = forward;
			}
			else
			{
				path[i] = rightTurn;
			}
		}
		else if( validLeftTurn && validForward )
		{
			//cout << "d" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else
			{
				path[i] = forward;
			}
		}
		else if( validLeftTurn )
		{
			path[i] = leftTurn;
		}
		else if( validForward )
		{
			path[i] = forward;
		}
		else if( validRightTurn )
		{
			path[i] = rightTurn;
		}
		else
		{
			Vector2i blahLeft = testIndex + leftTurn;
			Vector2i blahRight = testIndex + rightTurn;
			Vector2i blahForward = testIndex + forward;
			cout << (int)validLeftTurn << ", " << (int)validRightTurn << ", " << (int)validForward << endl;
			cout << "left: " << blahLeft.x << ", " << blahLeft.y << 
				", right: " << blahRight.x << ", " << blahRight.y << 
				", forward: " << blahForward.x << ", " << blahForward.y << endl;
			assert( 0 && "what options is this" );
		}

		
		//cout << "testindex: " << testIndex.x 
		//	<< ", " << testIndex.y << ", path[ " << i << "]: " << path[i].x << ", "<< path[i].y << endl;
		testIndex += path[i];
	}

	finalIndex = testIndex;
	//cout << "finalIndex: " << finalIndex.x << ", " << finalIndex.y << endl;

	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );

	V2d gridIndexPos = trueRight * (double)finalIndex.x + trueDown * (double)finalIndex.y;
	gridIndexPos *= gridSizeRatio;
	V2d curr = gridIndexPos + gridOriginPos;

	//testFinalCircle.setPosition( curr.x, curr.y );
}

void Boss_Bird::UpdatePathVA()
{
	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );
	 
	Vector2i testIndex = moveIndex;
	for( int i = 0; i <= travelIndex; ++i )
	{
		
		
		Vector2i dir( path[i].x, path[i].y );
		V2d along;
		if( dir == LEFT )
		{
			along = trueLeft;
		}
		else if( dir == DOWN ) 
		{
			along  = trueDown;
		}
		else if( dir == UP )
		{
			along  = trueUp;
		}
		else if( dir == RIGHT )
		{
			along  = trueRight;
		}
		
		V2d norm( along.y, -along.x );

		double height = 4;

		V2d gridIndexPos = trueRight * (double)testIndex.x + trueDown * (double)testIndex.y;
		gridIndexPos *= gridSizeRatio;


		double val = moveBez.GetValue( (double)travelFrame / nodeTravelFrames );
		V2d curr = gridIndexPos + gridOriginPos;
		V2d next = curr;
		if( i == travelIndex )
		{
			//in progress
			next += along * gridSizeRatio * val;
			testCircle.setPosition( next.x, next.y );
		}
		else
		{
			//non-in progresss
			next += along * gridSizeRatio;
		}

		V2d c0 = curr + norm * height;
		V2d c1 = next + norm * height;
		V2d c2 = next - norm * height;
		V2d c3 = curr - norm * height;


		pathVA[i*4 + 0].position = Vector2f( c0.x, c0.y );
		pathVA[i*4 + 1].position = Vector2f( c1.x, c1.y );
		pathVA[i*4 + 2].position = Vector2f( c2.x, c2.y );
		pathVA[i*4 + 3].position = Vector2f( c3.x, c3.y );

		testIndex += path[i];
	}
}

void Boss_Bird::HandleRayCollision( Edge *edge, double edgeQuantity, 
	double rayPortion )
{
	double edgeLength = length( edge->v1 - edge->v0 );
	if( edgeLength < 300 )
		return;
	if( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - rayStart ) < 
		length( rcEdge->GetPoint( rcQuantity ) - rayStart ) )
	{
		rcEdge = edge;
		rcQuantity = edgeQuantity;
	}
}
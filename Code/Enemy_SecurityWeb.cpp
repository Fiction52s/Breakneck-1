#include "Enemy.h"
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

SecurityWeb::SecurityWeb( GameSession *owner, bool hasMonitor, 
	sf::Vector2i &pos, int p_numProtrusions, float p_angleOffset, 
	double p_bulletSpeed )
	:Enemy( owner, Enemy::CORALNANOBOTS, hasMonitor, 4 ), 
	angleOffset( p_angleOffset ), bulletSpeed( p_bulletSpeed ),
	maxProtLength( 400 )
{
	ts = owner->GetTileset( "coral_128x128.png", 128, 128 );

	initHealth = 40;
	health = initHealth;

	targetFrames = 30;

	receivedHit = NULL;
	position = V2d( pos.x, pos.y );
	numProtrusions = p_numProtrusions;

	armLength = new int[numProtrusions];

	double offset = angleOffset / PI * 180;
	offset = floor( offset + .5 );

	sf::Transform rot;
	rot.rotate( offset );
	Vector2f transPoint( 0, -maxProtLength );

	//edges = new VertexArray( sf::Quads, numProtrusions * 4 );
	nodes = new VertexArray( sf::Quads, numProtrusions * 4 );
	origins = new V2d[numProtrusions];
	edgeHitboxes = new CollisionBox[numProtrusions];


	//VertexArray &eva = *edges;
	//VertexArray &pva = *edges;

	rayStart = position;

	int nodeSize = 128;
	int halfNode = 64;

	double quadWidth = 128.0;

	double edgeWidth = 10;
	Vector2f fPos( pos.x, pos.y );
	for( int i = 0; i < numProtrusions; ++i )
	{
		double currRadius = maxProtLength;
		Vector2f temp = rot.transformPoint( transPoint );
		rayEnd = V2d( temp.x, temp.y ) + rayStart;
		rcEdge = NULL;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		if( rcEdge != NULL )
		{
			V2d rpos = rcEdge->GetPoint( rcQuantity );
			double len = length( rpos - position );
			len = floor( len + .5 );
			if( len < currRadius )
				currRadius = len;
		}

		armLength[i] = ceil( currRadius );
	}

	int totalQuadsArms = 0;
	for( int i = 0; i < numProtrusions; ++i )
	{
		int armLen = armLength[i];
		int quads = ceil( armLen / quadWidth );
		
		totalQuadsArms += quads;
		//cout << "total quad arms is: " << totalQuadsArms << endl;
	}

	armVA = new VertexArray( sf::Quads, totalQuadsArms * 4 );
	VertexArray &ava = *armVA;

	//cout << "VA TOTAL vertices: " << armVA->getVertexCount() << endl;

	int currQuad = 0;
	for( int i = 0; i < numProtrusions; ++i )
	{
		int armLen = armLength[i];
		Vector2f currOffset( 0, -armLen );
		Vector2f currPoint = rot.transformPoint( currOffset );
		V2d currPointD( currPoint.x, currPoint.y );
		origins[i] = currPointD + position;

		V2d along = normalize( origins[i] - position );
		V2d other( along.y, -along.x );

		int quads = ceil( armLen / quadWidth );
		int remainder = armLen % (int)quadWidth;
		//might cause a random bug someday cuz of overflow quads? calculating ceil twice



		for( int j = 0; j < quads; ++j )
		{
			V2d start = position + along * quadWidth * (double)j;
			V2d end;
			
			if( j == quads - 1 )
			{
				end = origins[i];
			}
			else
			{
				end = start + along * quadWidth;
			}

			V2d p0 = start + other * quadWidth / 2.0;//edgeWidth;
			V2d p1 = end + other * quadWidth / 2.0;//edgeWidth;
			V2d p2 = end - other * quadWidth / 2.0;//edgeWidth;
			V2d p3 = start - other * quadWidth / 2.0;//edgeWidth;

			

			int ind = currQuad + j;

			//cout << "current index: i: " << i << ", ind: " << ind << endl;

			ava[ind * 4+0].position = Vector2f( p0.x, p0.y );//Vector2f(-halfNode,-halfNode  ) + currPoint;  
			ava[ind * 4+1].position = Vector2f( p1.x, p1.y );//Vector2f( halfNode, -halfNode ) + currPoint; 
			ava[ind * 4+2].position = Vector2f( p2.x, p2.y );//Vector2f( halfNode, halfNode  ) + currPoint; 
			ava[ind * 4+3].position = Vector2f( p3.x, p3.y );//Vector2f( -halfNode, halfNode ) + currPoint;
			//cout << "currPoint: " << currPoint.x << ", " << currPoint.y << endl;

			IntRect sub = ts->GetSubRect( 3 ); 

			if( j == quads - 1 )
			{
				sub.width = remainder;	
			}
			ava[ind*4+3].texCoords = Vector2f( sub.left, sub.top );
			ava[ind*4+0].texCoords = Vector2f( sub.left + sub.width, sub.top );
			ava[ind*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			ava[ind*4+2].texCoords = Vector2f( sub.left, sub.top + sub.height );
		}

		rot.rotate( 360.f / numProtrusions );
		currQuad += quads;

		
		/*eva[i*4+0].color = Color::Red;
		eva[i*4+1].color = Color::Red;
		eva[i*4+2].color = Color::Red;
		eva[i*4+3].color = Color::Red;*/

		
	}

	

	allNodes = new NodeProjectile*[numProtrusions];
	for( int i = 0; i < numProtrusions; ++i )
	{
		allNodes[i] = new NodeProjectile( this, i );
	}

	ResetEnemy();
	//ResetNodes();

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = halfNode;
	hurtBody.rh = halfNode;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = halfNode;
	hitBody.rh = halfNode;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;


	spawnRect = sf::Rect<double>( pos.x - maxProtLength, pos.y - maxProtLength, 
		maxProtLength * 2, maxProtLength * 2 );

	for( int i = 0; i < numProtrusions; ++i )
	{
		CollisionBox &edgeBox = edgeHitboxes[i];
		V2d &origin  = origins[i];
		edgeBox.type = CollisionBox::Hit;
		edgeBox.isCircle = false;
		
		V2d n = normalize( origin - position );
		edgeBox.globalAngle = atan2( n.y, n.x );
		
		//cout << "origin start: " << origins[i].x << ", " << origins[i].y << endl;
		V2d midPoint = ( origins[i] + position ) / 2.0;
		//cout << "position: " << midPoint.x << ", " << midPoint.y << "angle: " << edgeBox.globalAngle << endl;
		edgeBox.globalPosition = midPoint;
		edgeBox.rw = length( origin - position ) / 2.0;
		//width
		edgeBox.rh = edgeWidth;
	}

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, 
		sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	
	//dynamicMode = false;
	//dynamicFrame = 0;
	
	/*dead = false;
	dying = false;
	deathFrame = 0;
	health = initHealth;*/
}

SecurityWeb::~SecurityWeb()
{
	delete []armLength;
	delete armVA;
}

void SecurityWeb::ResetNodes()
{
	activeNodes = allNodes[numProtrusions-1];

	activeNodes->Reset( origins[numProtrusions-1] );
	//activeNodes->
	activeNodes->nextProj = NULL;
	for( int i = numProtrusions-2; i >= 0; --i )
	{
		allNodes[i]->nextProj = activeNodes;
		activeNodes = allNodes[i];
		activeNodes->Reset( origins[i] );
	}
}

void SecurityWeb::HandleRayCollision( Edge *edge, double equant, double rayPortion )
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

void SecurityWeb::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void SecurityWeb::ResetEnemy()
{
	dead = false;
	dying = false;
	deathFrame = 0;
	frame = 0;
	receivedHit = NULL;
	dynamicMode = false;
	dynamicFrame = 0;
	//NodeProjectile *currNode = allNodes;
	ResetNodes();


	//UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void SecurityWeb::DirectKill()
{
	dying = true;
	health = 0;
	receivedHit = NULL;
}

void SecurityWeb::ActionEnded()
{
	Actor *player = owner->player;
	double len = length( player->position - position );
	//if( frame == actionLength[action] )
	//{
	//	switch( action )
	//	{
	//	case REST:
	//		break;
	//	case GUARD:
	//		action = REST;
	//		break;
	//	case FIRE:
	//		if( length( player->position - position ) >= dist; )
	//		{
	//			action = REST;
	//			frame = 0;
	//		}
	//		//action = FIRE;
	//		break;
	//	}
	//	//if( action == FIRE )
	//	//{
	//	//	if( len > shotRadius )
	//	//	{
	//	//		action = REST;
	//	//	}
	//	//	else if( len > chaseRadius )
	//	//	{
	//	//		action = CHASE;
	//	//		velocity = normalize( player->position - position ) * 2.5;
	//	//	}
	//	//	else if( len < retreatRadius )
	//	//	{
	//	//		action = RETREAT;
	//	//		velocity = normalize( player->position - position ) * -2.5;
	//	//	}
	//	//	else
	//	//	{
	//	//		action = NEUTRAL;
	//	//	}	
	//	//}
	//	//else if( action == REST )
	//	//{
	//	//	if( len > shotRadius )
	//	//	{
	//	//		//stay the same
	//	//	}
	//	//	else if( len > chaseRadius )
	//	//	{
	//	//		action = CHASE;
	//	//		velocity = normalize( player->position - position ) * 2.5;
	//	//	}
	//	//	else if( len < retreatRadius )
	//	//	{
	//	//		action = RETREAT;
	//	//		velocity = normalize( player->position - position ) * -2.5;
	//	//	}
	//	//	else
	//	//	{
	//	//		action = NEUTRAL;
	//	//		velocity = V2d( 0, 0 );
	//	//	}	
	//	//}
	//	//else
	//	//{
	//	//	action = FIRE;
	//	//	velocity = V2d( 0, 0 );
	//	//}
	//	frame = 0;
	//}
}


void SecurityWeb::UpdatePrePhysics()
{

	Actor *player = owner->player;

	if( !dead && !dying )
	{
		ActionEnded();
	}
	


	if( dynamicMode )
	{
		if( dynamicFrame == targetFrames )
		{
			NodeProjectile *curr = activeNodes;
			while( curr != NULL )
			{
				
				//cout << "pre: " << curr << endl;
				curr->velocity = normalize( player->position - position ) * 10.0;
				curr = curr->nextProj;
			}
		}
		else if( dynamicFrame == 300 )
		{
			NodeProjectile *curr = activeNodes;
			while( curr != NULL )
			{
				dynamicMode = false;
				ResetNodes();
				//cout << "pre: " << curr << endl;
				//curr->velocity = normalize( player->position - position ) * 10.0;
				curr = curr->nextProj;
			}
		}
		
	}

	
	if( dynamicMode )
	{
		NodeProjectile *curr = activeNodes;
		while( curr != NULL )
		{
			//cout << "pre: " << curr << endl;
			curr->UpdatePrePhysics();
			curr = curr->nextProj;
		}
	}
	//Actor *player = owner->player;

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

			owner->player->ConfirmEnemyKill( this );
			//cout << "dying" << endl;
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}
}

void SecurityWeb::UpdatePhysics()
{	
	if( dynamicMode )
	{
		NodeProjectile *curr = activeNodes;
		while( curr != NULL )
		{
			//cout << "phys: " << curr << endl;
			curr->UpdatePhysics();
			curr = curr->nextProj;
		}
	}

	specterProtected = false;
	if( !dead && !dying )
	{
		

		//position += velocity / NUM_STEPS / (double)slowMultiple;

		PhysicsResponse();
	}

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

	return;
}

void SecurityWeb::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_ORANGE, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 6, 3, true );
			
		//	cout << "SecurityWeb received damage of: " << receivedHit->damage << endl;
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
		//	cout << "SecurityWeb just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		if( !dynamicMode )
		{
			for( int i = 0; i < numProtrusions;++i )//numProtrusions; ++i )//numProtrusions; ++i )
			{
				if( edgeHitboxes[i].Intersects( owner->player->hurtBody ) )
				{
					dynamicMode = true;
					dynamicFrame = 0;
					//cout << "gotcha: " << i << endl;
					break;
				}
			}
		}
		else
		{

		}
	}
}

void SecurityWeb::UpdatePostPhysics()
{
	NodeProjectile *curr = activeNodes;
	while( curr != NULL )
	{
		curr->UpdatePostPhysics();
		curr = curr->nextProj;
	}

	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
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

	if( dead && activeNodes == NULL )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	//launcher->UpdateSprites();

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		if( dynamicMode )
			dynamicFrame++;
		slowCounter = 1;
	
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
}

void SecurityWeb::UpdateSprite()
{
	if( !dying && !dead )
	{
		/*sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );*/

		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );

		}
	}
	if( dying )
	{

		/*botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 1 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );*/
	}
}

void SecurityWeb::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		target->draw( *armVA, ts->texture );
		//target->draw( *edges, ts->texture );
		
		if( hasMonitor && !suppressMonitor )
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite, keyShader );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			
		}
	}
	else if( !dead )
	{
		//target->draw( botDeathSprite );

		//if( deathFrame / 3 < 6 )
		//{
		//	
		//	/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
		//	bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
		//	bloodSprite.setPosition( position.x, position.y );
		//	bloodSprite.setScale( 2, 2 );
		//	target->draw( bloodSprite );*/
		//}
		//
		//target->draw( topDeathSprite );
	}

	if( !dead )
	{
		target->draw( *nodes, ts->texture );
	}


}

void SecurityWeb::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
	{
		if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else
		{
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}
}

bool SecurityWeb::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void SecurityWeb::UpdateHitboxes()
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
pair<bool,bool> SecurityWeb::PlayerHitMe()
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

bool SecurityWeb::PlayerSlowingMe()
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

void SecurityWeb::DebugDraw( RenderTarget *target )
{
	if( !dying && !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );


		for( int i = 0; i < numProtrusions; ++i )//i < numProtrusions; ++i )
		{
			edgeHitboxes[i].DebugDraw( target );
		}
	}

	if( dynamicMode )
	{
		NodeProjectile *curr = activeNodes;
		while( curr != NULL )
		{
			curr->hitBody.DebugDraw( target );
			curr = curr->nextProj;
		}
	}

	

}

void SecurityWeb::SaveEnemyState()
{
	//stored.dead = dead;
	//stored.deathFrame = deathFrame;
	//stored.frame = frame;
	//stored.hitlagFrames = hitlagFrames;
	//stored.hitstunFrames = hitstunFrames;
	//stored.position = position;
}

void SecurityWeb::LoadEnemyState()
{
	//dead = stored.dead;
	//deathFrame = stored.deathFrame;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
	//position = stored.position;
}

SecurityWeb::NodeProjectile::NodeProjectile( SecurityWeb *p_parent,
	int p_vaIndex )
	:Movable(), parent( p_parent ), vaIndex( p_vaIndex )
{
	framesToLive = 1;
	velocity = V2d( 0, 0 );

	int nodeSize = 128;
	int halfNode = 64;

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = halfNode;
	hurtBody.rh = halfNode;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = halfNode;
	hitBody.rh = halfNode;

	physBody.type = CollisionBox::Physics;
	physBody.isCircle = true;
	physBody.globalAngle = 0;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = halfNode;
	physBody.rh = halfNode;

	

	//Reset( parent->position );
	//V2d n = normalize( position - 
	
}

void SecurityWeb::NodeProjectile::Reset( sf::Vector2<double> &pos )
{
	active = false;
	frame = 0;
	Movable::Reset( pos );

	
	SetNode( 6 );
	

	hurtBody.globalPosition = pos;
	hitBody.globalPosition = pos;
	physBody.globalPosition = pos;

	
}
void SecurityWeb::NodeProjectile::UpdatePrePhysics()
{
	Movable::UpdatePrePhysics();
	if( framesToLive == 0 )
	{
		active = false;

		/*V2d diff = parent->arenaCenter - revNode->position;
		parent->launcher->facingDir = normalize( -diff );
		parent->launcher->position = parent->arenaCenter + diff;
		parent->launcher->Fire();*/
	}
}

void SecurityWeb::NodeProjectile::SetNode( int index )
{
	Vector2f fPos( position.x, position.y );
	int nodeSize = 128;
	int halfNode = nodeSize / 2;

	VertexArray &nva = *parent->nodes;

	V2d dir = normalize( parent->position - position );
	double angle = atan2( dir.x, -dir.y );
	Transform rot;
	rot.rotate( angle / PI * 180.0 );

	int size = 64;
	int halfSize = 32;

	nva[vaIndex*4+0].position = fPos + rot.transformPoint( Vector2f( -halfSize, -halfSize ) );
	nva[vaIndex*4+1].position = fPos + rot.transformPoint( Vector2f( halfSize, -halfSize ) );
	nva[vaIndex*4+2].position = fPos + rot.transformPoint( Vector2f( halfSize, halfSize ) );
	nva[vaIndex*4+3].position = fPos + rot.transformPoint( Vector2f( -halfSize, halfSize ) );
	/*nva[vaIndex*4+0].color = Color::Green;
	nva[vaIndex*4+1].color = Color::Green;
	nva[vaIndex*4+2].color = Color::Green;
	nva[vaIndex*4+3].color = Color::Green;*/

	IntRect sub = parent->ts->GetSubRect( index );
	nva[vaIndex*4+0].texCoords = Vector2f( sub.left, sub.top );
	nva[vaIndex*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );	
	nva[vaIndex*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
	nva[vaIndex*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );
}

void SecurityWeb::NodeProjectile::UpdatePostPhysics()
{
	//IntRect sub = parent->ts->GetSubRect( frame % 4 );

	SetNode( 6 );

	Movable::UpdatePostPhysics();

	//sprite.setPosition( position.x, position.y );
}

void SecurityWeb::NodeProjectile::HitPlayer()
{
	if( parent->dynamicMode && parent->dynamicFrame >= parent->targetFrames )
	{
		Actor *player = parent->owner->player;
	//probably deactivate the bullet during this also
		player->ApplyHit( parent->hitboxInfo );
	}
}

void SecurityWeb::NodeProjectile::IncrementFrame()
{
	++frame;
	//--framesToLive;
}
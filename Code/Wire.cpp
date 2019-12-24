#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>
#include <assert.h>
#include "Enemy.h"
#include "Grass.h"

using namespace sf;
using namespace std;


Wire::Wire( Actor *p, bool r)
	:state( IDLE ), numPoints( 0 ), framesFiring( 0 ), fireRate( 200/*120*/ ), maxTotalLength( 10000 ), maxFireLength( 5000 ), minSegmentLength( 128 )//50 )
	, player( p ), hitStallFrames( 10 ), hitStallCounter( 0 ), right( r )
	, extraBuffer( MAX_POINTS ),//64  ), 
	//eventually you can split this up into smaller sections so that they don't all need to draw
  quadHalfWidth( 8 ), ts_wire( NULL ), frame( 0 ), animFactor( 1 ), offset( 8, 18 ),
  numTotalCharges( 0 ), chargeVA( sf::Quads, MAX_CHARGES * 4 )
{
	numQuadVertices = (int)((ceil( maxTotalLength / 8.0 ) + extraBuffer) * 4 );
	quads = new Vertex[numQuadVertices];

	numMinimapQuads = (int)((ceil( maxTotalLength / 8.0 ) + extraBuffer) * 4 );
	minimapQuads = new Vertex[numMinimapQuads];

	aimingPrimaryAngleRange = 2;
	hitEnemyFramesTotal = 5;

	int tipIndex = 0;
	ts_wire = player->owner->GetTileset( "Kin/wires_16x16.png", 16, 16 );
	if( r )
	{
		ts_miniHit = player->owner->GetTileset( "Env/rain_64x64.png", 64, 64 );
		tipIndex = 0;
	}
	else
	{
		tipIndex = 2;
		ts_miniHit = player->owner->GetTileset( "Env/rain_64x64.png", 64, 64 );
	}

	ts_wireTip = player->owner->GetTileset( "Kin/wire_tips_16x16.png", 16, 16 );

	grassCheckRadius = 20;

	wireTip.setTexture( *ts_wireTip->texture );
	wireTip.setTextureRect( ts_wireTip->GetSubRect( tipIndex ) );
	wireTip.setOrigin( wireTip.getLocalBounds().width / 2, wireTip.getLocalBounds().height / 2 );


	ts_wireCharge = player->owner->GetTileset( "Kin/wirecharge_32x32.png", 32, 32 );


	tipHitboxInfo = new HitboxInfo();
	tipHitboxInfo->damage = 20;
	tipHitboxInfo->drainX = .5;
	tipHitboxInfo->drainY = .5;
	tipHitboxInfo->hitlagFrames = 0;
	tipHitboxInfo->hitstunFrames = 30;
	tipHitboxInfo->knockback = 0;
	tipHitboxInfo->freezeDuringStun = true;
	if (r)
	{
		tipHitboxInfo->hType = HitboxInfo::WIREHITRED;
	}
	else
	{
		tipHitboxInfo->hType = HitboxInfo::WIREHITBLUE;
	}
	

	minSideEdge = NULL;
	minSideOther = -1;
	minSideAlong = -1;

	triggerDown = false;
	prevTriggerDown = false;

	retractSpeed = 30;//20;//60;

	numAnimFrames = 16;

	//pullStrength = 10;
	maxPullStrength = 10;
	startPullStrength = 10;
	pullStrength = startPullStrength;
	pullAccel = (maxPullStrength - startPullStrength) / 180;
	//.1 = 10 frames per 1. 100 frames per 10

	maxDragStrength = 30;
	startDragStrength = 10;
	dragStrength = startDragStrength;
	dragAccel = (maxDragStrength - startDragStrength) / 180.0;
	//numTotalCharges = 0;

	activeChargeList = NULL;
	inactiveChargeList = NULL;
	//charges start at the earliest retraction points so they hit the most targets.
	for( int i = 0; i < MAX_CHARGES; ++i )
	{
		CreateWireCharge();
	}
	
	//offsetFlagged = false;
	//lockEdge = NULL;
}

Wire::~Wire()
{
	delete[] quads;
	delete[] minimapQuads;

	delete tipHitboxInfo;

	ClearCharges();
	DestroyDeactivatedCharges();
}

void Wire::ActivateCharges()
{
	for( int i = 0; i < numPoints; ++i )
	{
		ActivateWireCharge( i );
	}
}

int Wire::CountActiveCharges()
{
	int counter = 0;
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		++counter;
		curr = curr->next;
	}
	return counter;
}

int Wire::CountInactiveCharges()
{
	int counter = 0;
	WireCharge *curr = inactiveChargeList;
	while( curr != NULL )
	{
		++counter;
		curr = curr->next;
	}
	return counter;
}

sf::Vector2<double> Wire::GetPlayerPos()
{
	currOffset = V2d( 0, 0 );//GetOriginPos( true );
	if( false )//offset != currOffset )
	{
		//offsetFlagged = true;
		//currOffset = offset;
		//UpdateAnchors( V2d(0, 0 ) );
		return player->position + currOffset;
	}
	else
	{
		
		return player->position + currOffset;
	}
}

void Wire::UpdateState( bool touchEdgeWithWire )
{
	
	//cout << "update state" << endl;
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;

	//V2d playerPos = player->position;
	
	V2d playerPos;
	if( state == RETRACTING )
	{
		playerPos = retractPlayerPos;
	}
	else
	{
		playerPos = GetPlayerPos();//GetOriginPos(true);
	}
	storedPlayerPos = playerPos;

	if( right )
	{
		triggerDown = currInput.RightTriggerPressed();//currInput.rightTrigger >= triggerThresh;
		prevTriggerDown = prevInput.RightTriggerPressed();//prevInput.rightTrigger >= triggerThresh;
	}
	else
	{
		triggerDown = currInput.LeftTriggerPressed();//currInput.leftTrigger >= triggerThresh;
		prevTriggerDown = prevInput.LeftTriggerPressed();//prevInput.leftTrigger >= triggerThresh;
	}

	if( state == PULLING )
	{ 
		if (player->ground != NULL || player->bounceEdge != NULL || player->grindEdge != NULL )
		{
			state = HIT;
		}
	}


	switch( state )
	{
	case IDLE:
		{
			TryFire();
			break;
		}
	case FIRING:
		{
			//rcEdge = NULL;
		//	RayCast( this, player->owner->terrainTree->startNode, anchor.pos, player->position );
			if( rcEdge != NULL )
			{
				CheckAntiWireGrass();

				if (antiWireGrassCount == 0)
				{
					state = HIT;
					if (!triggerDown)
					{
						canRetractGround = true;
					}
					else
					{
						canRetractGround = false;
					}
					hitStallCounter = framesFiring;

				}
				else
				{
					Reset();
				}
			}

			if( framesFiring * fireRate > maxFireLength )
			{
				Reset();
				
			}
			break;
		}
	case HIT:
		{
			totalLength = GetCurrentTotalLength();

			if( totalLength > maxTotalLength )
			{
				state = RELEASED;
				numPoints = 0;
				break;
			}
			else
			{
				if( canRetractGround && !triggerDown && prevTriggerDown )
				{
					Retract();
					
					break;
				}
				else
				{
					if( !triggerDown )
					{
						canRetractGround = true;
					}
				}
			}

			bool a = player->ground == NULL;
			bool b = !touchEdgeWithWire;
			bool c = hitStallCounter >= hitStallFrames;
			bool d = triggerDown;
			bool e = player->oldAction != Actor::WALLCLING && player->oldAction != Actor::WALLATTACK && player->action != Actor::WALLATTACK;
			bool f = ( !player->bounceFlameOn || player->framesSinceBounce > 8 || player->oldBounceEdge == NULL ) && player->bounceEdge == NULL;
			
			if( a && b && c && d && e && f )
			{
				//cout << "playeraction: " << player->action << endl;
				//cout << "set state pulling" << endl;
				state = PULLING;
				pullStrength = startPullStrength;
				dragStrength = startDragStrength;
				if( right )
				{
					player->framesSinceRightWireBoost = 0;
				}
				else
				{
					player->framesSinceLeftWireBoost = 0;
				}
			}
			else
			{
				//cout << "a: " << a << ", b: " << b << ", c: " << c << ", d: " << d
				//	<< ", e: " << e << ",f: " << f << endl;
				//cout << "bounce edge: " << player->bounceEdge << ", " << player->framesSinceBounce << ", old: " <<
				//	player->oldBounceEdge << endl;
			}
			break;
		}
	case PULLING:
		{
			totalLength = GetCurrentTotalLength();

			if( totalLength > maxTotalLength )
			{
				state = RELEASED;
				numPoints = 0;
			}
			else
			{
				

				if( !triggerDown && player->ground == NULL )
				{
					state = RETRACTING;
					ActivateCharges();
					retractPlayerPos = playerPos;
					fusePointIndex = numPoints;
					if( numPoints == 0 )
					{
						fuseQuantity = length( anchor.pos - retractPlayerPos );
					}
					else
					{
						if( right )
						{
							fuseQuantity = length( retractPlayerPos - points[numPoints-1].pos );
						}
						else
						{
							fuseQuantity = length( anchor.pos - points[0].pos );
						}
					}
				}
				if( triggerDown && ( touchEdgeWithWire || player->action == Actor::WALLCLING ) )
				{
					state = HIT;
					if( !triggerDown )
					{
						canRetractGround = true;
					}
					else
					{
						canRetractGround = false;
					}
				}
			}
			break;
		}
	case RETRACTING:
		{
			TryFire();
			break;
		}
	case RELEASED:
		{
			Reset();
			break;
		}
	case HITENEMY:
	{
		if (TryFire())
		{
			break;
		}

		if (hitEnemyFrame == hitEnemyFramesTotal)
		{
			Reset();
			//Retract();
		}
		break;
	}
	}

	switch( state )
	{
	case IDLE:
		{
			//no updates
			break;
		}
	case FIRING:
		{
			rcEdge = NULL;
			//rcQuantity = 0;
			
			rayCancel = false;
			V2d currPos = playerPos + fireDir * fireRate * (double)(framesFiring);
			V2d futurePos = playerPos + fireDir * fireRate * (double)(framesFiring + 1);
			RayCast( this, player->owner->terrainTree->startNode, playerPos, futurePos );
			RayCast(this, player->owner->railEdgeTree->startNode, playerPos, futurePos);

			fireDir = normalize(fireDir);
			double len = length(futurePos - currPos);

			movingHitbox.SetRectDir(fireDir, len, 30);
			movingHitbox.globalPosition = (currPos + futurePos) / 2.0;

			tipHitboxInfo->hDir = fireDir;

			if( rayCancel )
			{
				Reset();
			}

			//cout << "framesFiring " << framesFiring << endl;

			if( rcEdge != NULL )
			{
				CheckAntiWireGrass();

				if (antiWireGrassCount > 0)
				{
					Reset();
					break;
				}

				//cout << "hit edge!: " << rcEdge->Normal().x << ", " << rcEdge->Normal().y << ", : " << rcEdge << endl;
				if( rcQuant < 4 )
				{
					//cout << "Aw" << endl;
					//cout << "lock1" << endl;
					anchor.pos = rcEdge->v0;
				}
				else if( rcQuant > length( rcEdge->v1 - rcEdge->v0 ) - 4 )
				{
					//cout << "Bw" << endl;
					//cout << "lock2" << endl;
					anchor.pos = rcEdge->v1;
				}
				else
				{
					//cout << "Cw" << endl;
					anchor.pos = rcEdge->GetPoint( rcQuant );
				}
				
				anchor.e = rcEdge;
				anchor.quantity = rcQuant;

				//cout << "anchor pos: " << anchor.pos.x << ", " << anchor.pos.y << endl;
				//player->owner->ActivateEffect( ts_miniHit, rcEdge->GetPoint( rcQuant ), true, 0, , 3, facingRight );

				numPoints = 0;

				state = HIT;

				if( !triggerDown )
				{
					canRetractGround = true;
				}
				else
				{
					canRetractGround = false;
				}
				hitStallCounter = framesFiring;

				storedPlayerPos = playerPos;
				//storedPlayerPos = playerPos;
				//state = HIT;
				//if( !triggerDown )
				//{
				//	canRetractGround = true;
				//}
				//else
				//{
				////	canRetractGround = false;
				//}
				//numPoints = 0;
				//anchor.pos = minSideEdge->v0;
				//anchor.quantity = 0;
				//anchor.e = minSideEdge;
				//UpdateAnchors( V2d( 0, 0 ) );

				//UpdateAnchors( V2d( 0, 0 ) );
			}
			break;
		}
	case HIT:
		{


			if( hitStallCounter < hitStallFrames )
				hitStallCounter++;
			break;
		}
	case PULLING:
		{
			//cout << "pulling!" << endl;
			totalLength = GetCurrentTotalLength();

			V2d wn;
			segmentLength = GetSegmentLength();

			if( numPoints == 0 )
			{
				//segmentLength = totalLength;
				wn = normalize( anchor.pos - playerPos );
				//cout << "A segmentLength: " << segmentLength << endl;
			}
			else
			{
				//double segmentLength = length( points[numPoints-1].pos - playerPos );
				wn = normalize( points[numPoints-1].pos - playerPos );
				//cout << "B segmentLength: " << segmentLength << endl;
				//cout << "segment length multi: " << segmentLength << endl;
			}
			
			bool shrinkInput = false;

			
			if( wn.y <= 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LUp();
			}
			else if( wn.y > 0 )
			{
				if( wn.x < 0 )
				{
					shrinkInput = currInput.LLeft();
				}
				else if( wn.x > 0 )
				{
					shrinkInput = currInput.LRight();
				}

				shrinkInput |= currInput.LDown();
			}

			shrinkInput = false;

			if( currInput.A )
			{
				shrinkInput = true;
				dragStrength = startDragStrength;
			}
			else if( currInput.B )
			{
				if( triggerDown && player->ground == NULL )
				{
					segmentLength += dragStrength;
					totalLength += dragStrength;

					dragStrength += dragAccel;
					if( dragStrength > maxDragStrength )
						dragStrength = maxDragStrength;
					
					//cout << "GROWING" << endl;
				}
			}
			else
			{
				dragStrength = startDragStrength;
			}

			bool bounceWindow = (player->action == Actor::BOUNCEAIR && player->framesSinceBounce > 10)
				|| player->action != Actor::BOUNCEAIR;
			bool c = totalLength > 128;//minSegmentLength > 128;//
			if( shrinkInput && triggerDown && player->ground == NULL && c && bounceWindow )
			{
				//cout << "SHRINKING " << endl;
				double segmentChange = pullStrength;
				double minSeg = 100;
				double maxSeg = 1000;

				if( segmentLength < minSeg )
				{
					segmentChange = pullStrength * .1;
				}
				else if( segmentLength > maxSeg )
				{
					segmentChange = pullStrength * 1.0;
				}
				else
				{
					segmentChange = pullStrength * min((segmentLength-minSeg) / (maxSeg - minSeg) + .1, 1.0);
				}
				//min( max( segmentLength / 1000.0, 1.0 ), 1.0;

				if( segmentLength - segmentChange < minSegmentLength )
					segmentChange = 0;//-(minSegmentLength - (segmentLength - pullStrength));

				totalLength -= segmentChange;
				segmentLength -= segmentChange;

				/*if( segmentChange > 0 )
				{
					pullStrength += pullAccel;
					if( pullStrength > maxPullStrength )
						pullStrength = maxPullStrength;
				}
				else
				{
					pullStrength = startPullStrength;
				}*/
			}
			else
			{
				pullStrength = startPullStrength;
			}
			break;
		}
	case RETRACTING:
		{
			UpdateFuse();
			break;
		}
	case RELEASED:
		{
			break;
		}
	case HITENEMY:
	{
		++hitEnemyFrame;
		break;
	}
	}

	++frame;
	if( frame / animFactor == numAnimFrames )
	{
		frame = 0;
	}

	
}

void Wire::ClearCharges()
{
	//cout << "about to clear active: " << CountActiveCharges() << ", inact: " << CountInactiveCharges() << endl;
	//cout << "clearing charges" << endl;
	WireCharge *curr = activeChargeList;
	WireCharge *temp;
	while( curr != NULL )
	{
		temp = curr->next;
		curr->ClearSprite();
		DeactivateWireCharge( curr );
		curr = temp;
	}
	activeChargeList = NULL;
}


bool Wire::TryFire()
{
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;
	if (player->CanShootWire() && triggerDown && !prevTriggerDown)
	{
		//cout << "firing" << endl;
		fireDir = V2d(0, 0);

		if (false)
		{
			if (currInput.LLeft())
			{
				fireDir.x -= 1;
			}
			else if (currInput.LRight())
			{
				fireDir.x += 1;
			}

			if (currInput.LUp())
			{
				if (player->reversed)
				{
					fireDir.y += 1;
				}
				else
				{
					fireDir.y -= 1;
				}

			}
			else if (currInput.LDown())
			{
				if (player->reversed)
				{
					fireDir.y -= 1;
				}
				else
				{
					fireDir.y += 1;
				}

			}
		}
		else
		{
			if (currInput.leftStickMagnitude > 0)
			{
				double angle = currInput.leftStickRadians;

				double degs = angle / PI * 180.0;
				double sec = 360.0 / 64.0;
				int mult = floor((degs / sec) + .5);

				if (mult < 0)
				{
					mult += 64;
				}

				int test;
				int bigger, smaller;
				for (int i = 0; i < aimingPrimaryAngleRange; ++i)
				{
					test = i + 1;
					for (int j = 0; j < 64; j += 16)
					{
						bigger = mult + test;
						smaller = mult - test;
						if (smaller < 0)
							smaller += 64;
						if (bigger >= 64)
							bigger -= 64;

						if (bigger == j || smaller == j)
						{
							mult = j;
						}
					}
				}

				angle = (PI / 32.0) * mult;

				//cout << "mult: " << mult << endl;

				fireDir.x = cos(angle);
				fireDir.y = -sin(angle);
			}
			else
			{
				if (player->facingRight)
				{
					fireDir = V2d(1, -1);
				}
				else
				{
					fireDir = V2d(-1, -1);
				}

				if (player->reversed)
				{
					fireDir.y = 1.0;
				}
			}

		}


		if (right)
		{
			player->lastWire = 1;
		}
		else
		{
			player->lastWire = 2;
		}

		fireDir = normalize(fireDir);

		float angle = atan2(fireDir.y, fireDir.x);

		state = FIRING;
		//cout << "firing from idle" << endl;
		framesFiring = 0;
		frame = 0;

		if (anchor.enemy != NULL)
		{
			anchor.enemy->HandleWireUnanchored(this);
		}
		anchor.enemy = NULL;

		wireTip.setRotation(angle);

		return true;
	}

	return false;
}

void Wire::SwapPoints( int aIndex, int bIndex )
{
	WirePoint temp = points[aIndex];
	points[aIndex] = points[bIndex];
	points[bIndex] = temp;
}

void Wire::UpdateEnemyAnchor()
{
	if (anchor.enemy != NULL)
	{
		V2d oldPos = anchor.pos;
		anchor.pos = anchor.enemy->GetCamPoint(anchor.enemyPosIndex);
		realAnchor = anchor.pos;
		anchorVel = realAnchor - oldPos;

	}
}

CollisionBox * Wire::GetTipHitbox()
{
	if (state == FIRING)
	{
		return &movingHitbox;
	}
	else
	{
		return NULL;
	}
}

void Wire::UpdateAnchors2( V2d vel )
{
	
	//assert( vel.x != 0 || vel.y != 0 );

	wireTestClock.restart();
	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if( (state == HIT || state == PULLING) && anchor.enemy == NULL )
	{
		//UpdateEnemyAnchor();


		if( oldPos.x == storedPlayerPos.x && oldPos.y == storedPlayerPos.y )
		{
			//return;
		}
		
		oldPos = storedPlayerPos;

		double radius = length( realAnchor - playerPos ); //new position after moving

		if( numPoints == 0 )
		{
			//line->append( sf::Vertex(sf::Vector2f(anchor.pos.x, anchor.pos.y), Color::Black) );
			realAnchor = anchor.pos;
		}
		else
		{
			//line->append( sf::Vertex(sf::Vector2f(points[numPoints - 1].pos.x, points[numPoints - 1].pos.y), Color::Black) );
			realAnchor = points[numPoints-1].pos;
		}

		if( vel.x == 0 && vel.y == 0 )
		{
			//cout << "skipping" << endl;
		//	return;
		}

		int counter = 0;
		V2d a;
		V2d b;
		double len;
		V2d oldDir;
		V2d dir;
		double left, right, top, bottom;
		double ex = 1;
		Rect<double> r;

		

		if( counter > 1 )
		{
			//cout << "COUNTER: " << counter << endl;
		}

		a = realAnchor - oldPos;
		b = realAnchor - playerPos;
		len = max( length( a ), length( b ) );

		oldDir = oldPos - realAnchor;
		dir = playerPos - realAnchor;

		left = min( min( realAnchor.x, oldPos.x ), playerPos.x );
		top = min( min( realAnchor.y, oldPos.y ), playerPos.y );
		right = max( max( realAnchor.x, oldPos.x ), playerPos.x );
		bottom = max( max( realAnchor.y, oldPos.y ), playerPos.y );

		r.left = left - ex;
		r.top = top - ex;
		r.width = (right - left) + ex * 2;
		r.height = (bottom - top) + ex * 2;
			
		foundPoint = false;
		newWirePoints = 0; //number of points added

		V2d normalizedA = normalize( a );
		V2d otherA( normalizedA.y, -normalizedA.x );
		if( dot( normalize( playerPos - oldPos ), otherA ) > 0 )
			clockwise = true;
		else
			clockwise = false;

		queryMode = "terrain";
		player->owner->terrainTree->Query( this, r );
		if( state == RELEASED )
		{
			cout << "went too many points" << endl;
			//should cut the wire when you go over the point count
			return;
		}

		if( player->owner->showDebugDraw )
		{
			sf::RectangleShape queryDebug;
			queryDebug.setPosition( Vector2f( r.left, r.top ) );
			queryDebug.setSize( Vector2f( r.width, r.height ) );
			queryDebug.setFillColor( Color::Transparent );//Color( 255, 255, 255, 100 ) );
			queryDebug.setOutlineColor( Color::White );
			queryDebug.setOutlineThickness( 1 );
			progressDraw.push_back( new sf::RectangleShape( queryDebug ) );
		}

		SortNewPoints();

		//remove points as need be
		for( int i = numPoints - 1; i >= 0; --i )
		{ 
			double result = cross( playerPos - points[numPoints-1].pos, points[i].test );
			if( result > 0 )
			{
				//V2d along = 
				//cout << "removing along: " << 
				//cout << "removePoint: " << points[numPoints-1].pos.x << ", " << points[numPoints-1].pos.y << endl;
				numPoints--;
			}
			else
			{
				break;
			}
		}
	}
	else if( state == FIRING )
	{
		oldPos = storedPlayerPos;
		V2d wireVec = fireDir * fireRate * (double)(framesFiring + 1 );
		
		V2d diff = playerPos - oldPos;
		
		V2d wirePos = playerPos + wireVec; 
		V2d oldWirePos = oldPos + wireVec;

		quadOldPosA = oldPos;
		quadOldWirePosB = oldWirePos;
		quadWirePosC = wirePos;
		quadPlayerPosD = playerPos;

		Enemy *foundEnemy = NULL;
		int foundIndex;
		if (GetClosestEnemyPos(player->owner, wirePos, 128, foundEnemy, foundIndex))
		{
			storedPlayerPos = playerPos;
			state = HIT;
			if (!triggerDown)
			{
				canRetractGround = true;
			}
			else
			{
				canRetractGround = false;
			}
			numPoints = 0;
			anchor.pos = foundEnemy->GetCamPoint(foundIndex); //minSideEdge->v0;
			anchor.quantity = 0;
			anchor.e = NULL;//minSideEdge;

			anchor.enemy = foundEnemy;
			anchorVel = V2d(0, 0);
			anchor.enemyPosIndex = foundIndex;
			UpdateAnchors(V2d(0, 0));

			foundEnemy->HandleWireAnchored(this);
		}

		//for grabbing onto points
		double top = min( quadOldPosA.y, min( quadOldWirePosB.y, min( quadWirePosC.y, quadPlayerPosD.y ) ) );
		double bot = max( quadOldPosA.y, max( quadOldWirePosB.y, max( quadWirePosC.y, quadPlayerPosD.y ) ) );
		double left = min( quadOldPosA.x, min( quadOldWirePosB.x, min( quadWirePosC.x, quadPlayerPosD.x ) ) );
		double right = max( quadOldPosA.x, max( quadOldWirePosB.x, max( quadWirePosC.x, quadPlayerPosD.x ) ) );

		double ex = 1;
		sf::Rect<double> r( left - ex, top - ex, (right - left) + ex * 2, ( bot - top ) + ex * 2 );

		if( r.width == 0 || r.height == 0 )
		{

		}
		else
		{
			
			//queryType = "terrain";
			minSideEdge = NULL;
			queryMode = "terrain";
			player->owner->terrainTree->Query( this, r );
			player->owner->railEdgeTree->Query(this, r);
			if( minSideEdge != NULL )
			{
				storedPlayerPos = playerPos;
				state = HIT;
				if( !triggerDown )
				{
					canRetractGround = true;
				}
				else
				{
					canRetractGround = false;
				}
				numPoints = 0;
				anchor.pos = minSideEdge->v0;
				anchor.quantity = 0;
				anchor.e = minSideEdge;
				UpdateAnchors( V2d( 0, 0 ) );
			}
		}
	}
	
	storedPlayerPos = playerPos;

	//cout << "wire update milli: " << wireTestClock.getElapsedTime().asMilliseconds() << endl;
}

bool Wire::IsValidTrackEnemy(Enemy *e)
{
	return e->CanBeAnchoredByWire(right);
}

void Wire::UpdateAnchors( V2d vel )
{
	UpdateAnchors2( vel );
}

void Wire::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( edge->edgeType == Edge::BORDER )
	{

		rayCancel = true;
		return;
	}
	else if( edge->edgeType == Edge::CLOSED_GATE )
	{
		Gate *g = (Gate*)edge->info;
		if( g->type != Gate::BLACK && g->gState != Gate::LOCKFOREVER && g->gState != Gate::HARD )
		{
			rayCancel = true;
			return;
		}
	}
	else if( edge->edgeType == Edge::OPEN_GATE )
	{
		return;
	}
	//rayPortion > 1 &&
	//V2d playerPos = player->position;
	//playerPos += V2d( offset.x, offset.y );	
	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if( rayPortion > .1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - playerPos ) < length( rcEdge->GetPoint( rcQuant ) - playerPos ) ) )
	{
		rcEdge = edge;
		rcQuant = edgeQuantity;
	}
}

//returns -1 on errror
double Wire::GetTestPointAngle( Edge *e )
{
	V2d p = e->v0;

	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if( length( p - realAnchor ) < 1 ) //if applied to moving platforms this will need to account for rounding bugs.
	{
		return -1;
	}

	double radius = length( realAnchor - playerPos ); //new position after moving

	double anchorDist = length( realAnchor - p );

	if( anchorDist > radius )
	{
		return -1;
	}

	V2d oldVec = normalize( oldPos - realAnchor );
	V2d newVec = normalize( playerPos - realAnchor );

	V2d pVec = normalize( p - realAnchor );

	double oldAngle = atan2( oldVec.y, oldVec.x );

	double newAngle = atan2( newVec.y, newVec.x );
	

	double pAngle = atan2( pVec.y, pVec.x );
	
	double angleDiff = abs( oldAngle - pAngle );

	double maxAngleDiff = abs( newAngle - oldAngle );

	if( oldAngle < 0 )
		oldAngle += 2 * PI;
	if( newAngle < 0 )
		newAngle += 2 * PI;
	if( pAngle < 0 )
		pAngle += 2 * PI;

	bool tempClockwise = false;
	if( newAngle > oldAngle )
	{
		if( newAngle - oldAngle < PI )
		{
			tempClockwise = true;
			//cw
			double diff = pAngle - oldAngle;
			if( diff >= 0 && diff <= newAngle - oldAngle )
			{
				//cout << "a" << endl;
				return diff;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			double diff; //= oldAngle - pAngle;
			if( pAngle >= newAngle )
			{
				diff = pAngle - oldAngle;
				//cout << "b" << endl;
				return diff;
				//cw
			}
			else if( pAngle <= oldAngle )
			{
				//cout << "b1" << endl;
				diff = oldAngle - pAngle;
				return diff;
			}
			else
			{
				return -1;
			}
		}
	}
	else if( newAngle < oldAngle )
	{
		if( oldAngle - newAngle < PI )
		{
			//ccw
			double diff = oldAngle - pAngle;
			if( diff >= 0 && diff <= oldAngle - newAngle )
			{
				//cout << "c" << endl;
				return diff;
				//good
			}
			else
			{
				return -1;
			}
		}
		else
		{
			tempClockwise = true;
			double diff = pAngle - oldAngle;
			if( pAngle <= newAngle )
			{
				diff = oldAngle - pAngle;
				//cout << "d" << endl;
				return diff;
				//ccw
			}
			else if( pAngle >= oldAngle )
			{
				diff = pAngle - oldAngle;
				return diff;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
}

void Wire::TestPoint( Edge *e )
{
	V2d p = e->v0;

	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if( length( p - realAnchor ) < 1 ) //if applied to moving platforms this will need to account for rounding bugs.
	{
		return;
	}

	double radius = length( realAnchor - playerPos ); //new position after moving

	double anchorDist = length( realAnchor - p );

	if( anchorDist > radius )
	{
		return;
	}
	

	V2d oldVec = normalize( oldPos - realAnchor );
	V2d newVec = normalize( playerPos - realAnchor );

	V2d pVec = normalize( p - realAnchor );

	double oldAngle = atan2( oldVec.y, oldVec.x );

	double newAngle = atan2( newVec.y, newVec.x );
	

	double pAngle = atan2( pVec.y, pVec.x );
	
	double angleDiff = abs( oldAngle - pAngle );

	double maxAngleDiff = abs( newAngle - oldAngle );

	if( oldAngle < 0 )
		oldAngle += 2 * PI;
	if( newAngle < 0 )
		newAngle += 2 * PI;
	if( pAngle < 0 )
		pAngle += 2 * PI;

	bool tempClockwise = false;
	if( newAngle > oldAngle )
	{
		if( newAngle - oldAngle < PI )
		{
			tempClockwise = true;
			//cw
			if( pAngle - oldAngle >= 0 && pAngle - oldAngle <= newAngle - oldAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			if( pAngle >= newAngle || pAngle <= oldAngle )
			{
				//cw
			}
			else
			{
				return;
			}
		}
	}
	else if( newAngle < oldAngle )
	{
		if( oldAngle - newAngle < PI )
		{
			//ccw
			if( pAngle - newAngle >= 0 && pAngle - newAngle <= oldAngle - newAngle )
			{
				//good
			}
			else
			{
				return;
			}
		}
		else
		{
			tempClockwise = true;
			if( pAngle <= newAngle || pAngle >= oldAngle )
			{
				//ccw
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		return;
	}

	//would be more efficient to remove this calculation and only do it once per frame
	clockwise = tempClockwise;
	
	if( !foundPoint )
	{
		foundPoint = true;
		closestDiff = angleDiff;
		closestPoint = p;
		//
	}
	else
	{
		if( angleDiff < closestDiff )
		{
			closestDiff = angleDiff;
			closestPoint = p;
		}
		else if( approxEquals( angleDiff, closestDiff ) )
		{
			double closestDist = length( realAnchor - closestPoint );
			if( anchorDist < closestDist )
			{
				closestDiff = angleDiff;
				closestPoint = p;
			}
		}
		
	}
}

void Wire::TestPoint2( Edge *e )
{
	V2d p = e->v0;

	double res = GetTestPointAngle( e );
	//cout << "Res: " << res << endl;
	if( res >=0 )
	{
		//cout << "adding point at p: " << p.x << ", " << p.y << endl;
		if( numPoints < MAX_POINTS )
		{
			WirePoint &wp = points[numPoints];
			wp.pos = p;
			
			wp.sortingAngleDist = res;

			wp.test = normalize( p - realAnchor );	
			
			if( !clockwise )
			{
				points[numPoints].test = -points[numPoints].test;
			}

			//cout << "adding point with test: " << wp.test.x << ", " << wp.test.y << endl;

			numPoints++;
		}
		else
		{
			state = RELEASED;
			numPoints = 0;
		}
		newWirePoints++;
	}
}

void Wire::HandleEntrant( QuadTreeEntrant *qte )
{
	if (queryMode == "terrain")
	{
		Edge *e = (Edge*)qte;

		if (state == FIRING)
		{
			V2d along = normalize(quadOldWirePosB - quadOldPosA);
			V2d other = normalize(quadOldPosA - quadPlayerPosD);

			double alongQ = dot(e->v0 - quadOldPosA, along);
			double otherQ = cross(e->v0 - quadOldPosA, along);

			double extra = 0;
			//cout << "checking: " << e->v0.x << ", " << e->v0.y << ", along/other: " << alongQ << ", " << otherQ 
			//	<< ", alongLen: " << length( quadOldWirePosB - quadOldPosA ) << ", otherLen: " << length( quadOldPosA - quadPlayerPosD ) << endl;
			if (-otherQ >= -extra  && -otherQ <= length(quadOldPosA - quadPlayerPosD) + extra)
			{
				if (alongQ >= -extra && alongQ <= length(quadOldWirePosB - quadOldPosA) + extra)
				{
					if (minSideEdge == NULL
						|| (minSideEdge != NULL
							&& (otherQ < minSideOther
								|| (otherQ == minSideOther && alongQ < minSideAlong))))
					{
						minSideOther = otherQ;
						minSideAlong = alongQ;
						minSideEdge = e;
						//		cout << "setting to: " << e->v0.x << ", " << e->v0.y << endl;
					}
				}
			}
		}
		else
		{
			//V2d v0 = e->v0;
			//V2d v1 = e->v1;
			TestPoint2(e);
		}
	}
	else if (queryMode == "grass")
	{
		Grass *g = (Grass*)qte;
		//Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);

		V2d touchPoint = rcEdge->GetPoint(rcQuant);
		if (g->grassType == Grass::ANTIWIRE && g->IsTouchingCircle( touchPoint, grassCheckRadius ))
		{
			antiWireGrassCount++;
		}
	}
}


void Wire::CheckAntiWireGrass()
{
	if (player->owner->hasGrass[Grass::ANTIWIRE])
	{
		V2d hitPoint = rcEdge->GetPoint(rcQuant);
		sf::Rect<double> r;
		r.left = hitPoint.x - grassCheckRadius / 2;
		r.top = hitPoint.y - grassCheckRadius / 2;
		r.width = grassCheckRadius;
		r.height = grassCheckRadius;
		antiWireGrassCount = 0;
		queryMode = "grass";
		player->owner->grassTree->Query(this, r);
	}
}
//make multiples of the quads for each edge later
void Wire::UpdateQuads()
{
	V2d playerPos;
	if( state == RETRACTING )
	{
		playerPos = retractPlayerPos;
	}
	else
	{
		playerPos = GetPlayerPos();//GetOriginPos(false);
	}

	V2d alongDir;
	V2d otherDir;
	double temp;

	
	int tileHeight = 16;//6;
	int tileWidth = 16;
	int startIndex = 0;
	bool hitOrPulling = (state == HIT || state == PULLING || state == RETRACTING );
	bool singleRope = ( hitOrPulling && numPoints == 0 );
	int cap = 0;

	int currNumPoints = numPoints;

	if( state == FIRING || singleRope || (state == HIT || state == PULLING ) || state == RETRACTING || state == HITENEMY )
	{
		if( state == RETRACTING )
		{
			if( right )
			{
				currNumPoints = fusePointIndex;
				//cap++;
			}
			else
			{
				cap = numPoints - fusePointIndex;
				//currNumPoints = fusePointIndex;
			}
		}
		for( int pointI = currNumPoints; pointI >= cap; --pointI )
		{
		V2d currWirePos;
		V2d currWireStart;
		if( hitOrPulling )
		{	
			if( pointI == cap )
			{
				//cout << "cap" << endl;
				if( numPoints == 0 )
				{

					currWirePos = anchor.pos;
					if( state == RETRACTING )
					{
						if( right )
						{
							V2d start = anchor.pos;//points[pointI-1].pos;
							V2d end = retractPlayerPos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							currWirePos = retractPlayerPos;
							V2d start = retractPlayerPos;//points[pointI-1].pos;
							V2d end = anchor.pos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
					}
					else
					{
						currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
					//currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
				}
				else
				{
					currWirePos = anchor.pos;
					//currWireStart = points[0].pos;
					if( state == RETRACTING && ( (right && fusePointIndex == 0) || ( !right ) ) )
					{
						if( right )
						{
							V2d start = anchor.pos;//points[0].pos;//points[pointI-1].pos;
							V2d end = points[pointI].pos;
						
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							V2d start;
							V2d end;
							if( cap == 0 )
							{
								//cout << "cap zero" << endl;
								start = points[0].pos;
								end = anchor.pos;
							}
							else if( cap == numPoints )
							{
								//cout << "cap max" << endl;
								start = retractPlayerPos;
								end = points[cap-1].pos;
								//currWirePos = points[
							}
							else
							{
								//cout << "cap mid" << endl;
								start = points[cap].pos;;
								end = points[cap-1].pos;
							}

							V2d dir = normalize( end - start );

							currWireStart = start;

							currWirePos = start + dir * fuseQuantity;
							//currWirePos = end;
							//currWirePos = end;
							
							//currWireStart = retractPlayerPos;
							//V2d start = retractPlayerPos;
							//V2d end = points[numPoints-1].pos;
							//currWireStart = retractPlayerPos;
							//V2d start = retractPlayerPos;//anchor.pos;
							//V2d end = points[numPoints-1].pos;//points[pointI].pos;

							//V2d dir = normalize( end - start );
							//cout << "final: " << fuseQuantity;
							////currWireStart = start + dir * fuseQuantity; 
							//if( cap == numPoints )
							//{
							//	currWirePos = start + dir * fuseQuantity; 
							//}
							//else
							//{
							//	currWirePos = end;
							//}
							
						}
					}
					/*else if( state == RETRACTING && !right  )
					{

					}*/
					else
					{
						currWireStart = points[0].pos;//playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
			}
			else
			{
				//cout << "not cap" << endl;
				if( pointI == currNumPoints )
				{
					
					currWirePos = points[pointI-1].pos;
					
					if( state == RETRACTING )
					{
						currWirePos = points[0].pos;
						if( right )
						{
							V2d start = points[pointI-1].pos;
							V2d end;
							if( pointI == numPoints )
							{
								end = retractPlayerPos;
							}
							else
							{
								end = points[pointI].pos;
							}
							currWirePos = start;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * fuseQuantity; 
						}
						else
						{
							V2d start = retractPlayerPos;
							V2d end = points[numPoints-1].pos;

							V2d dir = normalize( end - start );
							currWirePos = end;
							currWireStart = start;
							//currWirePos = points[0].pos;
							////currWireStart = retractPlayerPos;
							//V2d start = currWirePos;//points[pointI-1].pos;
							//V2d end = anchor.pos;
							//
							//V2d dir = normalize( end - start );
							////if( cap == 0 )
							//{
							//	currWireStart = start + dir * fuseQuantity;
							//}
							////else
							//{
							//	//currWireStart = end;
							//}



							//if( cap == currNumPoints )
							//{
								
							//}
							//else
							//{
							//	currWireStart = end;
							//}
							
							//cout << "STARTING REVERSE: " << fuseQuantity << endl;
						}
					}
					else
					{
						currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
				else
				{
					/*if( state == RETRACTING && !right )
					{
						currWireStart = points[pointI-1].pos;
						currWirePos = points[pointI].pos;
					}
					else
					{*/
					if( false )//if( state == RETRACTING && !right )
					{
						cout << "pos ind: " << (numPoints-1)-(pointI-1) << endl;
						cout << "start ind: " << (numPoints-1)-(pointI) << endl;
						currWirePos = points[(numPoints-1)-(pointI-1)].pos;
						currWireStart = points[(numPoints-1) -(pointI)].pos;
					}
					else
					{
						currWirePos = points[pointI-1].pos;
						currWireStart = points[pointI].pos;
					}
						
					//}
					
				}
			}
			alongDir = normalize( currWirePos - currWireStart );
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
		}
		else if( state == FIRING )
		{
			alongDir = fireDir;
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
			currWirePos = playerPos + fireDir * fireRate * (double)(framesFiring+1);
			currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
		}
		else if (state == HITENEMY)
		{
			alongDir = fireDir;
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
			currWirePos = playerPos + hitEnemyDelta;//playerPos + fireDir * fireRate * (double)(framesFiring + 1);
			currWireStart = playerPos;// +V2d(player->GetWireOffset().x, player->GetWireOffset().y);
		}
		
		
		firingTakingUp = ceil( length( currWirePos - currWireStart ) / tileHeight );

		
		V2d endBack = currWirePos - otherDir * quadHalfWidth;
		V2d endFront = currWirePos + otherDir * quadHalfWidth;

		if( firingTakingUp > numQuadVertices / 4 )
		{
			cout << "wirestart: " << currWireStart.x << ", " << currWireStart.y << ", curr: " << currWirePos.x << ", " << currWirePos.y << endl;
			cout << "firingTakingup: " << firingTakingUp << ", count: " << numQuadVertices / 4 << endl;
			assert( false );
		}

		V2d startPartial;
		V2d endPartial;
		

		//cout << "startIndex: " << startIndex << ", firingTakingUp: " << firingTakingUp << endl;
		for( int j = 0; j < firingTakingUp; ++j )
		{
			startPartial = ( currWireStart + alongDir * (double)(tileHeight * j) );
			endPartial = ( currWireStart + alongDir * (double)(tileHeight * ( j + 1 )) );

			int diff = tileHeight * (j + 1) - length( currWirePos - currWireStart );
		
			if( diff > 0 )
			{
				assert( j == firingTakingUp - 1 );
				endPartial = currWirePos;
			}

			V2d startPartialBack = startPartial - otherDir * quadHalfWidth;
			V2d startPartialFront = startPartial + otherDir * quadHalfWidth;

			V2d endPartialBack = endPartial - otherDir * quadHalfWidth;
			V2d endPartialFront = endPartial + otherDir * quadHalfWidth;
			int index = (j + startIndex );
			quads[index*4].position = Vector2f( startPartialBack.x, startPartialBack.y );
			quads[index*4+1].position = Vector2f( startPartialFront.x, startPartialFront.y );
			quads[index*4+2].position = Vector2f( endPartialFront.x, endPartialFront.y );
			quads[index*4+3].position = Vector2f( endPartialBack.x, endPartialBack.y );

			double miniExtraWidth = 20;
			startPartialBack -= otherDir * miniExtraWidth;
			startPartialFront += otherDir * miniExtraWidth;
			endPartialBack -= otherDir * miniExtraWidth;
			endPartialFront += otherDir * miniExtraWidth;
			minimapQuads[index*4].position = Vector2f( startPartialBack.x, startPartialBack.y );
			minimapQuads[index*4+1].position = Vector2f( startPartialFront.x, startPartialFront.y );
			minimapQuads[index*4+2].position = Vector2f( endPartialFront.x, endPartialFront.y );
			minimapQuads[index*4+3].position = Vector2f( endPartialBack.x, endPartialBack.y );

			Color miniColor;
			if( right )
			{
				miniColor = Color::Red;
				
			}
			else
			{
				miniColor = Color( 0, 100, 255 );
			}

			minimapQuads[index*4].color = miniColor;
			minimapQuads[index*4+1].color = miniColor;
			minimapQuads[index*4+2].color = miniColor;
			minimapQuads[index*4+3].color = miniColor;

			int trueFrame = frame / animFactor;

		

			int fr  = frame/animFactor;
			int ifr = (numAnimFrames-1) - fr;
			int f = ifr;
			if( right )
			{
				f = fr;
			}
			else
			{
				f += numAnimFrames;
			}
			IntRect subRect = ts_wire->GetSubRect( f );
		
			quads[index*4].texCoords = Vector2f( subRect.left, subRect.top );//realTopLeft;
			quads[index*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
			quads[index*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
			quads[index*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
		}

		startIndex += firingTakingUp;

		}

		numVisibleIndexes = startIndex - 1;

		if( state == FIRING )
			++framesFiring;
	}

	//does this get called more times per frame than it should be?
	if( state == RETRACTING )
	{
		//cout << "calling to update all sprites" << endl;
		UpdateChargesSprites();
	}
}

double Wire::GetSegmentLength()
{
	V2d playerPos = storedPlayerPos;//player->position;
	double segLength;
	if( numPoints == 0 )
	{
		segLength = length( anchor.pos - playerPos );
	}
	else
	{
		segLength = length( points[numPoints-1].pos - playerPos );
	}

	return segLength;
}

void Wire::Retract()
{
	if (state == HIT || state == PULLING)
	{
	state = RETRACTING;

	retractPlayerPos = storedPlayerPos;

	ActivateCharges();
	//cout << "beginning retracting" << endl;
					
	fusePointIndex = numPoints;
	if( numPoints == 0 )
	{
		fuseQuantity = length( anchor.pos - retractPlayerPos );
	}
	else
	{
		if( right )
		{
			fuseQuantity = length( retractPlayerPos - points[numPoints-1].pos );
		}
		else
		{
			fuseQuantity = length( anchor.pos - points[0].pos );
		}
	}
	}
	else if (state == HITENEMY)
	{
		//state = RETRACTING;
		//retractPlayerPos = storedPlayerPos;
	}
}

void Wire::Draw( RenderTarget *target )
{
	//return;
	if( player->owner->showDebugDraw )
	{
		CircleShape *cstest = new CircleShape;
		cstest->setRadius( 10 );
		cstest->setFillColor( Color::Green );
		cstest->setOrigin( cstest->getLocalBounds().width / 2, cstest->getLocalBounds().height / 2 );
		cstest->setPosition( Vector2f( player->position.x, player->position.y ) );
		progressDraw.push_back( cstest );
	}

	if( state == FIRING || state == HIT || state == PULLING || state == RETRACTING || state == HITENEMY )
	{	
		target->draw( quads, numVisibleIndexes * 4, sf::Quads, ts_wire->texture );
		target->draw( wireTip );
	}
	
	if( state == HIT || state == PULLING )
	{
		CircleShape cs1;
		cs1.setFillColor( Color::Red );
		cs1.setRadius( 2 );
		cs1.setOrigin( cs1.getLocalBounds().width / 2, cs1.getLocalBounds().height / 2 );
		cs1.setPosition( anchor.pos.x, anchor.pos.y );

		target->draw( cs1 );

		for( int i = 0; i < numPoints; ++i )
		{
			CircleShape cs;
			cs.setFillColor( Color::Cyan );
			cs.setRadius( 2 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( points[i].pos.x, points[i].pos.y );

			target->draw( cs );
		}
	}

	if( state == RETRACTING )
	{
		target->draw( chargeVA, ts_wireCharge->texture );
	}
}

void Wire::DrawMinimap( sf::RenderTarget *target )
{
	if( state == FIRING || state == HIT || state == PULLING || state == RETRACTING )
	{
		target->draw( minimapQuads, numVisibleIndexes * 4, sf::Quads );
	}
}

void Wire::DebugDraw( RenderTarget *target )
{
	/*for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
		delete (*it);
	}
	progressDraw.clear();*/

	if (state == FIRING)
	{
		movingHitbox.DebugDraw(target);
	}
}

void Wire::ActivateRetractionCharges()
{
	if( right )
	{
		int limit = min( numPoints, MAX_CHARGES );
		for( int i = 0; i < limit; ++i )
		{
			ActivateWireCharge( i );
		}
	}
	else
	{
		int limit = min( numPoints, MAX_CHARGES );
		for( int i = 0; i < limit; ++i )
		{
			ActivateWireCharge( (numPoints-1)-i );
		}
	}
}

void Wire::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}

void Wire::SortNewPoints()
{
	//insertion sort just for easy testing
	if( newWirePoints > 1 )
	{
		for( int i = numPoints - newWirePoints; i < numPoints; ++i )
		{
			int j = i;

			while( j > 0 && ( (points[j-1].sortingAngleDist > points[j].sortingAngleDist) 
				|| ( points[j-1].sortingAngleDist == points[j].sortingAngleDist && length( points[j-1].pos - realAnchor )
					> length( points[j].pos - realAnchor ) ) ) )
			{
				SwapPoints( j-1, j );
				--j;
			}
		}
	}
}

double Wire::GetCurrentTotalLength()
{
	V2d playerPos = storedPlayerPos;//player->position;
	double total = 0;
			
	if( numPoints > 0 )
	{
		total += length( points[0].pos - anchor.pos );
		for( int i = 1; i < numPoints; ++i )
		{
			total += length( points[i].pos - points[i-1].pos );
		}
		total += length( points[numPoints-1].pos - playerPos );
	}
	else
	{
		total += length( anchor.pos - playerPos );
	}

	return total;
}

void Wire::Reset()
{
	state = IDLE;
	numPoints = 0;
	framesFiring = 0;
	frame = 0;
	ClearCharges();
	pullStrength = startPullStrength;
	//cout << "reset wire reset clear" << endl;
	//offsetFlagged = false;
	//clear charges gets called twice and thats not useful
}

V2d Wire::GetOriginPos( bool test )
{
	if( player->ground != NULL )
	{//only when not reversed
		V2d gNorm = player->ground->Normal();
		
		return gNorm * ( player->normalHeight / 2 - 8 );
		//if( player->ground->Normal().y 
	}
	else
	{
		return V2d( 0, -10 );
	}
	offset = player->GetWireOffset();

	if( player->facingRight )
	{
		offset.x = -abs( offset.x );
	}
	else
	{
		offset.x = abs( offset.x );
	}
	V2d playerPos;
	double angle = player->GroundedAngle();
	double x = sin( angle );
	double y = -cos( angle );
	V2d gNormal( x, y ); 
	V2d other( -gNormal.y, gNormal.x );

	if( player->ground != NULL )
	{
		V2d norm = player->ground->Normal();
		V2d pp = player->ground->GetPoint( player->edgeQuantity );
		playerPos = pp + gNormal * player->normalHeight;
		if( norm.y == -1 )
		{
			playerPos.x += player->offsetX;
		}
		else
		{
			
		}
	}
	else
	{
		playerPos = player->position;
	}

	if( test )
		playerPos = player->position;

	playerPos += gNormal * (double)offset.y - other * (double)offset.x;
	return playerPos;
}

void Wire::HitEnemy(V2d &pos)
{
	state = HITENEMY;
	hitEnemyFrame = 0;
	hitEnemyDelta = pos - GetPlayerPos();
}

void Wire::UpdateFuse()
{
	int currPoints = fusePointIndex;
	double momentum = retractSpeed;
	while( !approxEquals( momentum, 0 ) )
	{
		if( right )
		{
			if( fuseQuantity > momentum )
			{
				fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - fuseQuantity;
				fusePointIndex--;
				if( fusePointIndex == -1 )
				{
					fuseQuantity = 0;
					state = RELEASED;
					numPoints = 0;
					ClearCharges();
					
					return;
				}
				else if( fusePointIndex == 0 )
				{
					fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
				}
			}
		}
		else
		{
			if( fuseQuantity > momentum )
			{
				fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - fuseQuantity;
				fusePointIndex--;
				if( fusePointIndex == -1 )
				{
					fuseQuantity = 0;
					state = RELEASED;
					numPoints = 0;
					ClearCharges();
					//cout << "setting released A" << endl;
					return;
				}
				else if( fusePointIndex == 0 )
				{
					//cout << "zero" << endl;
					fuseQuantity = length( points[numPoints - 1].pos - retractPlayerPos );
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					fuseQuantity = length( points[numPoints-1 - fusePointIndex].pos 
					- points[(numPoints-1) - (fusePointIndex-1)].pos );
					//cout << "length: " << fuseQuantity << endl;
					
					
				}
			}
		}
	}
}

Wire::WireCharge::WireCharge( Wire*w, int vIndex )
	:vaIndex( vIndex ), wire( w ),edgeIndex( -1 )
{
	prev = NULL;
	next = NULL;
	Reset();
}

void Wire::WireCharge::Reset()
{
	ClearSprite();
	action = INACTIVE;
}

//actor should change the offset every frame based on its info. need a before movement wire position and a post movement wire position consistently
void Wire::WireCharge::UpdatePhysics()
{
	int numPoints = wire->numPoints;
	int currPoints = edgeIndex;
	double momentum = wire->retractSpeed / NUM_STEPS;
	while( !approxEquals( momentum, 0 ) )
	{
		if( wire->right )
		{
			if( edgeQuantity > momentum )
			{
				edgeQuantity -= momentum;
				//cout << "changed: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = 0;
			}
			else
			{
				//cout << "OTHER: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if( edgeIndex == -1 )
				{
					//cout << "DEACTIVA" << endl;
					action = INACTIVE;
					ClearSprite();
					wire->DeactivateWireCharge( this );

					//assert( wire->activeChargeList == NULL );
					return;
				}
				else if( edgeIndex == 0 )
				{
					//cout << "blah anchor" << endl;
					edgeQuantity = length( wire->points[edgeIndex].pos - wire->anchor.pos );
				}
				else
				{
					//cout << "edgeIndex: " << edgeIndex << endl;
					edgeQuantity = length( wire->points[edgeIndex].pos - wire->points[edgeIndex-1].pos );
				}
			}

			if( edgeIndex == 0 )
			{
				V2d dir = normalize( wire->points[0].pos- wire->anchor.pos );
				position = wire->anchor.pos + dir * edgeQuantity;
			}
			else
			{
				V2d dir = normalize( wire->points[edgeIndex].pos - wire->points[edgeIndex-1].pos );
				position = wire->points[edgeIndex-1].pos + dir * edgeQuantity;
			}
		}
		else
		{
			if( edgeQuantity > momentum )
			{
				edgeQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				int numPoints = wire->numPoints;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if( edgeIndex == -1 )
				{
					action = INACTIVE;
					ClearSprite();
					wire->DeactivateWireCharge( this );
					return;
				}
				else if( edgeIndex == 0 )
				{
					//cout << "zero" << endl;
					edgeQuantity = length( wire->points[numPoints - 1].pos - wire->retractPlayerPos );
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					edgeQuantity = length( wire->points[(numPoints-1) - edgeIndex].pos 
					- wire->points[(numPoints-1) - (edgeIndex-1)].pos );
					//cout << "length: " << fuseQuantity << endl;
				}
			}

			
			if( edgeIndex == 0 )
			{
				V2d dir = normalize( wire->points[wire->numPoints-1].pos - wire->retractPlayerPos );//wire->anchor.pos );
				position = wire->retractPlayerPos + dir * edgeQuantity;
			}
			else
			{
				V2d dir = normalize( wire->points[(numPoints-1) - edgeIndex].pos - wire->points[(numPoints-1) - (edgeIndex-1)].pos );
				position = wire->points[(numPoints-1) - (edgeIndex-1)].pos + dir * edgeQuantity;
			}

			//cout << "position: " << position.x << ", " << position.y << endl;
			//position = //wire->points[edgeIndex].pos;
			//position = wire->retractPlayerPos;
		}
	}
}

void Wire::WireCharge::UpdatePrePhysics()
{
}

void Wire::WireCharge::HitEnemy()
{
	Reset();
	wire->DeactivateWireCharge( this );
}

void Wire::CreateWireCharge()
{
	WireCharge *charge = new WireCharge( this, numTotalCharges );
	++numTotalCharges;
	if( inactiveChargeList == NULL )
	{
		inactiveChargeList = charge;
	}
	else
	{
		charge->next = inactiveChargeList;
		inactiveChargeList->prev = charge;
		inactiveChargeList = charge;
	}
}

void Wire::ActivateWireCharge( int index )
{
	WireCharge *charge = GetWireCharge();
	if( charge != NULL )
	{
		if( right )
		{
			//cout << "activate!: " << index << endl;
			charge->edgeIndex = index;
			if( index > 0 )
			{
				charge->edgeQuantity = length(points[index].pos - points[index-1].pos);
				//cout << "setting quant: " << charge->edgeQuantity << endl;
			}
			else
			{
				charge->edgeQuantity = length( points[0].pos - anchor.pos );
				//cout << "setting quantb: " << charge->edgeQuantity << endl;
			}
		
			//charge->edgeQuantity = 0;
			charge->position = points[index].pos;
			charge->action = WireCharge::Action::RETRACTING;

			if( activeChargeList == NULL )
			{
				activeChargeList = charge;
				charge->next = NULL;
				charge->prev = NULL;
			}
			else
			{
				charge->next = activeChargeList;
				activeChargeList->prev = charge;
				activeChargeList = charge;
			}
		}
		else
		{
			//cout << "LEFT active: " << CountActiveCharges() << ", inact: " << CountInactiveCharges() << endl;
			//cout << "activate LEFT!: " << index << endl;
			charge->edgeIndex = index;
			if( index > 0 )
			{
				
				charge->edgeQuantity = length(points[(numPoints-1)-index].pos - points[(numPoints-1) - (index-1)].pos);
				//cout << "setting quant: " << charge->edgeQuantity << endl;
			}
			else
			{
				charge->edgeQuantity = length( points[numPoints-1].pos - retractPlayerPos );//..anchor.pos );
				//cout << "setting quantb: " << charge->edgeQuantity << endl;
			}
		
			//charge->edgeQuantity = 0;
			charge->position = points[(numPoints-1)-index].pos;
			charge->action = WireCharge::Action::RETRACTING;

			if( activeChargeList == NULL )
			{
				activeChargeList = charge;
				charge->next = NULL;
				charge->prev = NULL;
			}
			else
			{
				charge->next = activeChargeList;
				activeChargeList->prev = charge;
				activeChargeList = charge;
			}
		}
	}
}

void Wire::DeactivateWireCharge( WireCharge *charge )
{
	//cout << "removing charge from list!" << endl;
	assert( activeChargeList != NULL );
	//cout << "removing active: " << CountActiveCharges() << ", inact: " << CountInactiveCharges() << endl;
	if( charge->prev == NULL && charge->next == NULL )
	{
		assert( activeChargeList == charge );
		activeChargeList = NULL;
		//charge->prev = NULL;
		//charge->next = NULL;
	}
	else if( charge == activeChargeList )
	{
		WireCharge *n = activeChargeList->next;

		activeChargeList->next->prev = NULL;
		activeChargeList->next = NULL;

		activeChargeList = n;
	}
	else
	{
		if( charge->prev != NULL )
		{
			charge->prev->next = charge->next;
		}

		if( charge->next != NULL )
		{
			charge->next->prev = charge->prev;
		}

		charge->prev = NULL;
		charge->next = NULL;
	}

	if( inactiveChargeList == NULL )
	{
		inactiveChargeList = charge;
	}
	else
	{
		charge->next = inactiveChargeList;
		inactiveChargeList->prev = charge;
		inactiveChargeList = charge;
	}
	//cout << "AFTER removing active: " << CountActiveCharges() << ", inact: " << CountInactiveCharges() << endl;
}

void Wire::DestroyDeactivatedCharges()
{
	WireCharge *curr = inactiveChargeList;
	WireCharge *n = NULL;
	while (curr != NULL)
	{
		n = curr->next;
		delete curr;
		curr = n;
	}
}

Wire::WireCharge *Wire::GetWireCharge()
{
	if( inactiveChargeList == NULL )
		return NULL;
	else
	{
		if( inactiveChargeList->next == NULL )
		{
			Wire::WireCharge * temp = inactiveChargeList;
			inactiveChargeList = NULL;

			return temp;
		}
		else
		{
			Wire::WireCharge *old = inactiveChargeList;
			Wire::WireCharge *newList = old->next;

			old->next = NULL;

			newList->prev = NULL;

			inactiveChargeList = newList;

			return old;
		}
	}
}

void Wire::WireCharge::UpdateSprite()
{
	if( action == INACTIVE )
	{
		ClearSprite();
		//cout << "updating inactive sprite" << endl;
	}
	else
	{
		//cout << "wire charge sprite update" << endl;
		VertexArray &va = wire->chargeVA;
		IntRect subRect = wire->ts_wireCharge->GetSubRect( 0 );//frame / animFactor );
		//if( owner->GetPlayer( 0 )->position.x < position.x )
		//{
		//	subRect.left += subRect.width;
		//	subRect.width = -subRect.width;
		//}
		va[vaIndex*4+0].texCoords = Vector2f( subRect.left, 
			subRect.top );
		va[vaIndex*4+1].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top );
		va[vaIndex*4+2].texCoords = Vector2f( subRect.left 
			+ subRect.width, 
			subRect.top + subRect.height );
		va[vaIndex*4+3].texCoords = Vector2f( subRect.left, 
			subRect.top + subRect.height );

		//va[vaIndex*4+0].color = Color::Red;
		//va[vaIndex*4+1].color = Color::Red;
		//va[vaIndex*4+2].color = Color::Red;
		//va[vaIndex*4+3].color = Color::Red;

		Vector2f p( position.x, position.y );

		Vector2f spriteSize = Vector2f( subRect.width / 2, subRect.height / 2 );
		//spriteSize *= 10.f;
		va[vaIndex*4+0].position = p + Vector2f( -spriteSize.x, -spriteSize.y );
		va[vaIndex*4+1].position = p + Vector2f( spriteSize.x, -spriteSize.y );
		va[vaIndex*4+2].position = p + Vector2f( spriteSize.x, spriteSize.y );
		va[vaIndex*4+3].position = p + Vector2f( -spriteSize.x, spriteSize.y );
	}
}

void Wire::WireCharge::ClearSprite()
{
	VertexArray &va = wire->chargeVA;
	va[vaIndex*4+0].position = Vector2f( 0, 0 );
	va[vaIndex*4+1].position = Vector2f( 0, 0 );
	va[vaIndex*4+2].position = Vector2f( 0, 0 );
	va[vaIndex*4+3].position = Vector2f( 0, 0 );
}

void Wire::UpdateChargesPhysics()
{
	if( state != RETRACTING )
		return;

	WireCharge *curr = activeChargeList;
	WireCharge *temp;
	int numList = 0;
	while( curr != NULL )
	{
		temp = curr->next;
		//cout << "updating charge: " << numList << endl;
		curr->UpdatePhysics();
		curr = temp;
		++numList;
	}

}

void Wire::UpdateChargesPrePhysics()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		curr->UpdatePrePhysics();
		curr = curr->next;
	}
}

void Wire::UpdateChargesPostPhysics()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		curr->UpdatePrePhysics();
		curr = curr->next;
	}
}

void Wire::UpdateChargesSprites()
{
	WireCharge *curr = activeChargeList;
	while( curr != NULL )
	{
		//cout << "update sprite---!!: " << endl;
		curr->UpdateSprite();
		curr = curr->next;
	}
}
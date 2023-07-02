#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>
#include <assert.h>
#include "Enemy.h"
#include "Grass.h"
#include "GGPO.h"
#include "EditorRail.h"

using namespace sf;
using namespace std;

WirePoint::WirePoint()
{
	Reset();
}
void WirePoint::Reset()
{
	edgeInfo.Clear();
	enemyIndex = -1;
	start = false;
	clockwise = false;
	quantity = 0;
	angleDiff = 0;
	sortingAngleDist = 0;
}


void Wire::PopulateWireInfo(Wire::MyData *wi)
{
	data.rcEdge.SetFromEdge( rayCastInfo.rcEdge );
	data.rcQuant = rayCastInfo.rcQuant;

	memcpy(wi, &data, sizeof(data));
}

void Wire::PopulateFromWireInfo(Wire::MyData *wi)
{
	memcpy(&data, wi, sizeof(data));

	rayCastInfo.rcEdge = player->sess->GetEdge(&data.rcEdge);
	rayCastInfo.rcQuant = data.rcQuant;
}

//removed hitstallframes from the wire info

Wire::Wire( Actor *p, bool r)
	:fireRate( 200/*120*/ ), maxTotalLength( 10000 ), maxFireLength( 5000 ), player( p ), hitStallFrames( 8/*10*/ ), right( r )
	, extraBuffer( MAX_POINTS ),//64  ),
	//eventually you can split this up into smaller sections so that they don't all need to draw
  quadHalfWidth( 8 ), ts_wire( NULL ), animFactor( 1 )
{
	//int numCurveQuads = 2;
	//int numCurveVertices = (MAX_POINTS + 1) * 4;
	numQuadVertices = (MAX_POINTS+1) * 4;//(int)((ceil( maxTotalLength / 8.0 ) + extraBuffer) * 4 );
	quads = new Vertex[numQuadVertices];// +numCurveVertices];

	nodeQuads = new Vertex[numQuadVertices];

	minimapQuads = new Vertex[numQuadVertices];

	if (!wireShader.loadFromFile("Resources/Shader/wire_shader.frag", sf::Shader::Fragment))
	{
		assert(0);
	}

	data.state = IDLE;
	data.numPoints = 0;
	data.framesFiring = 0;
	data.minSegmentLength = 128;
	data.hitStallCounter = 0;
	data.frame = 0;
	data.offset = Vector2i(8, 18);
	

	data.aimingPrimaryAngleRange = 2;
	data.hitEnemyFramesTotal = 5;

	data.antiWireGrassCount = 0;

	int tipIndex = 0;
	ts_wire = player->sess->GetTileset( "Kin/Powers/wires_16x16.png", 16, 16 );
	if( r )
	{
		ts_miniHit = player->sess->GetTileset( "Env/rain_64x64.png", 64, 64 );
		tipIndex = 0;
	}
	else
	{
		tipIndex = 2;
		ts_miniHit = player->sess->GetTileset( "Env/rain_64x64.png", 64, 64 );
	}

	ts_wireNode = player->sess->GetSizedTileset("Kin/Powers/wire_node_16x16.png");

	ts_wireTip = player->sess->GetTileset( "Kin/Powers/wire_tips_16x16.png", 16, 16 );

	wireShader.setUniform("u_texture", *ts_wire->texture);//sf::Shader::CurrentTexture);
	wireShader.setUniform("u_quant", 0.f);

	grassCheckRadius = 20;

	wireTip.setTexture( *ts_wireTip->texture );
	wireTip.setTextureRect( ts_wireTip->GetSubRect( tipIndex ) );
	wireTip.setOrigin( wireTip.getLocalBounds().width / 2, wireTip.getLocalBounds().height / 2 );


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
	data.minSideOther = -1;
	data.minSideAlong = -1;

	triggerDown = false;
	prevTriggerDown = false;

	retractSpeed = 30;//20;//60;

	numAnimFrames = 16;

	//pullStrength = 10;
	maxPullStrength = 10;//10;
	startPullStrength = maxPullStrength;//10;
	data.pullStrength = startPullStrength;
	pullAccel = (maxPullStrength - startPullStrength) / 180;
	//.1 = 10 frames per 1. 100 frames per 10

	maxDragStrength = 30;
	startDragStrength = 10;
	data.dragStrength = startDragStrength;
	dragAccel = (maxDragStrength - startDragStrength) / 180.0;

	Reset();
}

Wire::~Wire()
{
	delete[] quads;
	delete[] minimapQuads;
	delete[] nodeQuads;

	delete tipHitboxInfo;
}

V2d Wire::GetPlayerPos()
{
	data.currOffset = V2d( 0, 0 );//GetOriginPos( true );
	if( false )//offset != currOffset )
	{
		//offsetFlagged = true;
		//currOffset = offset;
		//UpdateAnchors( V2d(0, 0 ) );
		return player->position + data.currOffset;
	}
	else
	{
		
		return player->position + data.currOffset;
	}
}

void Wire::UpdateState( bool touchEdgeWithWire )
{
	
	//cout << "update state" << endl;
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;

	//V2d playerPos = player->position;
	
	V2d playerPos;
	if(data.state == RETRACTING )
	{
		playerPos = data.retractPlayerPos;
	}
	else
	{
		playerPos = GetPlayerPos();//GetOriginPos(true);
	}
	data.storedPlayerPos = playerPos;

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

	if(data.state == PULLING )
	{ 
		if (player->ground != NULL || player->bounceEdge != NULL || player->grindEdge != NULL )
		{
			data.state = HIT;
		}
	}


	switch(data.state )
	{
	case IDLE:
		{
			TryFire();
			break;
		}
	case FIRING:
		{
			if( rayCastInfo.rcEdge != NULL )
			{
				CheckAntiWireGrass();

				if (data.antiWireGrassCount == 0)
				{
					data.state = HIT;
					SetCanRetractGround();
					data.hitStallCounter = data.framesFiring;

				}
				else
				{
					Reset();
				}
			}

			if(data.framesFiring * fireRate > maxFireLength )
			{
				Reset();
				
			}
			break;
		}
	case HIT:
		{
			data.totalLength = GetCurrentTotalLength();

			bool hitStallFinished = data.hitStallCounter >= hitStallFrames;

			if(data.totalLength > maxTotalLength )
			{
				data.state = RELEASED;
				data.numPoints = 0;
				break;
			}
			else
			{
				//if( canRetractGround && 
				if(hitStallFinished && !triggerDown && prevTriggerDown )
				{
					Retract();
					
					break;
				}
				else
				{
					if( !triggerDown )
					{
						data.canRetractGround = true;
					}
				}
			}

			bool a = player->ground == NULL;
			bool b = !touchEdgeWithWire;
			
			bool d = triggerDown;
			bool e = player->oldAction != Actor::WALLCLING && player->oldAction != Actor::WALLATTACK && player->action != Actor::WALLATTACK;
			bool f = ( !player->bounceFlameOn || player->framesSinceBounce > 8 || player->oldBounceEdge == NULL ) && player->bounceEdge == NULL;
			
			if( a && b && hitStallFinished && d && e && f )
			{
				//cout << "playeraction: " << player->action << endl;
				//cout << "set state pulling" << endl;
				data.state = PULLING;
				data.pullStrength = startPullStrength;
				data.dragStrength = startDragStrength;
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
		data.totalLength = GetCurrentTotalLength();

			if(data.totalLength > maxTotalLength )
			{
				data.state = RELEASED;
				data.numPoints = 0;
			}
			else
			{
				

				if( !triggerDown && player->ground == NULL )
				{
					data.state = RETRACTING;
					data.retractPlayerPos = playerPos;
					data.fusePointIndex = data.numPoints;
					if(data.numPoints == 0 )
					{
						data.fuseQuantity = length(data.anchor.pos - data.retractPlayerPos );
					}
					else
					{
						if( true )
						{
							data.fuseQuantity = length(data.retractPlayerPos - data.points[data.numPoints - 1].pos);
						}
						else
						{
							data.fuseQuantity = length(data.anchor.pos - data.points[0].pos );
						}
					}
				}
				if( triggerDown && ( touchEdgeWithWire || player->action == Actor::WALLCLING ) )
				{
					data.state = HIT;
					SetCanRetractGround();
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

		if (data.hitEnemyFrame == data.hitEnemyFramesTotal)
		{
			Reset();
			//Retract();
		}
		break;
	}
	}


	if (data.state == RETRACTING)
	{
		playerPos = data.retractPlayerPos;
	}
	else
	{
		playerPos = GetPlayerPos();//GetOriginPos(true);
	}
	data.storedPlayerPos = playerPos;

	switch(data.state )
	{
	case IDLE:
		{
			//no updates
			break;
		}
	case FIRING:
		{
			rayCastInfo.rcEdge = NULL;
			data.rcCancelDist = -1;
			V2d currPos = playerPos + data.fireDir * fireRate * (double)(data.framesFiring);
			V2d futurePos = playerPos + data.fireDir * fireRate * (double)(data.framesFiring + 1);
			RayCast( this, player->GetTerrainTree()->startNode, playerPos, futurePos );
			RayCast(this, player->GetRailEdgeTree()->startNode, playerPos, futurePos);
			RayCast(this, player->GetBarrierTree()->startNode, playerPos, futurePos);

			data.fireDir = normalize(data.fireDir);
			double len = length(futurePos - currPos);

			data.movingHitbox.SetRectDir(data.fireDir, len, 30);
			data.movingHitbox.globalPosition = (currPos + futurePos) / 2.0;

			tipHitboxInfo->hDir = data.fireDir;

			if (rayCastInfo.rcEdge != NULL && data.rcCancelDist >= 0)
			{
				double rcLength = length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant) - playerPos);
				if (data.rcCancelDist < rcLength)
				{
					Reset();
					return;
				}
			}
			else if (data.rcCancelDist >= 0)
			{
				Reset();
				return;
			}

			//cout << "framesFiring " << framesFiring << endl;

			if(rayCastInfo.rcEdge != NULL )
			{
				CheckAntiWireGrass();

				if (data.antiWireGrassCount > 0)
				{
					Reset();
					break;
				}

				if (rayCastInfo.rcEdge->rail != NULL
					&& rayCastInfo.rcEdge->rail->GetRailType() == TerrainRail::WIREBLOCKING)
				{
					Reset();
					break;
				}


				//cout << "hit edge!: " << rcEdge->Normal().x << ", " << rcEdge->Normal().y << ", : " << rcEdge << endl;
				if(rayCastInfo.rcQuant < 4 )
				{
					//cout << "Aw" << endl;
					//cout << "lock1" << endl;
					data.anchor.pos = rayCastInfo.rcEdge->v0;
				}
				else if(rayCastInfo.rcQuant > length(rayCastInfo.rcEdge->v1 - rayCastInfo.rcEdge->v0 ) - 4 )
				{
					//cout << "Bw" << endl;
					//cout << "lock2" << endl;
					data.anchor.pos = rayCastInfo.rcEdge->v1;
				}
				else
				{
					//cout << "Cw" << endl;
					data.anchor.pos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant );
				}
				
				data.anchor.edgeInfo.SetFromEdge(rayCastInfo.rcEdge);
				data.anchor.quantity = rayCastInfo.rcQuant;

				//cout << "anchor pos: " << anchor.pos.x << ", " << anchor.pos.y << endl;
				//player->owner->ActivateEffect( ts_miniHit, rcEdge->GetPoint( rcQuant ), true, 0, , 3, facingRight );

				data.numPoints = 0;

				data.state = HIT;

				SetCanRetractGround();
				data.hitStallCounter = data.framesFiring;

				data.storedPlayerPos = playerPos;
			}
			break;
		}
	case HIT:
		{


			if(data.hitStallCounter < hitStallFrames )
				data.hitStallCounter++;
			break;
		}
	case PULLING:
		{
			//cout << "pulling!" << endl;
		data.totalLength = GetCurrentTotalLength();

			V2d wn;
			data.segmentLength = GetSegmentLength();

			if(data.numPoints == 0 )
			{
				//segmentLength = totalLength;
				wn = normalize(data.anchor.pos - playerPos );
				//cout << "A segmentLength: " << segmentLength << endl;
			}
			else
			{
				//double segmentLength = length( points[numPoints-1].pos - playerPos );
				wn = normalize(data.points[data.numPoints-1].pos - playerPos );
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
			if( player->JumpButtonHeld() )
			{
				shrinkInput = true;
				data.dragStrength = startDragStrength;
			}
			else if( player->DashButtonHeld() )
			{
				if( triggerDown && player->ground == NULL )
				{
					data.segmentLength += data.dragStrength;
					data.totalLength += data.dragStrength;

					data.dragStrength += dragAccel;
					if(data.dragStrength > maxDragStrength )
						data.dragStrength = maxDragStrength;
					//cout << "GROWING" << endl;
				}
				
			}
			else
			{
				data.dragStrength = startDragStrength;
			}

			bool bounceWindow = (player->action == Actor::BOUNCEAIR && player->framesSinceBounce > 10)
				|| player->action != Actor::BOUNCEAIR;
			bool c = data.totalLength > 128;//minSegmentLength > 128;//
			if( shrinkInput && triggerDown && player->ground == NULL && c && bounceWindow )
			{
				//cout << "SHRINKING " << endl;
				double segmentChange = data.pullStrength;
				double minSeg = 100;
				double maxSeg = 1000;

				if(data.segmentLength < minSeg )
				{
					segmentChange = data.pullStrength * .1;
				}
				else if(data.segmentLength > maxSeg )
				{
					segmentChange = data.pullStrength * 1.0;
				}
				else
				{
					segmentChange = data.pullStrength * min((data.segmentLength-minSeg) / (maxSeg - minSeg) + .1, 1.0);
				}
				//min( max( segmentLength / 1000.0, 1.0 ), 1.0;

				if(data.segmentLength - segmentChange < data.minSegmentLength )
					segmentChange = 0;//-(minSegmentLength - (segmentLength - pullStrength));

				data.totalLength -= segmentChange;
				data.segmentLength -= segmentChange;

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
				data.pullStrength = startPullStrength;
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
		++data.hitEnemyFrame;
		break;
	}
	}

	++data.frame;
	if(data.frame / animFactor == numAnimFrames )
	{
		data.frame = 0;
	}

	double offsetAmount = .002;
	if (right)
	{
		data.shaderOffset -= offsetAmount;
	}
	else
	{
		data.shaderOffset += offsetAmount;
	}

	wireShader.setUniform("u_quant", data.shaderOffset);
}

bool Wire::TryFire()
{
	ControllerState &currInput = player->currInput;
	ControllerState &prevInput = player->prevInput;
	if (player->CanShootWire() && triggerDown && !prevTriggerDown)
	{
		//cout << "firing" << endl;
		data.fireDir = V2d(0, 0);

		if (false)
		{
			if (currInput.LLeft())
			{
				data.fireDir.x -= 1;
			}
			else if (currInput.LRight())
			{
				data.fireDir.x += 1;
			}

			if (currInput.LUp())
			{
				if (player->reversed)
				{
					data.fireDir.y += 1;
				}
				else
				{
					data.fireDir.y -= 1;
				}

			}
			else if (currInput.LDown())
			{
				if (player->reversed)
				{
					data.fireDir.y -= 1;
				}
				else
				{
					data.fireDir.y += 1;
				}

			}
		}
		else
		{
			int leftDir = currInput.GetLeftStickDirection();
			if( leftDir >= 0 )
			//if (currInput.leftStickMagnitude > 0)
			{
				double angle = (PI / 32.0) * leftDir;
				data.fireDir.x = cos(angle);
				data.fireDir.y = -sin(angle);
			}
			else
			{
				if (player->facingRight)
				{
					data.fireDir = V2d(1, -1);
				}
				else
				{
					data.fireDir = V2d(-1, -1);
				}

				if (player->reversed)
				{
					data.fireDir.y = 1.0;
				}
			}

		}


		data.fireDir = normalize(data.fireDir);

		float angle = atan2(data.fireDir.y, data.fireDir.x);

		data.state = FIRING;
		//cout << "firing from idle" << endl;
		data.framesFiring = 0;
		data.frame = 0;

		if (data.anchor.enemyIndex >= 0 )
		{
			Enemy *enemy = player->sess->GetEnemy(data.anchor.enemyIndex);
			enemy->HandleWireUnanchored(this);
		}

		data.anchor.Reset();

		wireTip.setRotation((angle / PI ) * 180 + 90);

		return true;
	}

	return false;
}

void Wire::SwapPoints( int aIndex, int bIndex )
{
	WirePoint temp = data.points[aIndex];
	data.points[aIndex] = data.points[bIndex];
	data.points[bIndex] = temp;
}

void Wire::UpdateEnemyAnchor()
{
	if (data.anchor.enemyIndex >= 0)
	{
		Enemy *enemy = player->sess->GetEnemy(data.anchor.enemyIndex);

		V2d oldPos = data.anchor.pos;
		data.anchor.pos = enemy->GetCamPoint(data.anchor.enemyPosIndex);
		data.realAnchor = data.anchor.pos;
		data.anchorVel = data.realAnchor - data.oldPos;

	}
}

CollisionBox * Wire::GetTipHitbox()
{
	if (data.state == FIRING)
	{
		return &data.movingHitbox;
	}
	else
	{
		return NULL;
	}
}

bool Wire::IsValidTrackEnemy(Enemy *e)
{
	return e->CanBeAnchoredByWire(right);
}

void Wire::UpdateAnchors( V2d vel )
{
	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if ((data.state == HIT || data.state == PULLING) && data.anchor.enemyIndex == -1)
	{
		if (data.oldPos.x == data.storedPlayerPos.x && data.oldPos.y == data.storedPlayerPos.y)
		{
			//return;
		}

		data.oldPos = data.storedPlayerPos;

		double radius = length(data.realAnchor - playerPos); //new position after moving

		if (data.numPoints == 0)
		{
			//line->append( sf::Vertex(sf::Vector2f(anchor.pos.x, anchor.pos.y), Color::Black) );
			data.realAnchor = data.anchor.pos;
		}
		else
		{
			//line->append( sf::Vertex(sf::Vector2f(points[numPoints - 1].pos.x, points[numPoints - 1].pos.y), Color::Black) );
			data.realAnchor = data.points[data.numPoints - 1].pos;
		}

		if (vel.x == 0 && vel.y == 0)
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



		if (counter > 1)
		{
			//cout << "COUNTER: " << counter << endl;
		}

		a = data.realAnchor - data.oldPos;
		b = data.realAnchor - playerPos;
		len = max(length(a), length(b));

		oldDir = data.oldPos - data.realAnchor;
		dir = playerPos - data.realAnchor;

		left = min(min(data.realAnchor.x, data.oldPos.x), playerPos.x);
		top = min(min(data.realAnchor.y, data.oldPos.y), playerPos.y);
		right = max(max(data.realAnchor.x, data.oldPos.x), playerPos.x);
		bottom = max(max(data.realAnchor.y, data.oldPos.y), playerPos.y);

		r.left = left - ex;
		r.top = top - ex;
		r.width = (right - left) + ex * 2;
		r.height = (bottom - top) + ex * 2;

		data.foundPoint = false;
		data.newWirePoints = 0; //number of points added

		V2d normalizedA = normalize(a);
		V2d otherA(normalizedA.y, -normalizedA.x);
		if (dot(normalize(playerPos - data.oldPos), otherA) > 0)
			data.clockwise = true;
		else
			data.clockwise = false;

		queryMode = "terrain";
		player->GetTerrainTree()->Query(this, r);
		//player->owner->barrierTree->Query(this, r);
		if (data.state == RELEASED)
		{
			//cout << "went too many points" << endl;
			//should cut the wire when you go over the point count
			return;
		}

		//if( player->owner != NULL && player->owner->showDebugDraw )
		//{
		//	sf::RectangleShape queryDebug;
		//	queryDebug.setPosition( Vector2f( r.left, r.top ) );
		//	queryDebug.setSize( Vector2f( r.width, r.height ) );
		//	queryDebug.setFillColor( Color::Transparent );//Color( 255, 255, 255, 100 ) );
		//	queryDebug.setOutlineColor( Color::White );
		//	queryDebug.setOutlineThickness( 1 );
		//	progressDraw.push_back( new sf::RectangleShape( queryDebug ) );
		//}

		SortNewPoints();

		//remove points as need be
		for (int i = data.numPoints - 1; i >= 0; --i)
		{
			double result = cross(playerPos - data.points[data.numPoints - 1].pos, data.points[i].test);
			if (result > 0)
			{
				//V2d along = 
				//cout << "removing along: " << 
				//cout << "removePoint: " << points[numPoints-1].pos.x << ", " << points[numPoints-1].pos.y << endl;
				data.numPoints--;
			}
			else
			{
				break;
			}
		}
	}
	else if (data.state == FIRING)
	{
		data.oldPos = data.storedPlayerPos;
		V2d wireVec = data.fireDir * fireRate * (double)(data.framesFiring + 1);

		V2d diff = playerPos - data.oldPos;

		V2d wirePos = playerPos + wireVec;
		V2d oldWirePos = data.oldPos + wireVec;

		data.quadOldPosA = data.oldPos;
		data.quadOldWirePosB = oldWirePos;
		data.quadWirePosC = wirePos;
		data.quadPlayerPosD = playerPos;

		Enemy *foundEnemy = NULL;
		int foundIndex;



		if ( ((right && player->HasUpgrade( Actor::UPGRADE_W6_WIRE_ENEMIES_RIGHT) )
			|| (!right && player->HasUpgrade( Actor::UPGRADE_W6_WIRE_ENEMIES_LEFT )))
			&& GetClosestEnemyPos(wirePos, 128, foundEnemy, foundIndex))
		{
			data.storedPlayerPos = playerPos;
			data.state = HIT;
			data.hitStallCounter = data.framesFiring;
			SetCanRetractGround();
			data.numPoints = 0;
			data.anchor.pos = foundEnemy->GetCamPoint(foundIndex); //minSideEdge->v0;
			data.anchor.quantity = 0;

			data.anchor.Reset();

			data.anchor.enemyIndex = player->sess->GetEnemyID(foundEnemy);
			data.anchorVel = V2d(0, 0);
			data.anchor.enemyPosIndex = foundIndex;
			UpdateAnchors(V2d(0, 0));

			foundEnemy->HandleWireAnchored(this);
		}

		//for grabbing onto points
		double top = min(data.quadOldPosA.y, min(data.quadOldWirePosB.y, min(data.quadWirePosC.y, data.quadPlayerPosD.y)));
		double bot = max(data.quadOldPosA.y, max(data.quadOldWirePosB.y, max(data.quadWirePosC.y, data.quadPlayerPosD.y)));
		double left = min(data.quadOldPosA.x, min(data.quadOldWirePosB.x, min(data.quadWirePosC.x, data.quadPlayerPosD.x)));
		double right = max(data.quadOldPosA.x, max(data.quadOldWirePosB.x, max(data.quadWirePosC.x, data.quadPlayerPosD.x)));

		double ex = 1;
		sf::Rect<double> r(left - ex, top - ex, (right - left) + ex * 2, (bot - top) + ex * 2);

		if (r.width == 0 || r.height == 0)
		{

		}
		else
		{

			//queryType = "terrain";
			minSideEdge = NULL;
			queryMode = "terrain";
			player->GetTerrainTree()->Query(this, r);
			player->GetRailEdgeTree()->Query(this, r);
			player->GetBarrierTree()->Query(this, r);
			if (minSideEdge != NULL)
			{
				if (minSideEdge->rail != NULL && minSideEdge->rail->GetRailType() == TerrainRail::WIREBLOCKING)
				{
					//dont connect here/block instead
				}
				else
				{
					data.storedPlayerPos = playerPos;
					data.state = HIT;
					data.hitStallCounter = data.framesFiring;
					SetCanRetractGround();
					data.numPoints = 0;
					data.anchor.pos = minSideEdge->v0;
					data.anchor.quantity = 0;
					data.anchor.edgeInfo.SetFromEdge(minSideEdge);
					UpdateAnchors(V2d(0, 0));
				}
			}
		}
	}

	data.storedPlayerPos = playerPos;
}

void Wire::SetCanRetractGround()
{
	if (!triggerDown)
	{
		data.canRetractGround = true;
	}
	else
	{
		data.canRetractGround = false;
	}

	//canRetractGround = true;
}

void Wire::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	/*if (edge->IsUnlockedGateEdge())
	{
		return;
	}*/

	V2d playerPos = GetPlayerPos();
	double lengthToPlayer = length(edge->GetPosition(edgeQuantity) - playerPos );
	
	double rcLength = 0;
	if (rayCastInfo.rcEdge != NULL)
	{
		rcLength = length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant) - playerPos);
	}

	if (edge->IsInvisibleWall())
	{
		if (data.rcCancelDist < 0 || lengthToPlayer < data.rcCancelDist)
			data.rcCancelDist = lengthToPlayer;
	}
	else if (edge->IsGateEdge())
	{
		//if (edge->IsUnlockedGateEdge())
		//{
		//	if (data.rcCancelDist < 0 || lengthToPlayer < data.rcCancelDist)
		//		data.rcCancelDist = lengthToPlayer;
		//	//return;
		//}


		Gate *g = (Gate*)edge->info;
		if (g->CanBeHitByWire())
		{
			if (rayCastInfo.rcEdge == NULL || lengthToPlayer < rcLength)
			{
				rayCastInfo.rcEdge = edge;
				rayCastInfo.rcQuant = edgeQuantity;
			}
		}
		else
		{
			if (data.rcCancelDist < 0 || lengthToPlayer < data.rcCancelDist)
			{
				data.rcCancelDist = lengthToPlayer;
			}
		}
	}
	else
	{
		if (rayCastInfo.rcEdge == NULL || lengthToPlayer < rcLength)
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
	}
}

//returns -1 on errror
double Wire::GetTestPointAngle( Edge *e )
{
	V2d p = e->v0;

	V2d playerPos = GetPlayerPos();//GetOriginPos(true);

	if( length( p - data.realAnchor ) < 1 ) //if applied to moving platforms this will need to account for rounding bugs.
	{
		return -1;
	}

	double radius = length(data.realAnchor - playerPos ); //new position after moving

	double anchorDist = length(data.realAnchor - p );

	if( anchorDist > radius )
	{
		return -1;
	}

	V2d oldVec = normalize(data.oldPos - data.realAnchor );
	V2d newVec = normalize(playerPos - data.realAnchor );

	V2d pDiff = p - data.realAnchor;
	V2d pVec = normalize( pDiff );

	double testAngleAngle = atan2((long double)pDiff.y, (long double)pDiff.x);

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

	if( length( p - data.realAnchor ) < 1 ) //if applied to moving platforms this will need to account for rounding bugs.
	{
		return;
	}

	double radius = length(data.realAnchor - playerPos ); //new position after moving

	double anchorDist = length(data.realAnchor - p );

	if( anchorDist > radius )
	{
		return;
	}
	

	V2d oldVec = normalize(data.oldPos - data.realAnchor );
	V2d newVec = normalize( playerPos - data.realAnchor );

	V2d pVec = normalize( p - data.realAnchor );

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
	data.clockwise = tempClockwise;
	
	if( !data.foundPoint )
	{
		data.foundPoint = true;
		data.closestDiff = angleDiff;
		data.closestPoint = p;
		//
	}
	else
	{
		if( angleDiff < data.closestDiff )
		{
			data.closestDiff = angleDiff;
			data.closestPoint = p;
		}
		else if( approxEquals( angleDiff, data.closestDiff ) )
		{
			double closestDist = length(data.realAnchor - data.closestPoint );
			if( anchorDist < closestDist )
			{
				data.closestDiff = angleDiff;
				data.closestPoint = p;
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
		if(data.numPoints < MAX_POINTS )
		{
			WirePoint &wp = data.points[data.numPoints];
			wp.pos = p;
			
			wp.sortingAngleDist = res;

			wp.test = normalize( p - data.realAnchor );
			
			if( !data.clockwise )
			{
				data.points[data.numPoints].test = -data.points[data.numPoints].test;
			}

			//cout << "adding point with test: " << wp.test.x << ", " << wp.test.y << endl;

			data.numPoints++;
		}
		else
		{
			data.state = RELEASED;
			data.numPoints = 0;
		}
		data.newWirePoints++;
	}
}

void Wire::HandleEntrant( QuadTreeEntrant *qte )
{
	if (queryMode == "terrain")
	{
		Edge *e = (Edge*)qte;

		if (data.state == FIRING)
		{
			V2d along = normalize(data.quadOldWirePosB - data.quadOldPosA);
			V2d other = normalize(data.quadOldPosA - data.quadPlayerPosD);

			double alongQ = dot(e->v0 - data.quadOldPosA, along);
			double otherQ = cross(e->v0 - data.quadOldPosA, along);

			double extra = 0;
			//cout << "checking: " << e->v0.x << ", " << e->v0.y << ", along/other: " << alongQ << ", " << otherQ 
			//	<< ", alongLen: " << length( quadOldWirePosB - quadOldPosA ) << ", otherLen: " << length( quadOldPosA - quadPlayerPosD ) << endl;
			if (-otherQ >= -extra  && -otherQ <= length(data.quadOldPosA - data.quadPlayerPosD) + extra)
			{
				if (alongQ >= -extra && alongQ <= length(data.quadOldWirePosB - data.quadOldPosA) + extra)
				{
					if (minSideEdge == NULL
						|| (minSideEdge != NULL
							&& (otherQ < data.minSideOther
								|| (approxEquals(otherQ, data.minSideOther) && alongQ < data.minSideAlong))))
								//|| (otherQ == minSideOther && alongQ < minSideAlong))))
					{
						//the reason for the approxEquals is when you are on a flat edge and you run off
						//and throw the wire sideways, if you dont use approxEquals it can potentially
						//latch on to the wrong end of the edge if you use == on a double, even if
						//the values are incredibly similar down to like 10 decimal places
						data.minSideOther = otherQ;
						data.minSideAlong = alongQ;
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

		V2d touchPoint = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);
		if (g->grassType == Grass::ANTIWIRE && g->IsTouchingCircle( touchPoint, grassCheckRadius ))
		{
			data.antiWireGrassCount++;
		}
	}
}

void Wire::CheckAntiWireGrass()
{
	//if ( player->owner != NULL && player->owner->hasGrass[Grass::ANTIWIRE])
	{
		V2d hitPoint = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);
		sf::Rect<double> r;
		r.left = hitPoint.x - grassCheckRadius / 2;
		r.top = hitPoint.y - grassCheckRadius / 2;
		r.width = grassCheckRadius;
		r.height = grassCheckRadius;
		data.antiWireGrassCount = 0;
		queryMode = "grass";
		player->sess->grassTree->Query(this, r);
	}
}
//make multiples of the quads for each edge later
void Wire::UpdateQuads()
{
	V2d playerPos;
	if(data.state == RETRACTING )
	{
		playerPos = data.retractPlayerPos;
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
	bool hitOrPulling = (data.state == HIT || data.state == PULLING || data.state == RETRACTING );
	bool singleRope = ( hitOrPulling && data.numPoints == 0 );
	int cap = 0;

	int currNumPoints = data.numPoints;

	if(data.state == FIRING || singleRope || (data.state == HIT || data.state == PULLING ) || data.state == RETRACTING || data.state == HITENEMY )
	{
		if(data.state == RETRACTING )
		{
			if( true )
			{
				currNumPoints = data.fusePointIndex;
				//cap++;
			}
			else
			{
				cap = data.numPoints - data.fusePointIndex;
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
				if(data.numPoints == 0 )
				{
					currWirePos = data.anchor.pos;
					if(data.state == RETRACTING )
					{
						if( true )
						{
							V2d start = data.anchor.pos;//points[pointI-1].pos;
							V2d end = data.retractPlayerPos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * data.fuseQuantity;
						}
						else
						{
							currWirePos = data.retractPlayerPos;
							V2d start = data.retractPlayerPos;//points[pointI-1].pos;
							V2d end = data.anchor.pos;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * data.fuseQuantity;
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
					currWirePos = data.anchor.pos;
					if(data.state == RETRACTING && data.fusePointIndex == 0 )//( (right && fusePointIndex == 0) || ( !right ) ) )
					{
						if( true )
						{
							V2d start = data.anchor.pos;//points[0].pos;//points[pointI-1].pos;
							V2d end = data.points[pointI].pos;
						
							V2d dir = normalize( end - start );
							currWireStart = start + dir * data.fuseQuantity;
						}
						else
						{
							V2d start;
							V2d end;
							if( cap == 0 )
							{
								start = data.points[0].pos;
								end = data.anchor.pos;
							}
							else if( cap == data.numPoints )
							{
								start = data.retractPlayerPos;
								end = data.points[cap-1].pos;
							}
							else
							{
								start = data.points[cap].pos;;
								end = data.points[cap-1].pos;
							}

							V2d dir = normalize( end - start );

							currWireStart = start;

							currWirePos = start + dir * data.fuseQuantity;
						}
					}
					else
					{
						currWireStart = data.points[0].pos;//playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
			}
			else
			{
				if( pointI == currNumPoints )
				{		
					currWirePos = data.points[pointI-1].pos;
					
					if(data.state == RETRACTING )
					{
						currWirePos = data.points[0].pos;
						if( true )
						{
							V2d start = data.points[pointI-1].pos;
							V2d end;
							if( pointI == data.numPoints )
							{
								end = data.retractPlayerPos;
							}
							else
							{
								end = data.points[pointI].pos;
							}
							currWirePos = start;
							V2d dir = normalize( end - start );
							currWireStart = start + dir * data.fuseQuantity;
						}
						else
						{
							V2d start = data.retractPlayerPos;
							V2d end = data.points[data.numPoints-1].pos;

							V2d dir = normalize( end - start );
							currWirePos = end;
							currWireStart = start;
						}
					}
					else
					{
						currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
					}
				}
				else
				{
					currWirePos = data.points[pointI - 1].pos;
					currWireStart = data.points[pointI].pos;
				}
			}
			alongDir = normalize( currWirePos - currWireStart );
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
		}
		else if(data.state == FIRING )
		{
			alongDir = data.fireDir;
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
			currWirePos = playerPos + data.fireDir * fireRate * (double)(data.framesFiring+1);
			currWireStart = playerPos + V2d( player->GetWireOffset().x, player->GetWireOffset().y );
		}
		else if (data.state == HITENEMY)
		{
			alongDir = data.fireDir;
			otherDir = alongDir;
			temp = otherDir.x;
			otherDir.x = otherDir.y;
			otherDir.y = -temp;
			currWirePos = playerPos + data.hitEnemyDelta;//playerPos + fireDir * fireRate * (double)(framesFiring + 1);
			currWireStart = playerPos;// +V2d(player->GetWireOffset().x, player->GetWireOffset().y);
		}
		
		if (pointI == 0)
		{
			wireTip.setPosition(Vector2f(currWirePos));
			//float angle = atan2(alongDir.y, alongDir.x);
			//wireTip.setRotation((angle / PI) * 180 + 90);
		}

		double wireLength = length(currWirePos - currWireStart);
		data.firingTakingUp = ceil(wireLength / tileHeight );

		V2d startBack = currWireStart - otherDir * quadHalfWidth;
		V2d startFront = currWireStart + otherDir * quadHalfWidth;
		V2d endBack = currWirePos - otherDir * quadHalfWidth;
		V2d endFront = currWirePos + otherDir * quadHalfWidth;

		quads[startIndex * 4].position = Vector2f(startBack.x, startBack.y);
		quads[startIndex * 4 + 1].position = Vector2f(startFront.x, startFront.y);
		quads[startIndex * 4 + 2].position = Vector2f(endFront.x, endFront.y);
		quads[startIndex * 4 + 3].position = Vector2f(endBack.x, endBack.y);

		float width = ts_wire->texture->getSize().x;
		float height = ts_wire->texture->getSize().y;

		if (pointI >= 0)
		{
			SetRectCenter(nodeQuads + startIndex * 4, ts_wireNode->tileWidth,
				ts_wireNode->tileHeight, Vector2f(currWireStart));
			int nodeTile = 0;
			if (!right)
			{
				nodeTile = 1;
			}
			SetRectSubRect(nodeQuads + startIndex * 4, ts_wireNode->GetSubRect(nodeTile));
		}
		

		IntRect ir(0, 0, 16, 16 * 16);

		if (!right)
		{
			ir.top += 16 * 16;
		}

		wireShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
			(ir.left + ir.width) / width, (ir.top + ir.height) / height));

		SetRectSubRect(quads + startIndex * 4, sf::FloatRect(0, 0, 1.0, (wireLength / ir.height)/2.0));

		//SetRectColor(quads + startIndex * 4, Color::Red);

		double miniExtraWidth = 20;
		startBack -= otherDir * miniExtraWidth;
		startFront += otherDir * miniExtraWidth;
		endBack -= otherDir * miniExtraWidth;
		endFront += otherDir * miniExtraWidth;
		minimapQuads[startIndex * 4].position = Vector2f(startBack.x, startBack.y);
		minimapQuads[startIndex * 4 + 1].position = Vector2f(startFront.x, startFront.y);
		minimapQuads[startIndex * 4 + 2].position = Vector2f(endFront.x, endFront.y);
		minimapQuads[startIndex * 4 + 3].position = Vector2f(endBack.x, endBack.y);

		Color miniColor;
		if (right)
		{
			miniColor = Color::Red;

		}
		else
		{
			miniColor = Color(0, 100, 255);
		}

		SetRectColor(minimapQuads + startIndex * 4, miniColor);

		startIndex++;

		}

		data.numVisibleIndexes = startIndex;//startIndex - 1;

		if (data.numVisibleIndexes * 4 > numQuadVertices)
		{
			//this is also the stop-glitch method
			//which I'll need to go over later
			data.numVisibleIndexes = numQuadVertices / 4;
		}

		if(data.state == FIRING )
			++data.framesFiring;
	}
}

double Wire::GetSegmentLength()
{
	V2d playerPos = data.storedPlayerPos;//player->position;
	double segLength;
	if(data.numPoints == 0 )
	{
		segLength = length(data.anchor.pos - playerPos );
	}
	else
	{
		segLength = length(data.points[data.numPoints-1].pos - playerPos );
	}

	return segLength;
}

void Wire::Retract()
{
	if (data.state == HIT || data.state == PULLING)
	{
		data.state = RETRACTING;

	data.retractPlayerPos = data.storedPlayerPos;
	//cout << "beginning retracting" << endl;
					
	data.fusePointIndex = data.numPoints;
	if(data.numPoints == 0 )
	{
		data.fuseQuantity = length(data.anchor.pos - data.retractPlayerPos );
	}
	else
	{
		data.fuseQuantity = length(data.retractPlayerPos - data.points[data.numPoints - 1].pos);
	}
	}
	else if (data.state == HITENEMY)
	{
		//state = RETRACTING;
		//retractPlayerPos = storedPlayerPos;
	}
}

void Wire::Draw( RenderTarget *target )
{
	//return;
	/*if( player->owner != NULL && player->owner->showDebugDraw )
	{
		CircleShape *cstest = new CircleShape;
		cstest->setRadius( 10 );
		cstest->setFillColor( Color::Green );
		cstest->setOrigin( cstest->getLocalBounds().width / 2, cstest->getLocalBounds().height / 2 );
		cstest->setPosition( Vector2f( player->position.x, player->position.y ) );
		progressDraw.push_back( cstest );
	}*/

if (data.state == FIRING || data.state == HIT || data.state == PULLING || data.state == RETRACTING || data.state == HITENEMY)
{
	target->draw(quads, data.numVisibleIndexes * 4, sf::Quads, &wireShader );//, ts_wire->texture);
	target->draw(wireTip);
}

if (data.state == HIT || data.state == PULLING || data.state == RETRACTING)
{
	/*CircleShape cs1;
	cs1.setFillColor(Color::Red);
	cs1.setRadius(8);
	cs1.setOrigin(cs1.getLocalBounds().width / 2, cs1.getLocalBounds().height / 2);
	cs1.setPosition(anchor.pos.x, anchor.pos.y);

	target->draw(cs1);*/

	//if (numPoints > 0)
	{
		target->draw(nodeQuads, data.numVisibleIndexes * 4, sf::Quads, ts_wireNode->texture);
	}
	
	for (int i = 0; i < data.numPoints; ++i)
	{
		/*CircleShape cs;
		if (right)
		{
			cs.setFillColor(Color::Red);
		}
		else
		{
			cs.setFillColor(Color::Cyan);
		}
		
		cs.setRadius(8);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setPosition(points[i].pos.x, points[i].pos.y);*/

		//target->draw(cs);
		
	}
}

}

void Wire::DrawMinimap(sf::RenderTarget *target)
{
	if (data.state == FIRING || data.state == HIT || data.state == PULLING || data.state == RETRACTING)
	{
		target->draw(minimapQuads, data.numVisibleIndexes * 4, sf::Quads);
	}
}

void Wire::DebugDraw(RenderTarget *target)
{
	/*for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
		delete (*it);
	}
	progressDraw.clear();*/

	if (data.state == FIRING)
	{
		data.movingHitbox.DebugDraw(CollisionBox::Hit, target);
	}
}

void Wire::ClearDebug()
{
	for (list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it)
	{
		delete (*it);
	}
	progressDraw.clear();
}

void Wire::SortNewPoints()
{
	//insertion sort just for easy testing
	if (data.newWirePoints > 1)
	{
		int closestIndex;
		for (int i = data.numPoints - data.newWirePoints; i < data.numPoints; ++i)
		{
			closestIndex = i;
			for (int j = i+1; j < data.numPoints; ++j)
			{
				if (abs(data.points[j].sortingAngleDist - data.points[closestIndex].sortingAngleDist) < .01)
				{
					if (length(data.points[j].pos - data.realAnchor) < length(data.points[closestIndex].pos - data.realAnchor))
					{
						closestIndex = j;
					}
				}
				else if (data.points[j].sortingAngleDist < data.points[closestIndex].sortingAngleDist)
				{
					closestIndex = j;
				}

				
				/*else if (points[j].sortingAngleDist == points[closestIndex].sortingAngleDist)
				{
					
				}*/
			}

			if (closestIndex != i)
			{
				SwapPoints(i, closestIndex);
			}
		}

		/*for( int i = numPoints - newWirePoints; i < numPoints; ++i )
		{
			int j = i;

			while( j > 0 && ( (points[j-1].sortingAngleDist > points[j].sortingAngleDist) 
				|| ( points[j-1].sortingAngleDist == points[j].sortingAngleDist && length( points[j-1].pos - realAnchor )
					> length( points[j].pos - realAnchor ) ) ) )
			{
				SwapPoints( j-1, j );
				--j;
			}
		}*/
	}
}

double Wire::GetCurrentTotalLength()
{
	V2d playerPos = data.storedPlayerPos;//player->position;
	double total = 0;
			
	if(data.numPoints > 0 )
	{
		total += length(data.points[0].pos - data.anchor.pos );
		for( int i = 1; i < data.numPoints; ++i )
		{
			total += length(data.points[i].pos - data.points[i-1].pos );
		}
		total += length(data.points[data.numPoints-1].pos - playerPos );
	}
	else
	{
		total += length(data.anchor.pos - playerPos );
	}

	return total;
}

void Wire::Reset()
{
	data.state = IDLE;
	data.numPoints = 0;
	data.framesFiring = 0;
	data.antiWireGrassCount = 0;
	data.frame = 0;
	data.pullStrength = startPullStrength;
	data.anchor.Reset();
	data.shaderOffset = 0;
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
	data.offset = player->GetWireOffset();

	if( player->facingRight )
	{
		data.offset.x = -abs(data.offset.x );
	}
	else
	{
		data.offset.x = abs(data.offset.x );
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
		V2d pp = player->ground->GetPosition( player->edgeQuantity );
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

	playerPos += gNormal * (double)data.offset.y - other * (double)data.offset.x;
	return playerPos;
}

void Wire::HitEnemy(V2d &pos)
{
	data.state = HITENEMY;
	data.hitEnemyFrame = 0;
	data.hitEnemyDelta = pos - GetPlayerPos();
}

void Wire::UpdateFuse()
{
	int currPoints = data.fusePointIndex;
	double momentum = retractSpeed;
	while( !approxEquals( momentum, 0 ) )
	{
		if( true )
		//if( right )
		{
			if(data.fuseQuantity > momentum )
			{
				data.fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - data.fuseQuantity;
				data.fusePointIndex--;
				if(data.fusePointIndex == -1 )
				{
					data.fuseQuantity = 0;
					data.state = RELEASED;
					data.numPoints = 0;
					return;
				}
				else if(data.fusePointIndex == 0 )
				{
					data.fuseQuantity = length(data.points[data.fusePointIndex].pos - data.anchor.pos );
				}
				else
				{
					data.fuseQuantity = length(data.points[data.fusePointIndex].pos - data.points[data.fusePointIndex-1].pos );
				}
			}
		}
		else
		{
			if(data.fuseQuantity > momentum )
			{
				data.fuseQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				momentum = momentum - data.fuseQuantity;
				data.fusePointIndex--;
				if( data.fusePointIndex == -1 )
				{
					data.fuseQuantity = 0;
					data.state = RELEASED;
					data.numPoints = 0;
					return;
				}
				else if(data.fusePointIndex == 0 )
				{
					//cout << "zero" << endl;
					data.fuseQuantity = length(data.points[data.numPoints - 1].pos - data.retractPlayerPos );
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					data.fuseQuantity = length( data.points[data.numPoints-1 - data.fusePointIndex].pos
					- data.points[(data.numPoints-1) - (data.fusePointIndex-1)].pos );
					//cout << "length: " << fuseQuantity << endl;
					
					
				}
			}
		}
	}
}

bool Wire::IsPulling()
{
	return data.state == PULLING;
}

bool Wire::IsRetracting()
{
	return data.state == RETRACTING;
}

void Wire::SetStoredPlayerPos(const V2d &p)
{
	data.storedPlayerPos = p;
}

bool Wire::IsHit()
{
	return data.state == HIT;
}

//int Wire::GetNumStoredBytes()
//{
//	return sizeof(MyData);
//}
//
//void Wire::StoreBytes(unsigned char *bytes)
//{
//	data.rcEdge.SetFromEdge( rayCastInfo.rcEdge );
//	data.rcQuant = rayCastInfo.rcQuant;
//
//	memcpy(bytes, &data, sizeof(data));
//}
//
//void Wire::SetFromBytes(unsigned char *bytes)
//{
//	memcpy(&data, bytes, sizeof(data));
//
//	rayCastInfo.rcEdge = player->sess->GetEdge(&data.rcEdge);
//	rayCastInfo.rcQuant = data.rcQuant;
//}
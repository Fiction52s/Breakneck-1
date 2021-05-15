#include "Camera.h"
#include "VectorMath.h"
#include <iostream>
#include "GameSession.h"
#include "Movement.h"
#include "Enemy.h"
#include "MapHeader.h"
#include "Actor.h"
#include "Barrier.h"

using namespace std;
using namespace sf;

CameraShot::CameraShot( const string &p_name, Vector2f &pos, float z)
{
	name = p_name;
	centerPos = pos;
	zoom = z;
}


void Camera::SetCamType(CamType c)
{
	camType = c;
}

Camera::Camera()
{
	playerIndex = 0;
	sess = NULL;
	zoomFactor = 1;
	zoomOutRate = 1;
	zoomInRate = .1;
	offsetRate = 3;

	minZoom = 1;
	maxZoom = 2.25;

	pos.x = 0;
	pos.y = 0;
	zoomLevel1 = 0;
	zoomLevel2 = .5;
	zoomLevel3 = 1.75;

	maxOffset.x = 100 * 5;//10;
	maxOffset.y = 100 * 5;//10;

	left = 300;
	right = -300;
	top = 150;
	bottom = -150;

	easing = false;
	manual = false;
	rumbling = false;

	offset.x = 0;
	offset.y = 0;

	zoomLevel = 0;
	easeOutFrame = 0;
	easingOut = false;
	currMove = NULL;
	
	rX = 0;
	rY = 0;
	offsetVel = Vector2f(0, 0);

	Reset();
}

void Camera::Reset()
{
	rX = 0;
	rY = 0;
	currMove = NULL;
	easingOut = false;
	zoomLevel = 0;
	easing = false;
	manual = false;
	rumbling = false;
	zoomFactor = 1;
	offset.x = 0;
	offset.y = 0;
	pos.x = 0;
	pos.y = 0;
	oldFramesGrinding = 0;
}

void Camera::EaseOutOfManual( int frames )
{
	SetManual( false );
	//manual = false;
	easingOut = true;
	easeOutCount = frames;
	easeOutFrame = 0;
	//manualPos = pos;
	//manualZoom = zoomFactor;

	startEase = pos;
}

int Camera::GetActiveEnemyCount( Actor *player, double &minX, double &maxX, double &minY, double &maxY )
{
	int numEnemies = 0;
	V2d playerPos = player->position;
	V2d vPos = V2d(pos.x, pos.y);

	//FloatRect fr = GetRect();

	

	double origMinX = minX;
	double origMaxX = maxX;
	double origMinY = minY;
	double origMaxY = maxY;

	//int testRadius = 1200;//900
	double extraCap = 500;//500;
	double extra = 200;
	double extraDist = 200;

	double capMinX = origMinX - extraCap;
	double capMaxX = origMaxX + extraCap;
	double capMinY = origMinY - extraCap;
	double capMaxY = origMaxY + extraCap;

	Enemy *curr = sess->activeEnemyList;
	while (curr != NULL)
	{
		int numPoints = curr->GetNumCamPoints();
		
		V2d currPos;
		for (int i = 0; i < numPoints; ++i)
		{
			currPos = curr->GetCamPoint(i);
			if (currPos.x < capMinX || currPos.x > capMaxX || currPos.y < capMinY || currPos.y > capMaxY)
			{
				//curr = curr->next;
				continue;
			}

			++numEnemies;

			//if (currPos.x < origMinX)
			{
				minX = min(minX, currPos.x - extra);
			}
			//else if (currPos.x > origMaxX)
			{
				maxX = max(maxX, currPos.x + extra);
			}

			//if (currPos.y < origMinY)
			{
				minY = min(minY, currPos.y - extra);
			}
			//else if (currPos.y > origMaxY)
			{
				maxY = max(maxY, currPos.y + extra);
			}
		}

		curr = curr->next;
	}

	return numEnemies;
}

Vector2f Camera::GetNewOffset( V2d &pVel )
{
	float offXAccelOut = .1;
	float offXAccelIn = .1;
	float offYAccelOut = .1;
	float offYAccelIn = .1;

	Vector2f targetOffset;
	double maxXVelRegistered = 30.0;
	Vector2f moveFactor;
	float moveF = 1.0;

	Vector2f tempOffset = offset;

	

	if (pVel.x > 0)
	{
		moveFactor.x = min(1.0, pVel.x / maxXVelRegistered);
		moveFactor.x = 1.f;
		moveFrames.x += moveFactor.x;
		//targetOffset.x = ;
	}
	else if( pVel.x < 0 )
	{
		
		moveFactor.x = max(-1.0, pVel.x / maxXVelRegistered);
		moveFactor.x = -1.f;
		moveFrames.x += moveFactor.x;
	//	targetOffset.x =  );
	}
	else
	{
		float zFactor = 1.f;
		if (moveFrames.x > 0)
		{
			moveFrames.x -= zFactor;
		}
		else if (moveFrames.x < 0)
		{
			moveFrames.x += zFactor;
		}
	}
	//cout << "moveframes.x: " << moveFrames.x << endl;

	double maxYVelRegistered = 30.0;
	if (pVel.y > 0)
	{
		
		moveFactor.y = min(1.0, pVel.y / maxYVelRegistered);
		moveFactor.y = 1.f;
		moveFrames.y += moveFactor.y;
		//targetOffset.y = min(1.0, pVel.y / maxYVelRegistered);
	}
	else if (pVel.y < 0)
	{
		
		moveFactor.y = max(-1.0, pVel.y / maxYVelRegistered);
		moveFactor.y = -1.f;
		moveFrames.y += moveFactor.y;
		//targetOffset.y = max(-1.0, pVel.y / maxYVelRegistered);
	}
	else
	{
		moveFrames.y = 0;
	}

	float maxFramesX = 60;
	float maxFramesY = 60;

	if (moveFrames.x > maxFramesX)
	{
		moveFrames.x = maxFramesX;
	}
	else if (moveFrames.x < -maxFramesX)
	{
		moveFrames.x = -maxFramesX;
	}

	if (moveFrames.y > maxFramesY)
	{
		moveFrames.y = maxFramesY;
	}
	else if (moveFrames.y < -maxFramesY)
	{
		moveFrames.y = -maxFramesY;
	}

	Vector2f maxOff;
	if (moveFrames.x > 0)
	{
		maxOff.x = 100.f + 150.f * (moveFrames.x / maxFramesX);
	}
	else if (moveFrames.x < 0)
	{
		maxOff.x = -100.f + 150.f * (moveFrames.x / maxFramesX);
	}
	else
	{
		maxOff.x = 0;
	}

	if (moveFrames.y > 0)
	{
		maxOff.y = 125.f;// + 150.f * (moveFrames.y / maxFramesX);
	}
	else if (moveFrames.y < 0)
	{
		maxOff.y = -75.f;// +150.f * (moveFrames.y / maxFramesX);
	}
	else
	{
		maxOff.y = 0;
	}
	

	Vector2f moveStuff;
	
	targetOffset = maxOff;// *5.f;


	/*if (moveFrames.x > 0)
	{
		targetOffset.x = moveFrames.x / maxFramesX;
	}
	else if (moveFrames.x < 0)
	{
		targetOffset.x = moveFrames.x / maxFramesX;
	}
	else
	{
		targetOffset.x = 0;
	}

	if (moveFrames.y > 0)
	{
		targetOffset.y = moveFrames.y / maxFramesY;
	}
	else if (moveFrames.y < 0)
	{
		targetOffset.y = moveFrames.y / maxFramesY;
	}
	else
	{
		targetOffset.y = 0;
	}*/
	

	//targetOffset = //moveFactor;
	if (pVel.x > 0)
	{
		targetOffset.x = 1.0;
	}
	else if (pVel.x < 0)
	{
		targetOffset.x = -1.0;
	}
	else
	{
		targetOffset.x = 0;
	}
	if (pVel.y > 0)
	{
		targetOffset.y = 1.0;
	}
	else if (pVel.y < 0)
	{
		targetOffset.y = -1.0;
	}
	else
	{
		targetOffset.y = 0;
	}
	targetOffset.x *= 250.0;
	
	if (targetOffset.y > 0)
	{
		targetOffset.y *= 125.0;//maxOffset.y;
	}
	else
	{
		targetOffset.y *= 75.0;
	}

	
	targetOffset *= GetZoom();//10.f;//zoomFactor * 2 + zoomLevel;//zoomFactor * 2;//GetZoom();
	//cout << "targetOffset: " << targetOffset.x << ", " << targetOffset.y << endl;

	//offset = targetOffset;
	//offset = targetOffset;
	Vector2f maxVel(1.0, 1.0);
	if (targetOffset.x > tempOffset.x)
	{
		offsetVel.x = maxVel.x;
	}
	else
	{
		offsetVel.x = -maxVel.x;
	}

	if (targetOffset.y > tempOffset.y)
	{
		offsetVel.y = maxVel.y;
	}
	else
	{
		offsetVel.y = -maxVel.y;
	}

	Vector2f dir = normalize(targetOffset - tempOffset);
	float speed = 2.0;
	if ((pVel.x > 0 && tempOffset.x < 0) || pVel.x < 0 && tempOffset.x > 0 )
	{
		speed *= 2;
	}

	dir.x *= speed;

	offsetVel = dir;// *speed;
	//offsetVel.x *= abs(dir.x);
	//offsetVel.y *= abs(dir.y);
	tempOffset += offsetVel;//targetOffset;
	Vector2f diff = targetOffset - tempOffset;
	if (abs(diff.x) <= maxVel.x ) 
	{
		tempOffset.x = targetOffset.x;
		offsetVel.x = 0;
	}
	if( abs( diff.y ) <= maxVel.y )
	{
		tempOffset.y = targetOffset.y;
		offsetVel.y = 0;
	}

	return tempOffset;
	//cout << "offset: " << offset.x << " , " << offset.y << endl;
}

double Camera::GetEnemyZoomTarget( Actor *player )
{
	double minX, maxX, minY, maxY;

	minX = pos.x; -480;// *zoomFactor;
	maxX = pos.x + 480;// *zoomFactor;
	minY = pos.y - 270;// *zoomFactor;
	maxY = pos.y + 270;// *zoomFactor;

	double origMinY = minY;

	numActive = GetActiveEnemyCount( player, minX, maxX, minY, maxY );

	FloatRect fr = GetRect();

	if (fr.left < minX)
	{
		minX = fr.left;
	}
	if (fr.left + fr.width > maxX)
	{
		maxX = fr.left + fr.width;
	}

	if (fr.top < minY && origMinY != minY)
	{
		//minY = fr.top;
	}
	if (fr.top + fr.height > maxY)
	{
		maxY = fr.top + fr.height;
	}

	if( numActive > 0 )
	{
		if( framesActive < 60 )
			framesActive++;	
	}
	else
	{
		framesActive--;
		if( framesActive < 0 )
			framesActive = 0;
	}

	//cout << "miny: " << minY << ", maxy: " << maxY << endl;

	double tw = maxX - minX;//max( abs( maxX - pos.x ), abs( minX - pos.x ) ) * 2;
	double th = maxY - minY;//max( abs( maxY - pos.y ), abs( minY - pos.y ) ) * 2;

	double w= tw/ 960.0;
	double h= th/ 540.0;

	double ratio = max( w, h );
	
	ratio = max( ratio, 1.0 );
	testZoom = ratio;

	return testZoom;
}

void Camera::SetRumble( int xFactor, int yFactor, int duration )
{
	assert( duration > 0 );

	rumbleLength = duration;
	rumbleX = xFactor;
	rumbleY = yFactor;
	rumbling = true;
	rumbleFrame = 0;
}

void Camera::StopRumble()
{
	if (!rumbling)
		return;

	rumbling = false;
	if (rX != 0)
	{
		pos.x -= rX;
	}
	if (rY != 0)
	{
		pos.y -= rY;
	}
	rX = 0;
	rY = 0;
}

void Camera::Ease( Vector2f &p_pos, float zFactor, int numFrames, CubicBezier bez)
{
	if (!manual)
		SetManual(true);
	easeFrame = 0;
	numEaseFrames = numFrames;
	startEase = pos;
	endEase = p_pos;
	easeBez = bez;
	startEaseZFactor = zoomFactor;
	endEaseZFactor = zFactor;
	easing = true;
}

//make sure to rumble off when you reset the level
void Camera::UpdateRumble()
{
	if( !rumbling )
		return;

	/*if (rumbleFrame > 0 && !easing )
	{
		pos.x -= rX;
		pos.y -= rY;
	}*/
	
	if( rumbleFrame == rumbleLength )
	{
		rumbling = false;
		rX = 0;
		rY = 0;
		return;
	}
	//this needs to be random?
	int fx = ( rand() % 3 ) - 1;
	int fy = (rand() % 3) - 1;
	
	rX = fx * rumbleX * GetZoom();
	rY = fy * rumbleY * GetZoom();

	//pos += sf::Vector2f( rX, rY );

	++rumbleFrame;
	
}

sf::FloatRect Camera::GetRect()
{
	Vector2f center = GetPos();
	float zoom = GetZoom();
	float width = 960 * zoom;
	float height = 540 * zoom;

	return FloatRect(center.x - width / 2, center.y - height / 2, width, height);
}

void Camera::Set( sf::Vector2f &p, float zFactor, int zLevel )
{
	SetManual(true);

	easing = false;
	/*if (!manual)
	{
		int x = 5;
	}*/
	assert( manual );
	pos.x = p.x;
	pos.y = p.y;

	UpdateRumble();

	zoomFactor = zFactor;
	zoomLevel = zLevel;
	
}

void Camera::UpdateEase()
{
	if (easing)
	{
		if (easeFrame > numEaseFrames)
		{
			easing = false;
			return;
		}

		float a = (float)easeFrame / numEaseFrames;
		float f = easeBez.GetValue(a);
		pos = startEase * (1.f - f) + endEase * f;
		zoomFactor = startEaseZFactor *(1. - f) + endEaseZFactor * f;

		++easeFrame;
	}
}

void Camera::ManualUpdate( Actor *player )
{
	if (manual && currMove != NULL)
	{
		for (int i = 0; i < NUM_MAX_STEPS; ++i)
		{
			currMove->Update();
		}

		if (relativeMoveSeq)
		{
			pos = sequenceStartPos + Vector2f(currMove->position.x, currMove->position.y);
		}
		else
		{
			pos = Vector2f(currMove->position.x, currMove->position.y);
		}

		if (currMove->currMovement == NULL)
			currMove = NULL;
	}

	UpdateRumble();

	UpdateEase();



	float xChangePos = 0, xChangeNeg = 0, yChangePos = 0, yChangeNeg = 0;

	//cout << "manual pos: " << pos.x << ", " << pos.y << "z: " << zoomFactor << endl;
	//UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);
}

void Camera::UpdateZoomLevel( ControllerState &con, ControllerState &prevcon )
{
	if (con.PUp() && !prevcon.PUp())
	{
		if (zoomLevel > 0)
		{
			zoomLevel -= .5;
			if (zoomLevel < 0)
				zoomLevel = 0;
		}
		else
		{
			zoomLevel -= .1;
			if (zoomLevel < -2)
				zoomLevel = -2;
		}
		
		/*if (zoomLevel < -1)
			zoomLevel = -1;*/
	}
	else if (con.PDown() && !prevcon.PDown())
	{
		if (zoomLevel < 0)
		{
			zoomLevel += .1;
		}
		else
		{
			zoomLevel += .5;
			if (zoomLevel > 3)
				zoomLevel = 3;
		}
	}
	else if (con.PLeft() || con.PRight())
	{
		zoomLevel = 0;
	}
}

sf::Vector2<double> Camera::GetPlayerVel( Actor *player)
{
	V2d pVel;
	if (player->grindEdge != NULL && (player->action == Actor::GRINDBALL))
	{
		//cout << "here!" << endl;
		V2d grindDir = normalize(player->grindEdge->v1 - player->grindEdge->v0);

		pVel = grindDir * player->grindSpeed;

		if (player->reversed)
		{
			pVel = -pVel;
		}
	}
	else if (player->ground != NULL)
	{
		if (player->action != Actor::JUMPSQUAT)
		{
			pVel = normalize(player->ground->v1 - player->ground->v0) * player->groundSpeed;
		}
		else
		{
			pVel = normalize(player->ground->v1 - player->ground->v0) * player->storedGroundSpeed;
		}

		if (player->reversed)
		{
			pVel = -pVel;
		}
	}
	else
	{
		pVel = player->velocity;
	}

	pVel.x /= player->slowMultiple;
	pVel.y /= player->slowMultiple;

	//pVel = V2d(60, -60);

	return pVel;
}

double Camera::GetMovementZoomTarget( Actor *player )
{
	double zFactor = zoomFactor;
	double temp;
	V2d f;
	double kk = 18.0;
	if (player->ground != NULL)
	{
		temp = abs(player->groundSpeed) / kk;
		f = normalize(player->ground->v1 - player->ground->v0) * player->groundSpeed * 10.0;
		if (player->reversed)
			f = -f;

	}
	else
	{
		temp = length(player->velocity) / kk;
		f = player->velocity * 10.0;
	}

	return max( 1.0, temp );
}

void Camera::SetMovementOffset( V2d &pVel )
{
	double offXAccel;
	double offX = pVel.x * .8;//1.0;
	double offXMax = 10;//10;

	if (offX > offXMax)
	{
		offX = offXMax;
	}
	else if (offX < -offXMax)
	{
		offX = -offXMax;
	}
	offset.x += offX;

	double offY = pVel.y * .8;//1.0;
	double offYMax = 10;//10;

	if (offY > offYMax)
	{
		offY = offYMax;
	}
	else if (offY < -offYMax)
	{
		offY = -offYMax;
	}

	offset.y += offY;

	if (pVel.x == 0)
	{
		offset.x += (-offset.x) / 40;
	}
	if (pVel.y == 0)
	{
		offset.y += (-offset.y) / 20;
	}


	int xLimit = 250;
	if (offset.x < -xLimit * zoomFactor)
		offset.x = -xLimit * zoomFactor;
	else if (offset.x > xLimit * zoomFactor)
		offset.x = xLimit * zoomFactor;

	//25
	if (offset.y < -75 * zoomFactor)
		offset.y = -75 * zoomFactor;
	else if (offset.y > 125 * zoomFactor)
		offset.y = 125 * zoomFactor;
}

void Camera::UpdateBarrier( Actor *player, float &xChangePos, float &xChangeNeg, float &yChangePos, float &yChangeNeg )
{
	Vector2f truePos = pos;// +Vector2f(rX, rY);

	xChangePos = 0;
	xChangeNeg = 0;
	yChangePos = 0;
	yChangeNeg = 0;

	int topBoundsCamExtra = 50;
	int bottomBoundsCamExtra = 0;
	int topBounds = sess->mapHeader->topBounds + topBoundsCamExtra;

	float halfw = 960 / 2;
	float halfh = 540 / 2;

	float left = truePos.x - halfw * GetZoom();

	float tdiff = sess->mapHeader->leftBounds - left;
	if (tdiff > 0)
	{
		xChangePos = tdiff;
	}
	float right = truePos.x + halfw * GetZoom();

	tdiff = right - (sess->mapHeader->leftBounds + sess->mapHeader->boundsWidth);
	if (tdiff > 0)
	{
		xChangeNeg = -tdiff;
	}
	float top = truePos.y - halfh * GetZoom();
	tdiff = topBounds - top;
	if (tdiff > 0)
	{
		yChangePos = tdiff;
	}

	int botBounds = sess->mapHeader->topBounds + sess->mapHeader->boundsHeight + bottomBoundsCamExtra;
	float bot = truePos.y + halfh * GetZoom();
	tdiff = bot - botBounds;
	if (tdiff > 0)
	{
		yChangeNeg = -tdiff;
	}

	Barrier *leftb = NULL, *rightb = NULL, *topb = NULL, *botb = NULL;
	
	for (list<Barrier*>::iterator it = sess->barriers.begin(); it != sess->barriers.end(); ++it)
	{
		//continue;
		if ((*it)->triggered)
		{
			continue;
		}


		if ((*it)->x)
		{
			if ((*it)->positiveOpen)
			{
				float diff = (*it)->GetCamPos() - left;
				if (diff > 0)
				{
					if (diff > xChangePos)
					{
						xChangePos = diff;
						leftb = (*it);
					}
					//pos.x += diff;
					//cout << "moving right" << endl;
					//offset.x += (*it)->pos - left;
				}
			}
			else
			{
				float diff = right - (*it)->GetCamPos();
				if (diff > 0)
				{
					//cout << "moving left: " << diff << endl;
					//pos.x -= right - (*it)->pos;
					//offset.x -= diff;
					if (-diff < xChangeNeg)
					{
						xChangeNeg = -diff;
						rightb = (*it);
					}
					//pos.x -= diff;
				}
			}
		}
		else
		{
			if ((*it)->positiveOpen)
			{
				float diff = (*it)->GetCamPos() - top;
				if (diff > 0)
				{
					if (diff > yChangePos)
					{
						yChangePos = diff;
						topb = (*it);
					}
					//offset.y += (*it)->pos - top;
					//pos.y += diff;
				}
			}
			else
			{
				float diff = bottom - (*it)->GetCamPos();
				if (diff > 0)
				{
					if (-diff < yChangeNeg)
					{
						yChangeNeg = -diff;
						botb = (*it);
					}
					//pos.y -= diff;
					//offset.y -= bot - (*it)->pos;
				}
			}
		}
	}
	

	Vector2f barrierOffset;
	Vector2f borderOffset;

	if (xChangePos != 0 && xChangeNeg != 0)
	{
		//float diff = rightb->pos - leftb->pos;//xChangePos - xChangeNeg;
		//barrierZoom = diff / 960.f + .5f;
		////cout << "diff: " << diff << endl;
		////cout << "bzoom: " << barrierZoom << endl;
		//pos.x = (rightb->pos + leftb->pos) / 2.f;
		//barrierOffset.x = ( xChangePos + xChangeNeg ) / 2.f;
	}
	else if (xChangePos != 0)
	{
		borderOffset.x = xChangePos;
	}
	else if (xChangeNeg != 0)
	{
		borderOffset.x = xChangeNeg;
	}

	if (yChangePos != 0 && yChangeNeg != 0)
	{
		//barrierOffset.y = ( yChangePos + yChangeNeg ) / 2.f;
	}
	else if (yChangePos != 0)
	{
		borderOffset.y = yChangePos;
	}
	else if (yChangeNeg != 0)
	{
		borderOffset.y = yChangeNeg;
	}

	pos += borderOffset;

	float barrierZoom = -1;
	if (leftb != NULL && rightb != NULL )
	{
		if (xChangePos != 0 && xChangeNeg != 0)
		{
			float diff = rightb->pos - leftb->pos;//xChangePos - xChangeNeg;
			barrierZoom = diff / 960.f + .5f;
			//cout << "diff: " << diff << endl;
			//cout << "bzoom: " << barrierZoom << endl;
			pos.x = (rightb->pos + leftb->pos) / 2.f;
			//barrierOffset.x = ( xChangePos + xChangeNeg ) / 2.f;
		}
		else if (xChangePos != 0)
		{
			barrierOffset.x = xChangePos;
		}
		else if (xChangeNeg != 0)
		{
			barrierOffset.x = xChangeNeg;
		}
	}


	if (topb != NULL & botb != NULL)
	{
		if (yChangePos != 0 && yChangeNeg != 0)
		{
			//barrierOffset.y = ( yChangePos + yChangeNeg ) / 2.f;
		}
		else if (yChangePos != 0)
		{
			barrierOffset.y = yChangePos;
		}
		else if (yChangeNeg != 0)
		{
			barrierOffset.y = yChangeNeg;
		}
	}

	pos += barrierOffset;

	if (barrierZoom > 0)
	{
		zoomFactor = barrierZoom;
	}


}

void Camera::UpdateBossFight(int bossFightType)
{
	assert(bossFightType > 0);

	if (bossFightType == 1)
	{
		//PoiInfo *pi = owner->poiMap["crawlercam"];
		//if (pi == NULL)
		//{
		//	assert(0);
		//	return;
		//}
		//else
		//{
		//	pos.x = pi->pos.x;//owner->mh->leftBounds + owner->mh->boundsWidth / 2.f;
		//	pos.y = pi->pos.y;//owner->mh->topBounds + owner->mh->boundsHeight / 2.f;
		//	zoomFactor = 1.75f;
		//}
		
	}
}

void Camera::Init(V2d &p_pos)
{
	if (sess == NULL)
	{
		sess = Session::GetSession();
		if (sess->IsSessTypeGame())
		{
			game = GameSession::GetSession();
		}
		else
		{
			game = NULL;
		}
	}

	Reset();
	pos.x = p_pos.x;
	pos.y = p_pos.y;
}

void Camera::UpdateBasicMode()
{
	Actor *player = sess->GetPlayer(playerIndex);

	slowMultiple = player->slowMultiple;

	if (manual)
	{
		ManualUpdate(player);
		return;
	}

	float xChangePos = 0, xChangeNeg = 0, yChangePos = 0, yChangeNeg = 0;

	V2d playerPos = player->position;

	if (player->action == Actor::RIDESHIP || player->action == Actor::SKYDIVE)
	{
		zoomFactor = 1;
		pos.x = playerPos.x;
		pos.y = playerPos.y;
		return;
	}

	if (sess->pauseFrames > 0)
	{
		pos.x = playerPos.x + offset.x;
		pos.y = playerPos.y + offset.y;
	}


	double grindFramesCap = 30;
	if (player->grindEdge != NULL && (player->action == Actor::GRINDBALL))
	{
		V2d grindNorm = player->grindEdge->Normal();


		if (oldFramesNotGrinding < grindFramesCap && player->framesGrinding == 1)
		{
			//cout << "bad trigger" << endl;
			player->framesGrinding = grindFramesCap - oldFramesNotGrinding;
			//cap2 = player->framesGrinding;
		}

		if (player->framesGrinding < grindFramesCap)
		{
			playerPos += grindNorm * player->normalHeight * (1 - player->framesGrinding / grindFramesCap);
			//cout << "height on: " << playerPos.y << endl;
		}
	}
	else if (player->ground != NULL)
	{
		if (oldFramesGrinding < grindFramesCap && player->framesNotGrinding == 1)
		{
			//cout << "bad trigger" << endl;
			player->framesNotGrinding = grindFramesCap - oldFramesGrinding;
			//cap2 = player->framesGrinding;
		}

		if (player->framesNotGrinding < grindFramesCap)
		{
			V2d oppositeGroundNormal;
			if (player->ground != NULL)
			{
				oppositeGroundNormal = -player->ground->Normal();

				V2d toffset = oppositeGroundNormal * player->normalHeight * (1 - player->framesNotGrinding / grindFramesCap);
				playerPos += toffset;
				//cout << "height off: " << playerPos.y << endl;
				//cout << "offset: " << offset.x << ", " << offset.y << ", framesNotGrinding: " << player->framesNotGrinding << endl;
			}
		}
	}

	oldFramesGrinding = player->framesGrinding;
	oldFramesNotGrinding = player->framesNotGrinding;

	ControllerState & con = player->currInput;
	ControllerState & prevcon = player->prevInput;
	//UpdateZoomLevel(con, prevcon);

	V2d pVel = GetPlayerVel(player);

	if (pVel.y > 0)
	{
		if (framesFalling < 60)
			framesFalling++;
	}
	else
	{
		if (framesFalling > 0)
			framesFalling--;
	}

	pos.x = playerPos.x;
	pos.y = playerPos.y;

	Vector2f currOffset;

	if (game != NULL && game->debugScreenRecorder != NULL)
	{
		offset.x = 0;
		offset.y = 0;
		zoomFactor = 1;
	}
	else
	{
		currOffset = GetNewOffset(pVel);
	}

	double moveZoom = GetMovementZoomTarget(player);

	double oldZoomFactor = zoomFactor;

	double nextMovementZoom = GetNextMovementZoom(moveZoom);

	zoomFactor = nextMovementZoom;

	pos += currOffset;// *GetZoom();
	UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);


	Vector2f oldOffset = offset;
	offset = currOffset;
	double enemyZoom = GetEnemyZoomTarget(player);
	offset = oldOffset;

	//if( numActive > 0 )
	if (enemyZoom > zoomFactor && numActive > 0)
	{
		double nextEnemyZoom = GetNextEnemyZoom(enemyZoom);
		zoomFactor = nextEnemyZoom;


		//UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);
		/*zoomFactor = oldZoomFactor;
		double nextEnemyZoom = GetNextEnemyZoom(enemyZoom);
		if (nextEnemyZoom > nextMovementZoom)
		{
		zoomFactor = nextEnemyZoom;
		}
		else
		{
		zoomFactor = nextMovementZoom;
		}*/

		pos.x = playerPos.x;
		pos.y = playerPos.y;

		pos += currOffset;

		UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);

		//cout << "zooming out for enemy!: " << enemyZoom << " ";
	}
	else
	{
		//cout << "enemy zoom: " << enemyZoom << " ";
	}

	//cout << "num enemies : " << numActive << endl;

	offset = currOffset;

	if (false)
	{

		cout << "numactive: " << numActive << endl;
		cout << "enemyZoom: " << enemyZoom << endl;
		cout << "zfactor: " << moveZoom << endl;
		if (enemyZoom > moveZoom)
		{

		}
		else if (enemyZoom <= moveZoom)
		{
			zoomFactor = GetNextMovementZoom(moveZoom);
		}

		if (zoomFactor < 1)
			zoomFactor = 1;
		else if (zoomFactor > maxZoom)
			zoomFactor = maxZoom;

	}





	UpdateEaseOut();

	UpdateRumble();

	//cout << "offset: " << offset.x << ", " << offset.y << "\n";
	//cout << "zoomfactor: " << zoomFactor << "\n";

	/*xChangePos = 0;
	xChangeNeg = 0;
	yChangePos = 0;
	yChangeNeg = 0;*/
	//UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);

	//cout << GetPos().x << ", " << GetPos().y << endl;
	//cout << "zoom: " << GetZoom() << endl;
}

void Camera::UpdateFightingMode()
{
	UpdateVS(sess->GetPlayer(0), sess->GetPlayer(1));
}

void Camera::Update()
{
	switch (camType)
	{
	case BASIC:
	{
		UpdateBasicMode();
		break;
	}
	case FIGHTING:
	{
		UpdateFightingMode();
		break;
	}

	}
}

void Camera::UpdateEaseOut()
{
	if (easingOut)
	{
		double r = (double)easeOutFrame / easeOutCount;
		sf::Vector2<double> p;
		p = r * sf::Vector2<double>(pos.x, pos.y) + (1.0 - r) * sf::Vector2<double>(manualPos.x, manualPos.y);
		//cout << "p: " << p.x << ", " << p.y << ", pos: " << pos.x << ", " << pos.y << endl;
		pos = Vector2f(p.x, p.y);
		zoomFactor = r * zoomFactor + (1 - r) * manualZoom;
		//cout << "pos: " << pos.x << ", " << pos.y << ", z: " 
		//	<< zoomFactor << ", easeframe: " << easeOutFrame << endl;
		++easeOutFrame;
		if (easeOutFrame == easeOutCount)
		{
			easingOut = false;

		}
	}
}

void Camera::UpdateVS( Actor *a, Actor *a2 )
{

	float targetZoomFactor = zoomFactor;

	V2d center = (a->position + a2->position) / 2.0;

	Vector2f res( 1920 / 2, 1080 / 2 );

	float xExtra = 600;
	float yExtra = 400;

	double distx = abs( a->position.x - a2->position.x ) + xExtra * 2;
	double disty = abs(a->position.y - a2->position.y) + yExtra * 2;

	double zx = distx / res.x;
	double zy = disty / res.y;
	
	double maxDist = max( zx, zy );
	
	if( zx == maxDist )
	{
		targetZoomFactor = zx;// / 2.0;

		//double adistx = distx + max(xExtra * targetZoomFactor, xExtra);
		double azx = distx / res.x;
		targetZoomFactor = azx;
	}
	else if( zy == maxDist )
	{
		targetZoomFactor = zy;// / 2.0;

		//double adisty = disty + max( yExtra * targetZoomFactor, yExtra);
		double azy = disty / res.y;
		targetZoomFactor = azy;
	}
	
	

	//double adistx = distx + 200;
	//double adisty = disty + 200;

	
	//double azy = adisty / res.y;
	
	

	
	//zoomFactor += 200 / res.x;

	double minZoom = 1.0;//1.0;
	double maxZoom = 8;
	if( targetZoomFactor < minZoom )
		targetZoomFactor = minZoom;
	else if( targetZoomFactor > maxZoom )
		targetZoomFactor = maxZoom;

	float rz = .25;
	zoomFactor = targetZoomFactor * rz + zoomFactor * (1-rz);
	
	float r = .25;
	pos.x = center.x * r + pos.x * (1-r) ;
	pos.y = center.y * r + pos.y * (1-r);

	//cout << "zoomfactor: " << zoomFactor << endl;
}

void Camera::SetManual( bool man )
{
	manual = man;
	currMove = NULL;
	if( man )
	{ 
		startManualPos = pos;
		startManualZoom = zoomFactor;
	}
	else
	{
		manualPos = pos;
		manualZoom = zoomFactor;
	}
}

float Camera::GetZoom()
{
	float z = zoomLevel + zoomFactor;
	if (z < .1)
		z = .1;
	return z;
}

sf::Vector2f Camera::GetPos()
{
	return pos + Vector2f( rX, rY );
}

void Camera::SetMovementSeq( MovementSequence *move, bool relative )
{
	currMove = move;
	relativeMoveSeq = relative;
	sequenceStartPos = pos;
	sequenceStartZoom = GetZoom();
}


double Camera::GetNextEnemyZoom(double enemyZoom)
{
	double tempZoomFactor = zoomFactor;
	double blah = .01;
	if (numActive > 0)
	{
		//cout << "zoomin out: " << enemyZoom - tempZoomFactor << endl;
		if (enemyZoom > tempZoomFactor)
		{
			tempZoomFactor += blah;
			if (tempZoomFactor > enemyZoom)
				tempZoomFactor = enemyZoom;
		}
	}

	return tempZoomFactor;
}

double Camera::GetNextMovementZoom( double moveZoom )
{
	double tempZoomFactor = zoomFactor;
	double zDiff = moveZoom - tempZoomFactor;
	double changeThresh = .001;
	double change;
	if (zDiff > 0)
	{
		change = zDiff / 100.0;
		if (change < changeThresh)
		{
			change = changeThresh;
		}
		if (zDiff < changeThresh)
		{
			tempZoomFactor = moveZoom;
		}
		else
		{
			tempZoomFactor += change / slowMultiple;
		}
	}
	else if (zDiff < 0)
	{
		double oldZoom = tempZoomFactor;
		//bool more = oldZoom > testZoom;
		//if (numActive == 0 )// || more)
		{
			change = zDiff / 200.0;
			if (-change < changeThresh)
			{
				change = -changeThresh;
			}

			if (-zDiff < changeThresh)
			{
				tempZoomFactor = moveZoom;
			}
			else
			{
				tempZoomFactor += change / slowMultiple;
			}
		}
	}

	if (tempZoomFactor < 1)
		tempZoomFactor = 1;
	else if (tempZoomFactor > maxZoom)
		tempZoomFactor = maxZoom;

	return tempZoomFactor;
}
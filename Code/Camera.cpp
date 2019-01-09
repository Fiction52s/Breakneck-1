#include "Camera.h"
#include "VectorMath.h"
#include <iostream>
#include "GameSession.h"
#include "Movement.h"
#include "Enemy.h"

using namespace std;
using namespace sf;


Camera::Camera()
{
	owner = NULL;
	easing = false;
	manual = false;
	//bossCrawler = false;
	rumbling = false;

	offset.x = 0;
	offset.y = 0;

	pos.x = 0;
	pos.y = 0;

	zoomFactor = 1;
	zoomOutRate = 1;
	zoomInRate = .1;
	offsetRate = 3;
	maxZoom = 2.25;//3.5;

	zoomLevel1 = 0;
	zoomLevel2 = .5;
	zoomLevel3 = 1.75;

	minZoom = 1;
	maxOffset.x = 100 * 5;//10;
	maxOffset.y = 100 * 5;//10;

	left = 300;
	right = -300;
	top = 150;
	bottom = -150;

	zoomLevel = 0;
	
	easeOutFrame = 0;
	easingOut = false;

	currMove = NULL;

	rX = 0;
	rY = 0;
	offsetVel = Vector2f(0, 0);
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
}

void Camera::EaseOutOfManual( int frames )
{
	SetManual( false );
	//manual = false;
	easingOut = true;
	easeOutCount = frames;
	easeOutFrame = 0;
	manualPos = pos;
}

int Camera::GetActiveEnemyCount( Actor *player, double &minX, double &maxX, double &minY, double &maxY )
{
	int numEnemies = 0;
	V2d playerPos = player->position;
	V2d vPos = V2d(pos.x, pos.y);

	

	minX = pos.x; -480;// *zoomFactor;
	maxX = pos.x + 480;// *zoomFactor;
	minY = pos.y - 270;// *zoomFactor;
	maxY = pos.y + 270;// *zoomFactor;

	double origMinX = minX;
	double origMaxX = maxX;
	double origMinY = minY;
	double origMaxY = maxY;

	int testRadius = 1200;//900
	double extraCap = 500;
	double extra = 200;
	double extraDist = 200;

	double capMinX = origMinX - extraCap;
	double capMaxX = origMaxX + extraCap;
	double capMinY = origMinY - extraCap;
	double capMaxY = origMaxY + extraCap;

	Enemy *curr = owner->activeEnemyList;
	while (curr != NULL)
	{
		//bool alwaysExclude = (curr->type == Enemy::BASICEFFECT || length( playerPos - curr->position ) > 900
		//	|| curr->dead);
		//bool extra = (curr->type == Enemy::GHOST || curr->type == Enemy::GORILLA );
		//have stuff here for relative movers so 
		//bool sometimesExclude = curr->type == Enemy::GHOST

		if ( !curr->affectCameraZoom)
		{
			curr = curr->next;
			continue;
		}

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

			if (currPos.x < origMinX)
			{
				minX = min(minX, currPos.x - extra);
			}
			else if (currPos.x > origMaxX)
			{
				maxX = max(maxX, currPos.x + extra);
			}

			if (currPos.y < origMinY)
			{
				minY = min(minY, currPos.y - extra);
			}
			else if (currPos.y > origMaxY)
			{
				maxY = max(maxY, currPos.y + extra);
			}
		}
		

		
		/*double len = length(curr->position - vPos);

		V2d dir = normalize(curr->position - vPos);

		double leftX = curr->position.x - extra;
		double leftCap = minX - extraCap;
		double rightX = curr->position.x + extra;
		double rightCap = maxX + extraCap;
		double topY = curr->position.y - extra;
		double topCap = minY - extraCap;
		double botY = curr->position.y + extra;
		double botCap = maxY + extraCap;

		bool a = leftX < minX && leftX >= leftCap;
		bool b = rightX > maxX && rightX <= rightCap;
		bool c = topY < minY && topY >= topCap;
		bool d = botY > maxY && botY <= botCap;

		bool in = leftX >= minX && rightX <= maxX && topY >= minY && botY <= maxY;
		bool inX = leftX >= minX && rightX <= maxX;
		bool inY = topY >= minY && botY <= maxY;

		bool aa = (a || b) && (inY || c || d);
		bool cc = (c || d) && (inX || a || b);

		if (in)
		{
			++numEnemies;
		}
		else if (aa || cc )
		{
			++numEnemies;
			if (a)
			{
				minX = leftX;
			}
			else if (b)
			{
				maxX = rightX;
			}

			if (c)
			{
				minY = topY;
			}
			else if (d)
			{
				maxY = botY;
			}

		}*/

		curr = curr->next;
	}

	return numEnemies;
}

void Camera::SetTestOffset( V2d &pVel )
{
	float offXAccelOut = .1;
	float offXAccelIn = .1;
	float offYAccelOut = .1;
	float offYAccelIn = .1;

	Vector2f targetOffset;
	double maxXVelRegistered = 30.0;
	Vector2f moveFactor;
	float moveF = 1.0;


	

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
		/*float zFactor = 1.f;
		if (moveFrames.y > 0)
		{
			moveFrames.y -= zFactor;
		}
		else if (moveFrames.y < 0)
		{
			moveFrames.y += zFactor;
		}*/
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
	if (targetOffset.x > offset.x)
	{
		offsetVel.x = maxVel.x;
	}
	else
	{
		offsetVel.x = -maxVel.x;
	}

	if (targetOffset.y > offset.y)
	{
		offsetVel.y = maxVel.y;
	}
	else
	{
		offsetVel.y = -maxVel.y;
	}

	Vector2f dir = normalize(targetOffset - offset);
	float speed = 2.0;
	if ((pVel.x > 0 && offset.x < 0) || pVel.x < 0 && offset.x > 0 )
	{
		speed *= 2;
	}

	dir.x *= speed;

	offsetVel = dir;// *speed;
	//offsetVel.x *= abs(dir.x);
	//offsetVel.y *= abs(dir.y);
	offset += offsetVel;//targetOffset;
	Vector2f diff = targetOffset - offset;
	if (abs(diff.x) <= maxVel.x ) 
	{
		offset.x = targetOffset.x;
		offsetVel.x = 0;
	}
	if( abs( diff.y ) <= maxVel.y )
	{
		offset.y = targetOffset.y;
		offsetVel.y = 0;
	}
	//cout << "offset: " << offset.x << " , " << offset.y << endl;
}

double Camera::GetNewEnemyZoom( Actor *player )
{
	double minX, maxX, minY, maxY;

	numActive = GetActiveEnemyCount( player, minX, maxX, minY, maxY );

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

	

	double tw = max( abs( maxX - pos.x ), abs( minX - pos.x ) ) * 2;
	double th = max( abs( maxY - pos.y ), abs( minY - pos.y ) ) * 2;

	double w= tw/ 960.0;
	double h= th/ 540.0;

	double ratio = max( w, h );
	
	ratio = max( ratio, 1.0 );
	testZoom = ratio;

	double blah = .01;
	double zFactor = zoomFactor;

	return testZoom;

	/*if (numActive > 0)
	{
		if (testZoom > zFactor)
		{
			zFactor += blah;
			if (zFactor > testZoom)
				zFactor = testZoom;
		}
	}

	if (zFactor < 1)
		zFactor = 1;
	else if (zFactor > maxZoom)
		zFactor = maxZoom;

	return zFactor;*/
	//cout << "numActive: " << numActive << endl;
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

void Camera::Ease( Vector2f &p_pos, float zFactor, int numFrames, CubicBezier &bez)
{
	assert(manual);

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

	if (rumbleFrame > 0)
	{
		pos.x -= rX;
		pos.y -= rY;
	}
	
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
	
	rX = fx * rumbleX;
	rY = fy * rumbleY;

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
	if (!manual)
	{
		int x = 5;
	}
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
		for (int i = 0; i < NUM_STEPS; ++i)
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
	UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);
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
			if (zoomLevel < -.8)
				zoomLevel = -.8;
		}
		
		if (zoomLevel < -1)
			zoomLevel = -1;
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
}

sf::Vector2<double> Camera::GetPlayerVel( Actor *player)
{
	V2d pVel;
	double cap = 30;
	if (player->grindEdge != NULL && (player->action == Actor::GRINDBALL))
	{
		//cout << "here!" << endl;
		V2d grindDir = normalize(player->grindEdge->v1 - player->grindEdge->v0);
		V2d otherDir = grindDir;
		double oTemp = otherDir.x;
		otherDir.x = otherDir.y;
		otherDir.y = -oTemp;

		if (player->framesGrinding < cap)
		{

			playerPos += otherDir * player->normalHeight * (1 - player->framesGrinding / cap);
			//cout << "height on: " << playerPos.y << endl;
		}
		pVel = grindDir * player->grindSpeed;
		//cout << "grindspeed: " << player->grindSpeed << endl;

		if (player->reversed)
		{
			pVel = -pVel;
		}
	}
	else if (player->ground != NULL)
	{
		double cap2 = cap;
		if (player->framesGrinding < cap)
		{
			//cout << "bad trigger" << endl;
			player->framesNotGrinding = cap - player->framesGrinding;
			//cap2 = player->framesGrinding;
		}

		if (player->framesNotGrinding <= cap2)
		{
			V2d otherDir;
			if (player->ground != NULL)
			{
				otherDir = -player->ground->Normal();

				V2d toffset = otherDir * player->normalHeight * (1 - player->framesNotGrinding / cap2);
				playerPos += toffset;
				//cout << "height off: " << playerPos.y << endl;
				//cout << "offset: " << offset.x << ", " << offset.y << ", framesNotGrinding: " << player->framesNotGrinding << endl;
			}
		}

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

	return pVel;
}

double Camera::GetNewZoomFactor( Actor *player )
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

	//double zDiff = temp - zFactor;
	//if (zDiff > 0)
	//{
	//	zFactor += zDiff / 100.0/*35.0*/ / player->slowMultiple;
	//}
	//else if (zDiff < 0)
	//{
	//	double oldZoom = zFactor;
	//	bool more = oldZoom > testZoom;
	//	if (numActive == 0 )// || more)
	//	{
	//		zFactor += zDiff / 350.0 / player->slowMultiple;
	//		if (more)
	//		{
	//			if (zFactor < testZoom)
	//				zFactor = testZoom;
	//		}
	//	}
	//}

	//if (zFactor < 1)
	//	zFactor = 1;
	//else if (zFactor > maxZoom)
	//	zFactor = maxZoom;

	//return zFactor;
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

	int topBoundsCamExtra = 50;
	int topBounds = owner->mh->topBounds + topBoundsCamExtra;

	float halfw = 960 / 2;
	float halfh = 540 / 2;

	float left = truePos.x - halfw * GetZoom();

	float tdiff = owner->mh->leftBounds - left;
	if (tdiff > 0)
	{
		xChangePos = tdiff;
	}
	float right = truePos.x + halfw * GetZoom();

	tdiff = right - (owner->mh->leftBounds + owner->mh->boundsWidth);
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

	float bot = truePos.y + halfh * GetZoom();
	tdiff = bot - (owner->mh->topBounds + owner->mh->boundsHeight);
	if (tdiff > 0)
	{
		yChangeNeg = -tdiff;
	}

	Barrier *leftb = NULL, *rightb = NULL, *topb = NULL, *botb = NULL;
	for (list<Barrier*>::iterator it = owner->barriers.begin(); it != owner->barriers.end(); ++it)
	{
		if ((*it)->triggered)
		{
			continue;
		}


		if ((*it)->x)
		{
			if ((*it)->positiveOpen)
			{
				float diff = (*it)->pos - left;
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
				float diff = right - (*it)->pos;
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
				float diff = (*it)->pos - top;
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
				float diff = bottom - (*it)->pos;
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

void Camera::Update( Actor *player )
{
	if( owner == NULL )
		owner = player->owner;

	if (manual)
	{
		ManualUpdate( player );
		return;
	}
	V2d playerPos = player->position;


	if( player->action == Actor::RIDESHIP || player->action == Actor::SKYDIVE )
	{
		zoomFactor = 1;
		pos.x = playerPos.x;
		pos.y = playerPos.y;
		return;
	}

	if( owner->pauseFrames > 0 )
	{
		pos.x = playerPos.x + offset.x;
		pos.y = playerPos.y + offset.y;
	}
	
	ControllerState & con = player->currInput;
	ControllerState & prevcon = player->prevInput;
	UpdateZoomLevel(con, prevcon);

	V2d pVel = GetPlayerVel(player);

	if( pVel.y > 0 )
	{
		if( framesFalling < 60 )
			framesFalling++;
	}
	else 
	{
		if( framesFalling > 0 )
			framesFalling--;
	}



	pos.x = playerPos.x;
	pos.y = playerPos.y;
	
	if (owner->debugScreenRecorder != NULL)
	{
		offset.x = 0;
		offset.y = 0;
		zoomFactor = 1;
	}
	else
	{
		SetTestOffset(pVel);
		//SetMovementOffset(pVel);
	}

	pos.x += offset.x;
	pos.y += offset.y;

	

	double enemyZoom = GetNewEnemyZoom( player );
	//if( numActive == 0 )
	double zFactor = GetNewZoomFactor(player);

	if (enemyZoom > zFactor)
	{
		double blah = .01;
		if (numActive > 0)
		{
			if (testZoom > zoomFactor)
			{
				zoomFactor += blah;
			if (zoomFactor > testZoom)
				zoomFactor = testZoom;
			}
		}
	}
	else if( enemyZoom <= zFactor )
	{
		double zDiff = zFactor - zoomFactor;
		if (zDiff > 0)
		{
			zoomFactor += zDiff / 100.0/*35.0*/ / player->slowMultiple;
		}
		else if (zDiff < 0)
		{
			double oldZoom = zoomFactor;
			//bool more = oldZoom > testZoom;
			//if (numActive == 0 )// || more)
			{
				//zoomFactor += zDiff / 350.0 / player->slowMultiple;
				zoomFactor += zDiff / 200.0 / player->slowMultiple;
				/*if (more)
				{
					if (zoomFactor < testZoom)
						zoomFactor = testZoom;
				}*/
			}
		}
	}


	

	if( zoomFactor < 1 )
		zoomFactor = 1;
	else if( zoomFactor > maxZoom )
		zoomFactor = maxZoom;

	UpdateEaseOut();

	UpdateRumble();

	float xChangePos = 0, xChangeNeg = 0, yChangePos = 0, yChangeNeg = 0;
	UpdateBarrier(player, xChangePos, xChangeNeg, yChangePos, yChangeNeg);




	//offset.y += (double)framesFalling / 60 * 100.0;


	//cout << "offset: " << offset.x << ", " << offset.y << endl;
	/*double numActive = 0;
	Enemy *curr = owner->activeEnemyList;
	while( curr != NULL )
	{
		++numActive;
		curr = curr->next;
	}
	cout << "num active: " << numActive << endl;*/
}

void Camera::UpdateEaseOut()
{
	if (easingOut)
	{
		double r = (double)easeOutFrame / easeOutCount;
		sf::Vector2<double> p;
		p = r * sf::Vector2<double>(pos.x, pos.y) + (1.0 - r) * sf::Vector2<double>(manualPos.x, manualPos.y);
		pos = Vector2f(p.x, p.y);
		zoomFactor = r * zoomFactor + (1 - r) * manualZoom;

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
	double distx = abs( a->position.x - a2->position.x );
	double disty = abs( a->position.y - a2->position.y );

	double zx = distx / res.x;
	double zy = disty / res.y;
	
	double maxDist = max( zx, zy );

	if( zx == maxDist )
	{
		targetZoomFactor = zx;// / 2.0;

		double adistx = distx + max( 200.f * targetZoomFactor, 200.f );
		double azx = adistx / res.x;
		targetZoomFactor = azx;
	}
	else if( zy == maxDist )
	{
		targetZoomFactor = zy;// / 2.0;

		double adisty = disty + max( 200.f * targetZoomFactor, 200.f );
		double azy = adisty / res.y;
		targetZoomFactor = azy;
	}
	
	

	//double adistx = distx + 200;
	//double adisty = disty + 200;

	
	//double azy = adisty / res.y;
	
	

	
	//zoomFactor += 200 / res.x;

	double minZoom = .6;//1.0;
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
	return zoomLevel + zoomFactor;
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


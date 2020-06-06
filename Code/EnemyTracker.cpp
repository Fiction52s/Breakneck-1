#include "EnemyTracker.h"
#include "GameSession.h"
#include "Enemy.h"

bool EnemyTracker::GetClosestEnemyPos(GameSession *owner, V2d &pos,
	double radius, Enemy *&foundEnemy, int &foundIndex )
{
	Enemy *curr = owner->activeEnemyList;
	double lenSqr;
	int numPoints;
	V2d camPoint;
	int i;
	V2d closestPoint;
	bool foundPoint = false;
	double closestLenSqr;
	double radSqr = radius * radius;
	Enemy *fEnemy = NULL;
	int fIndex = 0;

	bool enemyCanBeAnchored;
	while (curr != NULL)
	{
		if (IsValidTrackEnemy(curr))
		{
			numPoints = curr->GetNumCamPoints();
			for (i = 0; i < numPoints; ++i)
			{
				camPoint = curr->GetCamPoint(i);
				lenSqr = lengthSqr(pos - camPoint);
				if (lenSqr <= radSqr && (!foundPoint || (foundPoint && lenSqr <= closestLenSqr)))
				{
					foundPoint = true;
					closestLenSqr = lenSqr;
					closestPoint = camPoint;
					fEnemy = curr;
					fIndex = i;
				}
			}
		}
		
		curr = curr->next;
	}

	if (foundPoint)
	{
		foundEnemy = fEnemy;
		foundIndex = fIndex;
	}

	return foundPoint;
}

bool EnemyTracker::IsValidTrackEnemy( Enemy *e )
{
	if (e->dead)
		return false;

	return true;
}
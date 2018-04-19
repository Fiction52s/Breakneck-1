#include "ObjectPool.h"
#include <assert.h>

ObjectPool::ObjectPool()
{
	activeListStart = NULL;
	inactiveListStart = NULL;
	Reset();
}

void ObjectPool::ResetPool()
{
	DeactivateAll();
	numInactiveMembers = 0;
	numActiveMembers = 0;

	activeListStart = NULL;
	activeListEnd = NULL;
	//inactiveListStart = NULL;
}

PoolMember *ObjectPool::ActivatePoolMember( bool front )
{
	PoolMember *newActiveMember = NULL;
	if (inactiveListStart == NULL)
	{
		return NULL;
	}
	else
	{
		newActiveMember = inactiveListStart;


		
		if (inactiveListStart->pmnext != NULL)
		{
			inactiveListStart->pmnext->pmprev = NULL;
		}

		inactiveListStart = inactiveListStart->pmnext;

		++numActiveMembers;

		newActiveMember->pmprev = NULL;
		newActiveMember->pmnext = NULL;

		if (activeListStart == NULL)
		{
			activeListStart = newActiveMember;
			activeListEnd = newActiveMember;
		}
		else if (front)
		{
			activeListStart->pmprev = newActiveMember;
			newActiveMember->pmnext = activeListStart;
			activeListStart = newActiveMember;
		}
		else
		{
			activeListEnd->pmnext = newActiveMember;
			newActiveMember->pmprev = activeListEnd;
			activeListEnd = newActiveMember;
		}


		return newActiveMember;
	}

	
}

void ObjectPool::DeactivatePoolMember(PoolMember *pm)
{
	DeactivateMember( pm );

	if (activeListStart == NULL)
	{
		assert(0);
		return;
	}

	if (pm->pmprev == NULL && pm->pmnext == NULL)
	{
		activeListStart = NULL;
		activeListEnd = NULL;
	}
	else if (pm->pmprev == NULL)
	{
		pm->pmnext->pmprev = NULL;
		activeListStart = pm->pmnext;
	}
	else if (pm->pmnext == NULL)
	{
		pm->pmprev->pmnext = NULL;
		activeListEnd = pm->pmprev;
	}
	else
	{
		pm->pmprev->pmnext = pm->pmnext;
		pm->pmnext->pmprev = pm->pmprev;
	}

	AddToInactiveList(pm);

	assert(numActiveMembers > 0);

	--numActiveMembers;
}

void ObjectPool::AddToInactiveList( PoolMember *pm )
{
	pm->pmprev = NULL;
	pm->pmnext = NULL;

	if (inactiveListStart != NULL)
	{	
		pm->pmnext = inactiveListStart;
		inactiveListStart->pmprev = pm;
	}
	inactiveListStart = pm;
}

void ObjectPool::DeactivateAll()
{
	PoolMember *curr = activeListStart;
	PoolMember *tNext = NULL;
	while (curr != NULL)
	{
		tNext = curr->pmnext;
		DeactivatePoolMember(curr);
		curr = tNext;
	}
}
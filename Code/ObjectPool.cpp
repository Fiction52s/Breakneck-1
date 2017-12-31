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


		
		if (inactiveListStart->next != NULL)
		{
			inactiveListStart->next->prev = NULL;
		}

		inactiveListStart = inactiveListStart->next;

		++numActiveMembers;

		newActiveMember->prev = NULL;
		newActiveMember->next = NULL;

		if (activeListStart == NULL)
		{
			activeListStart = newActiveMember;
			activeListEnd = newActiveMember;
		}
		else if (front)
		{
			activeListStart->prev = newActiveMember;
			newActiveMember->next = activeListStart;
			activeListStart = newActiveMember;
		}
		else
		{
			activeListEnd->next = newActiveMember;
			newActiveMember->prev = activeListEnd;
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

	if (pm->prev == NULL && pm->next == NULL)
	{
		activeListStart = NULL;
		activeListEnd = NULL;
	}
	else if (pm->prev == NULL)
	{
		pm->next->prev = NULL;
		activeListStart = pm->next;
	}
	else if (pm->next == NULL)
	{
		pm->prev->next = NULL;
		activeListEnd = pm->prev;
	}
	else
	{
		pm->prev->next = pm->next;
		pm->next->prev = pm->prev;
	}

	AddToInactiveList(pm);

	assert(numActiveMembers > 0);

	--numActiveMembers;
}

void ObjectPool::AddToInactiveList( PoolMember *pm )
{
	pm->prev = NULL;
	pm->next = NULL;

	if (inactiveListStart != NULL)
	{	
		pm->next = inactiveListStart;
		inactiveListStart->prev = pm;
	}
	inactiveListStart = pm;
}

void ObjectPool::DeactivateAll()
{
	PoolMember *curr = activeListStart;
	PoolMember *tNext = NULL;
	while (curr != NULL)
	{
		tNext = curr->next;
		DeactivatePoolMember(curr);
		curr = tNext;
	}
}
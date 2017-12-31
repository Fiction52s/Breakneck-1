#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <SFML/Graphics.hpp>

struct PoolMember
{
	PoolMember(int p_index)
		:prev(NULL), next(NULL), index( p_index )
	{}
	PoolMember *next;
	PoolMember *prev;
	int index;
};

struct ObjectPool
{
	ObjectPool();
	virtual void Reset() { ResetPool(); }
	void ResetPool();
	PoolMember *activeListStart;
	PoolMember *activeListEnd;
	PoolMember *inactiveListStart;

	PoolMember *ActivatePoolMember( bool front = true );
	void DeactivatePoolMember(PoolMember *pm);
	void AddToInactiveList(PoolMember *pm);
	void AllocateMember(int index);
	void DeactivateAll();
	virtual void DeactivateMember(PoolMember *pm) {}

	//int numMembersTotal;
	int numActiveMembers;
	int numInactiveMembers;
};

#endif
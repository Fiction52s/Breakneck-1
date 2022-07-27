#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <SFML/Graphics.hpp>

struct PoolMember
{
	PoolMember(int p_index)
		:pmprev(NULL), pmnext(NULL), index( p_index )
	{}
	virtual ~PoolMember() {}
	PoolMember *pmnext;
	PoolMember *pmprev;
	int index;
};

struct ObjectPool
{
	ObjectPool();
	virtual ~ObjectPool();
	virtual void Reset() { ResetPool(); }
	void ResetPool();
	PoolMember *activeListStart;
	PoolMember *activeListEnd;
	PoolMember *inactiveListStart;

	PoolMember *ActivatePoolMember( bool front = true );
	void DeactivatePoolMember(PoolMember *pm);
	void AddToInactiveList(PoolMember *pm);
	void DeactivateAll();
	void DestroyAllMembers();
	virtual void DeactivateMember(PoolMember *pm) {}

	//int numMembersTotal;
	int numActiveMembers;
	int numInactiveMembers;
};

//struct ObjectPoolArray
//{
//	ObjectPoolArray();
//	virtual void Reset() { ResetPool(); }
//	void ResetPool();
//	PoolMember *activeListStart;
//	PoolMember *activeListEnd;
//	PoolMember *inactiveListStart;
//
//	PoolMember *ActivatePoolMember(bool front = true);
//	void DeactivatePoolMember(PoolMember *pm);
//	void AddToInactiveList(PoolMember *pm);
//	void DeactivateAll();
//	void DestroyAllMembers();
//	virtual void DeactivateMember(PoolMember *pm) {}
//
//	//int numMembersTotal;
//	int numActiveMembers;
//	int numInactiveMembers;
//};

#endif
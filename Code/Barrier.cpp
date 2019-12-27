#include "Barrier.h"
#include "GameSession.h"
#include "BarrierReactions.h"

Barrier::Barrier(GameSession *p_owner, const std::string &p_name, bool p_x, int p_pos, BarrierCallback *cb)
{
	owner = p_owner;
	name = p_name;
	callback = cb;
	x = p_x;
	pos = p_pos;
	triggered = false;

	if (x)
	{
		positiveOpen = (owner->GetPlayerPos().x > pos);
	}
	else
	{
		positiveOpen = (owner->GetPlayerPos().y > pos);
	}
}

void Barrier::Reset()
{
	triggered = false;
}

bool Barrier::Update()
{
	if (triggered)
		return false;

	V2d playerPos = owner->GetPlayerPos();

	if (x)
	{
		if (positiveOpen) //player starts right
		{
			if (playerPos.x < pos)
			{
				triggered = true;
			}
		}
		else //starts left
		{
			if (playerPos.x > pos)
			{
				triggered = true;
			}
		}
	}
	else
	{
		if (positiveOpen) // player starts below
		{
			if (playerPos.y < pos)
			{
				triggered = true;
			}
		}
		else //player starts above
		{
			if (playerPos.y > pos)
			{
				triggered = true;
			}
		}
	}

	return triggered;
}

void Barrier::SetPositive()
{
	V2d playerPos = owner->GetPlayerPos();
	//should use a parameter eventually but for now just using this
	if (x)
	{
		if (playerPos.x - pos > 0)
		{
			positiveOpen = true;
		}
		else
			positiveOpen = false;
	}
	else
	{
		if (playerPos.y - pos > 0)
		{
			positiveOpen = true;
		}
		else
		{
			positiveOpen = false;
		}
	}
}
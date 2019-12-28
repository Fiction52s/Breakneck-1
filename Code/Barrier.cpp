#include "Barrier.h"
#include "GameSession.h"
#include "BarrierReactions.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

Barrier::Barrier(GameSession *p_owner, const std::string &p_name, bool p_x, int p_pos, BarrierCallback *cb)
{
	owner = p_owner;
	name = p_name;
	callback = cb;
	x = p_x;
	pos = p_pos;
	triggered = false;
	edgeActive = true;

	double top = owner->mh->topBounds;
	double bottom = owner->mh->topBounds + owner->mh->boundsHeight;

	barrierEdge = new Edge;
	barrierEdge->edgeType = Edge::BARRIER;
	barrierEdge->info = this;

	line[0].color = Color::Red;
	line[1].color = Color::Red;

	//globalBorderEdges.push_back(left);
	//globalBorderEdges.push_back(right);
	//globalBorderEdges.push_back(top);
	//globalBorderEdges.push_back(bot);

	auto it = owner->globalBorderEdges.begin();
	++it;
	++it;


	if (x)
	{
		positiveOpen = (owner->GetPlayerPos().x > pos);

		if (positiveOpen)
		{
			barrierEdge->v0 = V2d(pos, top);
			barrierEdge->v1 = V2d(pos, bottom);
		}
		else
		{
			barrierEdge->edge0 = (*it);
			++it;
			barrierEdge->edge1 = (*it);
			barrierEdge->v0 = V2d(pos, bottom);
			barrierEdge->v1 = V2d(pos, top);
		}
	}
	else
	{
		positiveOpen = (owner->GetPlayerPos().y > pos);
	}

	line[0].position = Vector2f(barrierEdge->v0);
	line[1].position = Vector2f(barrierEdge->v1);

	owner->barrierTree->Insert(barrierEdge);
}

Barrier::~Barrier()
{
	delete barrierEdge;
}

void Barrier::Reset()
{
	triggered = false;
	edgeActive = true;
}

void Barrier::DebugDraw(sf::RenderTarget *target)
{
	target->draw(line, 2, sf::Lines);
}

bool Barrier::IsPointWithinBarrier(sf::Vector2<double> &p)
{
	if (triggered)
		return true;

	if (x)
	{
		if (positiveOpen)
		{
			return p.x >= pos;
		}
		else
		{
			return p.x <= pos;
		}
	}
	else 
	{
		if (positiveOpen)
		{
			return p.y >= pos;
		}
		else
		{
			return p.y <= pos;
		}
	}
}

bool Barrier::Update()
{
	if (triggered )
		return false;

	V2d playerPos = owner->GetPlayerPos();

	double extra = 0;

	if (x)
	{
		if (positiveOpen) //player starts right
		{
			if (playerPos.x < pos - extra)
			{
				triggered = true;
			}
		}
		else //starts left
		{
			if (playerPos.x > pos + extra)
			{
				triggered = true;
			}
		}
	}
	else
	{
		if (positiveOpen) // player starts below
		{
			if (playerPos.y < pos - extra)
			{
				triggered = true;
			}
		}
		else //player starts above
		{
			if (playerPos.y > pos + extra)
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

void Barrier::Trigger()
{
	edgeActive = false;
	triggered = true;
}
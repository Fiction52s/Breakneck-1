#include "Barrier.h"
#include "Session.h"
#include "BarrierReactions.h"
#include "MapHeader.h"
#include "Sequence.h"
#include "GameSession.h"
#include "PauseMenu.h"
#include "WarpTransitionSequence.h"

using namespace std;
using namespace sf;

Barrier::Barrier(const std::string &p_name, bool p_x, int p_pos, bool hasEdge, BarrierCallback *cb, bool p_hasWarp )
{
	sess = Session::GetSession();
	name = p_name;
	callback = cb;
	x = p_x;
	pos = p_pos;
	hasWarp = p_hasWarp;

	myBonus = NULL;
	warpSeq = NULL;
	triggerSeq = NULL;

	double top = sess->mapHeader->topBounds;
	double bottom = sess->mapHeader->topBounds + sess->mapHeader->boundsHeight;

	line[0].color = Color::Red;
	line[1].color = Color::Red;

	extraDistance = 50;
	if (x)
	{
		positiveOpen = (sess->playerOrigPos[0].x > pos);
		
	}
	else
	{
		positiveOpen = (sess->playerOrigPos[0].y > pos);
	}

	if (!positiveOpen)
		extraDistance = -extraDistance;

	if (hasEdge)
	{
		barrierEdge = new Edge;
		barrierEdge->edgeType = Edge::BARRIER;
		barrierEdge->info = this;

		auto it = sess->globalBorderEdges.begin();
		++it;
		++it;

		if (x)
		{
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
			
		}

		line[0].position = Vector2f(pos, top);
		line[1].position = Vector2f(pos, bottom);

		sess->barrierTree->Insert(barrierEdge);
	}
	else
	{
		barrierEdge = NULL;
	}

	Reset();
}

int Barrier::GetCamPos()
{
	return pos + extraDistance;
}

void Barrier::SetScene()
{
	if (!hasWarp)
	{
		Sequence *seq = Sequence::CreateScene(name);

		if (seq != NULL)
		{
			seq->Reset();
			seq->barrier = this;
			triggerSeq = seq;
		}
	}
	else
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			myBonus = game->CreateBonus(name);

			warpSeq = new WarpTransitionSequence;
			warpSeq->Init();

			warpSeq->bonus = myBonus;
			warpSeq->barrier = this;
		}
	}
}

Barrier::~Barrier()
{
	if( barrierEdge != NULL )
		delete barrierEdge;

	if (triggerSeq != NULL)
	{
		delete triggerSeq;
	}

	if (myBonus != NULL)
		delete myBonus;

	if (warpSeq != NULL)
		delete warpSeq;
}

void Barrier::Reset()
{
	triggered = false;

	if (barrierEdge != NULL)
	{
		edgeActive = true;
	}
	else
	{
		edgeActive = false;
	}

	if (triggerSeq != NULL)
		triggerSeq->Reset();
}

void Barrier::SetWarpSeq()
{
	assert(warpSeq != NULL);
	warpSeq->Reset();
	sess->SetActiveSequence(warpSeq);
}

void Barrier::DeactivateEdge()
{
	edgeActive = false;
}

void Barrier::ActivateEdge()
{
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

	V2d playerPos = sess->GetPlayerPos();

	bool t = false;

	double extra = 0;

	if (x)
	{
		if (positiveOpen) //player starts right
		{
			if (playerPos.x < pos - extra)
			{
				t = true;
			}
		}
		else //starts left
		{
			if (playerPos.x > pos + extra)
			{
				t = true;
			}
		}
	}
	else
	{
		if (positiveOpen) // player starts below
		{
			if (playerPos.y < pos - extra)
			{
				t = true;
			}
		}
		else //player starts above
		{
			if (playerPos.y > pos + extra)
			{
				t = true;
			}
		}
	}

	return t;
}

void Barrier::SetPositive()
{
	V2d playerPos = sess->GetPlayerPos();
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

double Barrier::GetPlayerDist()
{
	V2d playerPos = sess->GetPlayerPos();

	double dist = 0;
	if (x)
	{
		dist = playerPos.x - pos;

		if (!positiveOpen)
		{
			dist = -dist;
		}
	}
	else
	{

	}

	return dist;
}
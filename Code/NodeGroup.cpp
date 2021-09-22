#include "NodeGroup.h"
#include <assert.h>
#include "CircleGroup.h"
#include "PoiInfo.h"
#include <iostream>

using namespace sf;
using namespace std;

NodeGroup::NodeGroup( int p_numPickers, sf::Color c)
{
	nodeVec = NULL;
	circles = NULL;
	circleColor = c;
	numPickers = p_numPickers;
	pickers.resize(numPickers);
}

NodeGroup::~NodeGroup()
{
	if (circles != NULL)
	{
		delete circles;
	}
}

void NodeGroup::SetNodeVec(std::vector<PoiInfo*> *nv)
{
	if (nv == NULL)
		return;

	nodeVec = nv;
	assert(nodeVec != NULL);

	int numNodes = nodeVec->size();

	if (circles != NULL)
	{
		if (circles->numCircles != numNodes)
		{
			delete circles;
			circles = NULL;
		}
	}

	if (circles == NULL)
	{
		circles = new CircleGroup(numNodes, 10, circleColor, 6);
	}

	for (int i = 0; i < numPickers; ++i)
	{
		pickers[i].ReserveNumOptions(numNodes);
		pickers[i].Reset();
	}
	

	for (int i = 0; i < numNodes; ++i)
	{
		for (int j = 0; j < numPickers; ++j)
		{
			pickers[j].AddActiveOption(i);
		}
		
		circles->SetPosition(i, Vector2f(nodeVec->at(i)->pos));
	}

	for (int i = 0; i < numPickers; ++i)
	{
		pickers[i].SetPreventShuffleRepetition(true);
	}
	

	circles->ShowAll();
}

void NodeGroup::Draw(sf::RenderTarget *target)
{
	if (circles != NULL)
	{
		circles->Draw(target);
	}
}

PoiInfo *NodeGroup::AlwaysGetNextNode( int pickerIndex )
{
	int ind = pickers[pickerIndex].AlwaysGetNextOption();
	return nodeVec->at(ind);
}

PoiInfo * NodeGroup::TryGetNextNode( int pickerIndex )
{
	int res = pickers[pickerIndex].TryGetNextOption();
	if (res == -1)
	{
		return NULL;
	}
	else
	{
		return nodeVec->at(res);
	}
}

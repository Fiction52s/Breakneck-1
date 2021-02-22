#include "NodeGroup.h"
#include <assert.h>
#include "CircleGroup.h"
#include "PoiInfo.h"
#include <iostream>

using namespace sf;
using namespace std;

NodeGroup::NodeGroup(sf::Color c)
{
	nodeVec = NULL;
	circles = NULL;
	circleColor = c;
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

	picker.ReserveNumOptions(numNodes);
	picker.Reset();

	for (int i = 0; i < numNodes; ++i)
	{
		picker.AddActiveOption(i);
		circles->SetPosition(i, Vector2f(nodeVec->at(i)->pos));
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

PoiInfo *NodeGroup::AlwaysGetNextNode()
{
	int ind = picker.AlwaysGetNextOption();
	return nodeVec->at(ind);
}

PoiInfo * NodeGroup::TryGetNextNode()
{
	int res = picker.TryGetNextOption();
	if (res == -1)
	{
		return NULL;
	}
	else
	{
		return nodeVec->at(res);
	}
}

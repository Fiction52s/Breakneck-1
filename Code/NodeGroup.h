#ifndef __NODEGROUP_H__
#define __NODEGROUP_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include "RandomPicker.h"

struct PoiInfo;
struct CircleGroup;

struct NodeGroup
{
	NodeGroup(sf::Color c);
	~NodeGroup();
	void Draw(sf::RenderTarget *target);
	void SetNodeVec(std::vector<PoiInfo*> *nv);
	PoiInfo *AlwaysGetNextNode();
	PoiInfo * TryGetNextNode();
	std::vector<PoiInfo*> *nodeVec;
	CircleGroup *circles;
	sf::Color circleColor;
	RandomPicker picker;
	
};

#endif
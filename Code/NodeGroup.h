#ifndef __NODEGROUP_H__
#define __NODEGROUP_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include "RandomPicker.h"
#include "PoiInfo.h"

struct CircleGroup;

struct NodeGroup
{
	NodeGroup(sf::Color c = sf::Color::Magenta);
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
#ifndef __NODEGROUP_H__
#define __NODEGROUP_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include "RandomPicker.h"
#include "PoiInfo.h"

struct CircleGroup;

struct NodeGroup
{
	NodeGroup( int numPickers = 1, sf::Color c = sf::Color::Magenta);
	~NodeGroup();
	void Draw(sf::RenderTarget *target);
	void SetNodeVec(std::vector<PoiInfo*> *nv);
	PoiInfo *AlwaysGetNextNode(int pickerIndex = 0);
	PoiInfo * TryGetNextNode( int pickerIndex = 0);
	std::vector<PoiInfo*> *nodeVec;
	CircleGroup *circles;
	sf::Color circleColor;
	std::vector<RandomPicker> pickers;
	int numPickers;

	
};

#endif
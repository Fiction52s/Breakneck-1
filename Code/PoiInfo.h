#ifndef __POIINFO_H__
#define __POIINFO_H__

#include "ISelectable.h"
struct Edge;

struct PoiInfo
{
	PoiInfo( const std::string &name, sf::Vector2i &p );
	PoiInfo( const std::string &name, PolyPtr p,
		int edgeIndex, double q );
	sf::Vector2<double> pos;	
	Edge *edge;
	PolyPtr poly;
	double edgeQuantity;
	std::string name;
	int edgeIndex;
};

#endif
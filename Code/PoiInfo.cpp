#include "PoiInfo.h"
#include <string>
#include <SFML\Graphics.hpp>
#include "Physics.h"
#include "EditorTerrain.h"

using namespace sf;

PoiInfo::PoiInfo(const std::string &pname, Vector2i &p)
{
	name = pname;
	pos.x = p.x;
	pos.y = p.y;
	edge = NULL;
	poly = NULL;
	edgeIndex = -1;
}

PoiInfo::PoiInfo(const std::string &pname, PolyPtr p, int eIndex, double q)
{
	poly = p;
	name = pname;
	edgeIndex = eIndex;
	edgeQuantity = q;
	edge = p->GetEdge(eIndex);

	pos = edge->GetPosition(edgeQuantity);

}
#ifndef __EDITORGATEINFO_H__
#define __EDITORGATEINFO_H__

#include "ISelectable.h"
#include "Gate.h"
#include <boost/shared_ptr.hpp>

struct TerrainPolygon;
struct TerrainPoint;
struct ActorParams;

struct GateInfo : ISelectable
{
	GateInfo();
	void SetType(const std::string &gType);
	void SetShard(int shardW, int shardI);
	sf::IntRect GetAABB();
	TerrainPoint *point0;
	TerrainPoint *point1;
	bool ContainsPoint(V2d &p);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	boost::shared_ptr<TerrainPolygon> poly0;
	int vertexIndex0;
	boost::shared_ptr<TerrainPolygon> poly1;
	int vertexIndex1;
	sf::VertexArray thickLine;
	EditSession *edit;
	void UpdateLine();
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	void DrawPreview(sf::RenderTarget *target);
	bool IsTouchingEnemy(ActorParams * a);
	int numKeysRequired;
	sf::Color color;
	Gate::GateType type;

	int shardWorld;
	int shardIndex;
	sf::Sprite shardSpr;
	sf::RectangleShape shardBG;

	static double lineWidth;

};

typedef boost::shared_ptr<GateInfo> GateInfoPtr;

struct GatePoint
{
	int vertexIndex;
	GateInfo *info;
};

#endif
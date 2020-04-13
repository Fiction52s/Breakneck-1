#ifndef __EDITORGATEINFO_H__
#define __EDITORGATEINFO_H__

#include "ISelectable.h"
#include "Gate.h"

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
	void Deactivate();
	void Activate();
	PolyPtr poly0;
	int vertexIndex0;
	PolyPtr poly1;
	int vertexIndex1;
	sf::VertexArray thickLine;
	EditSession *edit;
	void UpdateLine();
	void WriteFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	void DrawPreview(sf::RenderTarget *target);
	bool IsTouchingEnemy(ActorParams * a);
	sf::Color color;
	Gate::GateType type;

	int shardWorld;
	int shardIndex;
	sf::Sprite shardSpr;
	sf::RectangleShape shardBG;

	static double lineWidth;

};

#endif
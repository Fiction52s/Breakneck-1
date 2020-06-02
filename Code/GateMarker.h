#ifndef __GATEMARKER_H__
#define __GATEMARKER_H__

#include <SFML/Graphics.hpp>
#include "VectorMath.h"

struct GateInfo;
struct Tileset;
struct Gate;
struct GateMarker;
struct Zone;

struct GateMarkerGroup
{
	GateMarkerGroup(int maxGates);
	~GateMarkerGroup();
	void SetToZone(Zone *z);
	void Update(sf::View &v);
	void Draw(sf::RenderTarget *target);
	sf::Vertex *allQuads;
	Tileset *ts_gateMarker;
	sf::Font *font;
	int fadeInFrames;
	int fadeOutFrames;
};

struct GateMarker
{
	enum State
	{
		HIDE,
		FADEIN,
		SHOW,
		FADEOUT,
	};
	GateMarker(GateMarkerGroup *g, sf::Vertex *p_quad);
	void FadeIn();
	void FadeOut();
	void Reset();
	State state;
	int frame;
	int fadeInFrames;
	int fadeOutFrames;
	sf::Vertex *quad;
	
	void SetGate(Gate *g);
	void Update(sf::View &v );
	void SetGatePos(V2d pos);
	//bool show;
	//void SetType(int type, int num);
	void Draw(sf::RenderTarget *target);
	sf::CircleShape cs;
	sf::Text numText;

	GateInfo *currInfo;
	Gate *currGate;
	GateMarkerGroup *group;
};

#endif
#ifndef __GATEMARKER_H__
#define __GATEMARKER_H__

#include <SFML/Graphics.hpp>
#include "VectorMath.h"

struct GateInfo;

struct GateMarker
{
	enum State
	{
		HIDE,
		FADEIN,
		SHOW,
		FADEOUT,
	};

	void FadeIn();
	void FadeOut();
	void Reset();
	State state;
	int frame;
	int fadeInFrames;
	int fadeOutFrames;
	sf::Vertex quad[4];
	GateMarker();
	void Update(sf::View &v,
		GateInfo *gi);
	void SetGatePos(V2d pos);
	//bool show;
	//void SetType(int type, int num);
	void Draw(sf::RenderTarget *target);
};

#endif
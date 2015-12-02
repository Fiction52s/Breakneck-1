#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"


struct Gate : public Edge
{
	
	enum GateType
	{
		RED,
		GREEN,
		BLUE,
		Count		
	};
	Gate( GateType type);
	GateType type;
	bool locked;
	sf::Color c;
	sf::VertexArray thickLine;
	void UpdateLine();
	void Draw( sf::RenderTarget *target );
};

#endif
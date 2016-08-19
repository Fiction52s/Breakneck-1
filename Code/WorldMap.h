#ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>

struct WorldMap
{
	enum State
	{
		OFF,
		PLANET_AND_SPACE,
		PLANET_TRANSITION,
		PLANET,
		SECTION_TRANSITION,
		SECTION,
		COLONY_TRANSITION,
		COLONY
	};

	WorldMap( );
	void Reset();
	~WorldMap();
	void Update();
	void Draw( sf::RenderTarget *target );

	State state;
	int frame;
	sf::Texture *planetAndSpaceTex;
	sf::Texture *planetTex;
	int selectedColony;
	
	sf::Texture *sectionTex[6];
	sf::Texture *colonyTex[6];
	sf::Sprite back;
	sf::Sprite front;
	


};

#endif
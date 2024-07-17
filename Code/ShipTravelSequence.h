#ifndef __SHIP_TRAVEL_SEQUENCE_H__
#define __SHIP_TRAVEL_SEQUENCE_H__

#include "Sequence.h"

struct CompositeImage;


struct ShipTravelSequence : Sequence
{
	enum State
	{
		SHIP_FLYING,
		KIN_JUMP,
		Count
	};

	struct ShipTravelData
	{
		float extraBackgroundOffset;
	};

	ShipTravelData shipEnterData;
	CompositeImage *shipComp;

	Tileset *ts_shipTest;

	sf::Sprite shipTestSprite;
	//ship sequence. should be in its own structure
	Tileset *ts_w1ShipClouds0;
	Tileset *ts_w1ShipClouds1;
	Tileset *ts_ship;
	sf::Vertex cloud0[3 * 4];
	sf::Vertex cloud1[3 * 4];
	sf::Vertex cloudBot0[3 * 4];
	sf::Vertex cloudBot1[3 * 4];
	/*sf::VertexArray cloud0;
	sf::VertexArray cloud1;
	sf::VertexArray cloudBot0;
	sf::VertexArray cloudBot1;*/
	sf::Vector2f relShipVel;
	sf::Sprite shipSprite;
	sf::RectangleShape middleClouds;
	bool shipSequence;
	//bool hasShipEntrance;
	V2d shipTravelPos;
	sf::Vector2f cloudVel;
	//int shipSeqFrame;
	sf::Vector2f shipStartPos;

	ShipTravelSequence();
	~ShipTravelSequence();
	void Reset();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void AddFlashes();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	//void Draw(sf::RenderTarget *target);
};

#endif
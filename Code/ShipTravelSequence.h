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
	sf::Vector2f relShipVel;
	bool shipSequence;
	//bool hasShipEntrance;
	V2d shipTravelPos;
	sf::Vector2f cloudVel;
	//int shipSeqFrame;
	sf::Vector2f shipStartPos;

	ShipTravelSequence();
	~ShipTravelSequence();
	void Reset();
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);
	void DrawShip(sf::RenderTarget *target);
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
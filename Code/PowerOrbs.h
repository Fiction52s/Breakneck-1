#ifndef __PoWERORBS_H__
#define __PoWERORBS_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include <map>

struct GameSession;

struct FillRingSection
{
	enum RingType
	{
		NORMAL
	};

	FillRingSection(TilesetManager &tm,
		const sf::Color &active0, const sf::Color &remove0,
		const sf::Color &empty0, int rType, int p_maxPower,
		float startAngle );
	sf::Glsl::Vec4 activeColor;
	sf::Glsl::Vec4 removeColor;
	sf::Glsl::Vec4 emptyColor;
	void SetupSection(sf::Vector2f &centerPos);
	void Update();
	void UpdateSprite();
	void ResetEmpty();
	void ResetFull();
	bool IsFull();
	bool IsEmpty();
	RingType ringType;
	int prevPower;
	int maxPower;
	int currPower;
	//amount behind curr to color
	int currRemovingPower;
	int GetDivsActive();
	int GetDivsRemoved();
	int GetDivsEmpty();
	int Fill(int dmg);
	int Drain(int heal);
	void Draw(sf::RenderTarget *target);

	sf::Shader ringShader;
	//sf::Vertex ringSprite[4];
	sf::Sprite ringSprite;
	Tileset *ts_ring;

	int numRemovedDivs;
};

struct DesperationOrb
{
	Tileset *ts_orb;
	DesperationOrb(TilesetManager &tm, sf::Vector2f &pos );
	void Draw(sf::RenderTarget *target);
	void SetPosition(sf::Vector2f &pos);
	sf::Vector2f GetPosition();
	sf::Sprite orbSpr;
};

struct FillRing
{
	FillRing( sf::Vector2f &pos,int numRings,
		FillRingSection **rings);
	~FillRing();
	void SetPowers( bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires );

	FillRingSection **rings;
	void SetPosition(sf::Vector2f &pos);
	int numRings;
	int currRing;
	int Fill(int fill);
	int Drain(int drain);
	void ResetEmpty();
	void ResetFull();
	bool IsFull();
	bool IsEmpty();

	sf::VertexArray *ringVA;
	sf::VertexArray *middleVA;
	sf::CircleShape scorpTest;
	sf::CircleShape keyTest;
	void Update();

	//sf::Vertex *ringVA;
	int numTotalVertices;

	sf::Vector2f centerPos;
	void CreateRing();
	void Draw( sf::RenderTarget *target );

};

struct PowerRing : FillRing
{
	enum Mode
	{
		NORMAL,
		DESPERATION
	};
	PowerRing(sf::Vector2f &pos, int numRings,
		FillRingSection **rings)
		:FillRing( pos, numRings, rings ), mode( NORMAL )
	{

	}
	Mode mode;
	sf::CircleShape despCircle;
};

struct Actor;
struct KinRing
{
	KinRing( Actor *actor );
	~KinRing();

	sf::Vector2f GetCenter();
	void SetCenter(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void Reset();
	void Update();
	PowerRing *powerRing;
	DesperationOrb *despOrb;
};

#endif
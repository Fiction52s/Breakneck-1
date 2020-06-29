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

	sf::Glsl::Vec4 activeColor;
	sf::Glsl::Vec4 removeColor;
	sf::Glsl::Vec4 emptyColor;
	RingType ringType;
	int maxPower;
	int prevPower;
	int currPower;
	sf::Shader ringShader;
	sf::Sprite ringSprite;
	Tileset *ts_ring;

	void Set(int prevPower, int currPower );

	FillRingSection(TilesetManager &tm,
		const sf::Color &active0, const sf::Color &remove0,
		const sf::Color &empty0, int rType, int p_maxPower,
		float startAngle );
	void SetupSection(sf::Vector2f &centerPos);
	void Update();
	void UpdateSprite();
	void ResetEmpty();
	void ResetFull();
	bool IsFull();
	bool IsEmpty();
	int GetDivsActive();
	int GetDivsRemoved();
	int GetDivsEmpty();
	int Fill(int dmg);
	int Drain(int heal);
	void Draw(sf::RenderTarget *target);
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

	void Set(int currRing, int *prevArr, int *currArr);
	void GetData(int *currRing, int *prevArr,
		int *currArr);

	FillRingSection **rings;
	int numRings;
	int currRing;
	sf::Vector2f centerPos;
	int numTotalVertices;
	sf::VertexArray *ringVA;
	sf::VertexArray *middleVA;
	sf::CircleShape scorpTest;
	sf::CircleShape keyTest;

	void SetPosition(sf::Vector2f &pos);
	int Fill(int fill);
	int Drain(int drain);
	void ResetEmpty();
	void ResetFull();
	bool IsFull();
	bool IsEmpty();
	float GetCurrPortionOfTotal();
	void Update();
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

	void GetData(int *currRing, int *prevArr,
		int *currArr);
	void Set(int currRing, int *prevArr,
		int *currArr);
	sf::Vector2f GetCenter();
	void SetCenter(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void Reset();
	void Update();
	PowerRing *powerRing;
	DesperationOrb *despOrb;
	sf::Vertex orbQuad[4];
	Tileset *ts_powerOrb;
	//sf::CircleShape statusCircle;
};

#endif
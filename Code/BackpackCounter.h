#ifndef __BACKPACK_COUNTER_H_
#define __BACKPACK_COUNTER_H_

#include <SFML/Graphics.hpp>

struct TilesetManager;
struct Tileset;
struct Session;

struct BackpackCounter
{
	int currLevel;
	sf::Text levelText;
	sf::Shader ringShader;

	Session *sess;

	Tileset *ts_ring;
	Tileset *ts_cage;
	Tileset *ts_backpack;

	sf::Vertex backpackQuad[4];
	sf::Vertex cageQuad[4];
	sf::Vertex ringQuad[4];

	int pointsToAdd;

	float scale;

	int numDivisions;
	int numDivisionsToFill;
	int numParticlesPerDivision;
	int currParticlePoints;
	double currFactor;

	bool showContainer;

	BackpackCounter(TilesetManager *tm);
	void SetCenter(sf::Vector2f pos);
	void SetScale(float f);
	void SetSession(Session *p_sess);
	void Reset();
	void Update();
	int GetTotalPotentialPoints();
	void SetLevel(int lev);
	void AddParticle();
	void SetContainerVisible(bool on);
	void Set(BackpackCounter *b);
	void Draw(sf::RenderTarget *target);
};

#endif
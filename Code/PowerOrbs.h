#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include <map>

struct GameSession;
struct PowerOrbs
{
	
	enum OrbColor
	{
		TEAL0,
		TEAL1,
		TEAL2,
		TEAL3,
		TEAL4,
		TEAL5,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		Count
	};

	PowerOrbs( GameSession *owner, bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires );
	void Draw( sf::RenderTarget *target );

	Tileset *ts_largeOrbs;
	Tileset *ts_smallOrbs;
	//sf::Texture largeOrbTex;
	//sf::Texture smallOrbTex;
	sf::Sprite largeOrb;

	void SetStarPositions( OrbColor oc );
	OrbColor orbColors[6];

	sf::Vector2i * starPositions[7];

	std::map<OrbColor, int> numStars;
	int activeOrb;
	//int numOrbStars[OrbColor::Count];
	int currStars;
	Tileset * ts_charges[OrbColor::Count];
	sf::VertexArray smallOrbVA;
	sf::Vector2f basePos;
};
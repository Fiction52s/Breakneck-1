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
	void UpdateStarVA();
	void Reset();

	Tileset *ts_largeOrbs;
	Tileset *ts_smallOrbs;
	//sf::Texture largeOrbTex;
	//sf::Texture smallOrbTex;
	sf::Sprite largeOrb;

	int testBlah;
	

	void SetStarPositions( int index, OrbColor oc );
	OrbColor orbColors[6];

	sf::Vector2f * starPositions[7];
	sf::Vertex *starVA[7];

	int activeStars;
	int starState;
	int starFrame;
	//int activeStars[];
	std::map<OrbColor, int> numStars;
	int activeOrb;
	//int numOrbStars[OrbColor::Count];
	int currStars;
	Tileset * ts_charge;
	sf::VertexArray smallOrbVA;
	sf::Vector2f basePos;
};
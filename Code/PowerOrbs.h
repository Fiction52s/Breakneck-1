#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include <map>

struct GameSession;

struct PowerWheel
{
	enum Mode
	{
		FILL,
		NORMAL,
		DESPERATION
	};

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

	PowerWheel( GameSession *owner, bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires );
	void Draw( sf::RenderTarget *target );
	//void UpdateStarVA();
	void Reset();
	sf::VertexArray *CreateSectionVA( OrbColor col,
		float radius );
	
	void Hide( bool hide, int frames );
	void UpdateHide();
	bool shifting;
	bool hiding;
	int hideFrame;
	int hideLength;

	bool Damage( int power );
    bool Use( int power );
	void Recover( int power );
	void Charge( int power );
	void UpdateSections();
	void UpdateSwivel();
	void OrbUp();
	int swivelFrame;
	int swivelLength;
	int swivelLengthFill;
	bool swivelingUp;
	bool swivelingDown;
	float swivelStartAngle;

	int lifeTextureFrame;
	int lifeTextureMultiple;

	Mode mode;

	float radius;

	Tileset *ts_largeOrbs;
	Tileset *ts_smallOrbs;
	Tileset *ts_lifeTexture;
	//sf::Texture largeOrbTex;
	//sf::Texture smallOrbTex;
	sf::Sprite largeOrb;
	sf::Shader lifeTextureShader;

	void UpdateSpritePositions();

	int testBlah;
	void UpdateSmallOrbs();
	void SetVisibleSections( int orbIndex, int visSections,
		int currentLevel );
	void SetVisibleCurrentSection( int orbIndex, int currentSection, float radius );
	
	sf::Sprite orbPointer;
	Tileset *ts_orbPointer;

	sf::VertexArray *orbSectionVA[6];
	sf::VertexArray partialSectionVA;
	
	//Tileset *ts_lifeStop;
	//sf::Sprite lifeStop;


	//void SetStarPositions( int index, OrbColor oc );
	OrbColor orbColors[6];
	sf::Color orbHues[OrbColor::Count];

	//sf::Vector2f * starPositions[7];
	//sf::Vertex *starVA[7];

	//int activeStars;
	//int starState;
	//int starFrame;
	//int activeStars[];
	std::map<OrbColor, int> numSections;
	int activeOrb;
	int activeSection;
	int activeLevel;
	//int numOrbStars[OrbColor::Count];
	//int currStars;
	//Tileset * ts_charge;
	sf::VertexArray smallOrbVA;
	sf::Vector2f basePos;
	sf::Vector2f origBasePos;
};

//struct PowerRingSection
//{
//	PowerRingSection(
//		const sf::Color &active, const sf::Color &remove,
//		const sf::Color &empty, int p_width, int p_maxPower);
//	sf::Color activeColor;
//	sf::Color removeColor;
//	sf::Color emptyColor;
//	void SetupSection(int startRadius);
//	void Update();
//	void Reset();
//	
//	int width;
//	int prevPower;
//	sf::Vertex *va;
//	int maxPower;
//	int currPower;
//	//amount behind curr to color
//	int currRemovingPower;
//	int GetDivsActive();
//	int GetDivsRemoved();
//	int GetDivsEmpty();
//	int Damage(int dmg);
//	int Heal(int heal);
//
//
//	int numEmptyDivs;
//	int numRemovedDivs;
//};
//
//struct PowerRing
//{
//	PowerRing( GameSession *owner, int numRings,
//		PowerRingSection *rings, int *spacing );
//	void SetPowers( bool hasAirDash,
//		bool hasGravReverse,
//		bool hasBounce,
//		bool hasGrind,
//		bool hasTimeSlow,
//		bool hasWires );
//
//	PowerRingParams *rings;
//	int *spacing;
//
//
//	sf::VertexArray *ringVA;
//	sf::VertexArray *middleVA;
//	sf::CircleShape scorpTest;
//	sf::CircleShape keyTest;
//	sf::Vector2f centerPos;
//	void CreateRing();
//	void Draw( sf::RenderTarget *target );
//
//};

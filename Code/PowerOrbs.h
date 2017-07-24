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

struct PowerRingSection
{
	enum RingType
	{
		NORMAL
	};

	PowerRingSection(TilesetManager &tm,
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

struct PowerRing
{
	PowerRing( sf::Vector2f &pos,int numRings,
		PowerRingSection **rings);
	~PowerRing();
	void SetPowers( bool hasAirDash,
		bool hasGravReverse,
		bool hasBounce,
		bool hasGrind,
		bool hasTimeSlow,
		bool hasWires );

	PowerRingSection **rings;
	int numRings;
	int currRing;
	int Fill(int fill);
	int Drain(int drain);
	void ResetEmpty();
	void ResetFull();
	bool IsFull();

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

#ifndef __MOVING_GEO_H__
#define __MOVING_GEO_H__

#include <SFML\Graphics.hpp>
#include "Movement.h"
#include <list>
#include "Tileset.h"


sf::Color GetBlendColor(
	sf::Color c0,
	sf::Color c1,
	float progress);




struct MovingGeo
{
	struct MovingGeoBasicData
	{
		int state;
		int frame;
		bool done;
	};

	MovingGeoBasicData data;
	sf::Vertex *points;
	sf::Color color;
	sf::Vector2f basePos;

	MovingGeo();
	~MovingGeo();
	void Clear();
	virtual void Init() { Reset(); };
	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual void SetColor(sf::Color c);
	virtual int GetNumPoints() = 0;
	virtual void SetPoints(sf::Vertex *p);
	virtual void SetBase(sf::Vector2f &base);
	virtual int GetNumStoredBytes();
	virtual void StoreBytes(unsigned char *bytes);
	virtual void SetFromBytes(unsigned char *bytes);
};

struct MovingGeoGroup
{
	//rollback
	struct MyData
	{
		bool running;
		int frame;
	};

	std::list<MovingGeo*> geoList;
	std::list<int> waitFrames;
	sf::Vertex *points;
	int numTotalPoints;
	MyData data;

	MovingGeoGroup();
	~MovingGeoGroup();
	void Start();
	void Reset();
	bool Update();
	void AddGeo(MovingGeo *mg, int waitFrames = 0);
	void Init();
	void Draw(sf::RenderTarget *target);
	void SetBase(sf::Vector2f &pos);
	void RemoveAll();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


struct SpecialMovingGeoGroup
{
	SpecialMovingGeoGroup();
	virtual ~SpecialMovingGeoGroup();
	void Start();
	void Reset();
	virtual bool Update();
	virtual void StartGeo() = 0;
	void AddGeo(MovingGeo *mg);
	void Init();
	void Draw(sf::RenderTarget *target);
	void SetBase(sf::Vector2f &pos);
	void RemoveAll();
	void StopGenerating();

	bool continueGenerating;

	std::vector<MovingGeo*> geoVector;
	int frame;
	sf::Vertex *points;
	int numTotalPoints;
	bool running;
};

struct PokeTriangleScreenGeoGroup : SpecialMovingGeoGroup
{
	PokeTriangleScreenGeoGroup();
	~PokeTriangleScreenGeoGroup();
	
	void SetLengthFactor(float f);
	void StartGeo();

	float lengthFactor;
};

struct SpinningTri : MovingGeo
{
	enum State
	{
		S_EXPANDING,
		S_GROW,
		S_ROTATE,
		S_ROTATE_AND_FADE,
		S_Count
	};

	int stateLength[S_Count];
	int maxLength;
	float length;
	float width;
	float angle;
	float startAngle;
	sf::Color startColor;
	sf::Color fadeColor;

	float finalWidth;
	float startWidth;

	SpinningTri(float startAngle);
	void Reset();
	void Update();

	void SetColorGrad(sf::Color startCol,
		sf::Color endCol);
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);
	int GetNumPoints() { return 4; }
	void UpdatePoints();
};

struct Laser: MovingGeo
{
	enum State
	{
		S_VERTICALGROW,
		S_WIDEN,
		S_SHRINK,
		S_SLOWGROW,
		S_DISAPPEAR,
		S_Count
	};

	int stateLength[S_Count];
	int maxHeight;
	float height;
	float currWidth;
	float angle;
	float startAngle;
	sf::Color startColor;
	float growWidth;
	float shrinkWidth;
	float startWidth;
	sf::Vector2f center;

	Laser( float startWidth, float growWidth, float shrinkWidth, float startAngle,
		sf::Color startColor );
	void Reset();
	void Update();

	void SetColor(sf::Color c);
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);
	void SetHeight(float h);
	void SetWidth(float w);
	int GetNumPoints() { return 8; }
};

struct Ring : MovingGeo
{
	bool ownsPoints;
	float innerRadius;
	float outerRadius;
	sf::Shader *shader;
	sf::Vector2f position;
	int circlePoints;

	Ring( int p_circlePoints );
	~Ring();
	virtual void Reset() {};
	virtual void Update() {};
	void SetPoints(sf::Vertex *p);
	void CreatePoints();
	void Draw(sf::RenderTarget *target);
	void UpdatePoints();
	void Set(sf::Vector2f pos,
		float innerR, float ringWidth);
	void SetShader(sf::Shader *sh);
	int GetNumPoints() { return circlePoints * 4; }
};

struct MovingRing : Ring
{	
	float startInner;
	float startWidth;
	float endInner;
	float endWidth;
	sf::Color startColor;
	sf::Color endColor;

	sf::Vector2f startPos;
	sf::Vector2f endPos;

	int totalFrames;

	CubicBezier sizeBez;
	CubicBezier colorBez;
	CubicBezier posBez;
	CubicBezier innerBez;

	MovingRing(int p_circlePoints,
		float p_startInner,
		float p_endInner,
		float p_startWidth,
		float p_endWidth,
		sf::Vector2f p_startPos,
		sf::Vector2f p_endPos,
		sf::Color p_startCol,
		sf::Color p_endCol,
		int totalFrames);
	void Reset();
	void Update();
};

struct PokeTri : MovingGeo
{
	enum State
	{
		S_POKING,
		S_SHRINKING,
		S_Count
	};

	int stateLength[S_Count];
	int maxLength;
	float length;
	float width;
	float angle;
	float pokeAngle;
	sf::Color startColor;
	sf::Color fadeColor;
	sf::Vector2f offset;
	float lengthFactor;
	float startWidth;


	PokeTri(sf::Vector2f &offset );
	void Reset();
	void Update();
	int GetNumPoints() { return 4; }
	void UpdatePoints();
	void SetLengthFactor(float f);
};

#endif
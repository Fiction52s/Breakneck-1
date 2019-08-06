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

struct ShapeEmitter;

struct ShapeParticle
{
	ShapeParticle(int numPoints, sf::Vertex *v,
		ShapeEmitter *emit);
	
	void Activate(float radius,
		sf::Vector2f &pos,
		sf::Vector2f &vel,
		float angle, int ttl,
		sf::Color c = sf::Color::White );
	void SetTileIndex(int ti);
	virtual void SpecialActivate() {};
	virtual void SpecialUpdate() {};
	virtual void UpdateColor() {};
	//void SetEndColor(sf::Color &c );
	bool Update();
	void Clear();
	void SetColor(sf::Color &c);
	sf::Vector2f vel;
	sf::Vector2f pos;
	float angle;
	sf::Vertex *points;
	int numPoints;
	float radius;
	int ttl;
	sf::Color color;
	int tileIndex;
	ShapeEmitter *emit;

};

struct FadingParticle : ShapeParticle
{
	enum Action
	{
		FADEIN,
		NORMAL,
		FADEOUT,
	};
	FadingParticle(int numPoints,
		sf::Vertex *v,
		ShapeEmitter *emit);
	void SpecialUpdate();
	void SpecialActivate();
	void SetColorShift( sf::Color &start,
		sf::Color &end, int fadeInFrames,
		int fadeOutFrames);
	float GetNormalPortion();
	//void SetSizeShift( )
	
	Action action;
	int fadeOutThresh;
	int fadeInThresh;
	int startAlpha;

	void UpdateColor();
	sf::Color startColor;
	sf::Color endColor;

	int maxTimeToLive;
	
};

struct ShapeEmitter
{
	enum ParticleType
	{
		NORMAL,
		FADE
	};
	

	ShapeEmitter(int pointsPerShape,
		int numShapes, float angle,
		float angleRange,
		float minSpeed, float maxSpeed);
	~ShapeEmitter();
	void SetPos(sf::Vector2f &pos);
	virtual sf::Vector2f GetSpawnPos();
	void SetTileset(Tileset *ts);
	void SetColor(sf::Color &c);
	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);
	virtual void ActivateParticle( int index );
	void AddForce(sf::Vector2f &f);
	void ClearForces();
	void SetForce(sf::Vector2f &f);
	void SetOn(bool on);
	
	
	int pointsPerShape;
	int numShapesTotal;
	int numPoints;
	float angle;
	float angleRange;
	float minSpeed;
	float maxSpeed;
	sf::Vector2f pos;
	sf::Vertex *points;
	ShapeParticle **particles;
	int frame;
	int numActive;
	bool IsDone();

	sf::Vector2f accel;
	bool emitting;
	void SetRatePerSecond(int rate);
	int ratePerSecond;
	Tileset *ts;

	ParticleType pType;

	float lastCreationTime;

	ShapeEmitter *next;
};

struct BoxEmitter : ShapeEmitter
{
	BoxEmitter(int pointsPerShape,
		int numShapes, float angle,
		float angleRange,
		float minSpeed, float maxSpeed,
		float w, float h );

	sf::Vector2f GetSpawnPos();
	void SetRect(float w, float h);

	int width;
	int height;
};



struct MovingGeo
{
	MovingGeo();
	~MovingGeo();
	void Clear();
	virtual void Init() { Reset(); };
	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual void SetColor(sf::Color c);
	virtual int GetNumPoints() = 0;
	virtual void SetPoints(sf::Vertex *p);
	void SetBase(sf::Vector2f &base);
	sf::Vertex *points;
	sf::Color color;
	sf::Vector2f basePos;
	bool done;
};

struct MovingGeoGroup
{
	~MovingGeoGroup();
	void Reset();
	bool Update();
	void AddGeo(MovingGeo *mg, int waitFrames = 0);
	void Init();
	void Draw(sf::RenderTarget *target);
	void SetBase(sf::Vector2f &pos);

	std::list<MovingGeo*> geoList;
	int frame;
	std::list<int> waitFrames;
	sf::Vertex *points;
	int numTotalPoints;
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


	SpinningTri(float startAngle);
	void Reset();
	void Update();

	void SetColorGrad(sf::Color startCol,
		sf::Color endCol);
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);
	int GetNumPoints() { return 4; }
	void UpdatePoints();
	int stateLength[S_Count];
	int maxLength;
	State state;
	int frame;

	float length;
	float width;
	float angle;
	float startAngle;
	sf::Color startColor;
	sf::Color fadeColor;

	float finalWidth;
	float startWidth;
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


	Laser(float startAngle);
	void Reset();
	void Update();

	void SetColor(sf::Color c);
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);
	void SetHeight(float h);
	void SetWidth(float w);
	int GetNumPoints() { return 8; }

	int stateLength[S_Count];
	int maxHeight;
	State state;
	int frame;

	float height;
	float width;
	float angle;
	float startAngle;
	sf::Color startColor;

	float growWidth;
	float shrinkWidth;
	float startWidth;
	sf::Vector2f center;
};

struct Ring : MovingGeo
{
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
	bool ownsPoints;
	float innerRadius;
	float outerRadius;
	sf::Shader *shader;
	sf::Vector2f position;
	int circlePoints;
};

struct MovingRing : Ring
{	
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
	//void UpdatePoints();
	//void SetColor(sf::Color c);
	//void SetShader(sf::Shader *sh);

	float startInner;
	float startWidth;
	float endInner;
	float endWidth;
	sf::Color startColor;
	sf::Color endColor;

	sf::Vector2f startPos;
	sf::Vector2f endPos;

	int frame;
	int totalFrames;

	CubicBezier sizeBez;
	CubicBezier colorBez;
	CubicBezier posBez;
	CubicBezier innerBez;

	

	
};

#endif
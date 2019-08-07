
#ifndef __PARTICLEEFFECTS_H__
#define __PARTICLEEFFECTS_H__

#include <SFML/Graphics.hpp>

struct ShapeEmitter;
struct Tileset;
struct ShapeParticle;

struct PSizeUpdater
{
	virtual void PUpdateSize(ShapeParticle*) = 0;
};

struct PColorUpdater
{
	virtual void PUpdateColor(ShapeParticle*) = 0;
};

struct PAngleUpdater
{
	virtual void PUpdateAngle(ShapeParticle*) = 0;
};

struct PTileUpdater
{
	virtual void PUpdateTile(ShapeParticle*) = 0;
};

struct PPosUpdater
{
	virtual void PUpdatePos(ShapeParticle*) = 0;
};

struct ShapeParticle
{
	ShapeParticle(int numPoints, sf::Vertex *v,
		ShapeEmitter *emit);
	~ShapeParticle();
	virtual void Activate(float radius,
		sf::Vector2f &pos,
		float angle, int ttl,
		sf::Color c = sf::Color::White);

	void Clear();
	bool Update();
	void SetTileIndex(int ti);
	void SetColor(sf::Color &c);

	PPosUpdater *posUpdater;
	PSizeUpdater *sizeUpdater;
	PColorUpdater *colorUpdater;
	PAngleUpdater *angleUpdater;
	

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
	void SetColorShift(sf::Color &start,
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

struct PosSpawner
{
	virtual sf::Vector2f GetSpawnPos(ShapeEmitter *emit) = 0;
};

struct ColorSpawner
{
	virtual sf::Color GetSpawnColor(ShapeEmitter *emit) = 0;
};

struct AngleSpawner
{
	virtual float GetSpawnAngle(ShapeEmitter *emit) = 0;
};

struct RadiusSpawner
{
	virtual float GetSpawnRadius(ShapeEmitter *emit) = 0;
};

struct TTLSpawner
{
	virtual int GetSpawnTTL(ShapeEmitter *emit) = 0;
};

struct PosUpdater
{

};

struct ColorUpdater
{

};

struct ShapeEmitter
{
	enum ParticleType
	{
		NORMAL,
		FADE
	};


	ShapeEmitter(int pointsPerShape,
		int numShapes);
	~ShapeEmitter();

	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);

	void SetPos(sf::Vector2f &pos);
	void SetTileset(Tileset *ts);

	void SetRatePerSecond(int rate);
	virtual void ActivateParticle(int index);
	sf::Vector2f GetSpawnPos();
	virtual sf::Vector2f GetSpawnVel();
	virtual sf::Color GetSpawnColor();
	virtual float GetSpawnAngle();
	virtual float GetSpawnRadius();
	virtual int GetSpawnTTL();


	virtual ShapeParticle * CreateParticle(int index);

	PosSpawner *posSpawner;
	ColorSpawner *colorSpawner;
	RadiusSpawner *radiusSpawner;
	AngleSpawner *angleSpawner;
	TTLSpawner *ttlSpawner;


	void SetOn(bool on);
	bool IsDone();

	int pointsPerShape;
	int numShapesTotal;
	int numPoints;
	sf::Vector2f pos;
	sf::Vertex *points;
	ShapeParticle **particles;
	int frame;
	int numActive;
	bool emitting;

	int ratePerSecond;
	Tileset *ts;

	ParticleType pType;

	float lastCreationTime;

	ShapeEmitter *next;
};

struct BoxEmitter : ShapeEmitter
{
	BoxEmitter(int pointsPerShape,
		int numShapes, float w, float h);

	sf::Vector2f GetSpawnPos();
	void SetRect(float w, float h);

	int width;
	int height;
};


struct BoxPosSpawner : PosSpawner
{
	BoxPosSpawner(int w, int h);
	sf::Vector2f GetSpawnPos(ShapeEmitter *emit);
	void SetRect(int w, int h);
	int width;
	int height;
};

#endif
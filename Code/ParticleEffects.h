
#ifndef __PARTICLEEFFECTS_H__
#define __PARTICLEEFFECTS_H__

#include <SFML/Graphics.hpp>

struct ShapeEmitter;
struct Tileset;
struct ShapeParticle;
struct GameSession;

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
	enum Action
	{
		FADEIN,
		NORMAL,
		FADEOUT,
	};

	ShapeParticle(int numPoints, sf::Vertex *v,
		ShapeEmitter *emit);
	~ShapeParticle();
	virtual void Activate(float radius,
		sf::Vector2f &pos,
		float angle, int ttl,
		sf::Color c = sf::Color::White,
		int tileIndex = 0 );

	void Clear();
	bool Update();
	void SetTileIndex(int ti);
	void SetColor(sf::Color &c);
	void SetColorShift(sf::Color start,
		sf::Color end, int fadeInFrames,
		int fadeOutFrames);
	float GetNormalPortion();

	PPosUpdater *posUpdater;
	PSizeUpdater *sizeUpdater;
	PColorUpdater *colorUpdater;
	PAngleUpdater *angleUpdater;
	
	int action;

	sf::Vector2f pos;
	float angle;
	sf::Vertex *points;
	int numPoints;
	float radius;
	int ttl;
	sf::Color color;
	int tileIndex;
	ShapeEmitter *emit;

	int fadeOutThresh;
	int fadeInThresh;
	int startAlpha;
	int maxTimeToLive;

	
	sf::Color startColor;
	sf::Color endColor;

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
	void CreateParticles();
	~ShapeEmitter();

	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);

	void SetPos(sf::Vector2f &pos);
	void SetTileset(Tileset *ts);

	void SetRatePerSecond(int rate);

	sf::Vector2f GetSpawnPos();
	virtual sf::Vector2f GetSpawnVel();
	virtual sf::Color GetSpawnColor();
	virtual float GetSpawnAngle();
	virtual float GetSpawnRadius();
	virtual int GetSpawnTTL();
	virtual int GetSpawnTile();

	static float GetRandomAngle(float baseAngle,
		float angleRange);

	virtual void ActivateParticle(int index);
	virtual ShapeParticle * CreateParticle(int index);

	bool active; //in session
	PosSpawner *posSpawner;
	ColorSpawner *colorSpawner;
	RadiusSpawner *radiusSpawner;
	AngleSpawner *angleSpawner;
	TTLSpawner *ttlSpawner;

	void SetOn(bool on);
	virtual bool IsDone();

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

	ShapeEmitter *prev;
	ShapeEmitter *next;
};

struct BoxPosSpawner : PosSpawner
{
	BoxPosSpawner(int w, int h);
	virtual sf::Vector2f GetSpawnPos(ShapeEmitter *emit);
	void SetRect(int w, int h);
	int width;
	int height;
};

struct LinearVelPPosUpdater : PPosUpdater
{
	LinearVelPPosUpdater();
	virtual void PUpdatePos(ShapeParticle*);
	sf::Vector2f vel;
};

struct FeatherPosUpdater : PPosUpdater
{
	FeatherPosUpdater();
	virtual void PUpdatePos(ShapeParticle*);
	sf::Vector2f startVel;
	sf::Vector2f vel;
};

//struct LimitedAngleSpawner : 
//{
//	LimitedAngleSpawner(float angle, float range);
//	void SetAngleRange(float angle, float range);
//	virtual float GetSpawnAngle(ShapeEmitter *emit);
//
//	float angleRange;
//	float angle;
//};

struct LeafEmitter : ShapeEmitter
{
	LeafEmitter();
	int GetSpawnTTL();
	ShapeParticle * CreateParticle(int index);
	void ActivateParticle(int index);
	int GetSpawnTile();
};

struct Actor;
struct PlayerBoosterEffectEmitter : ShapeEmitter
{
	enum BoosterType
	{
		BOOSTER_GRAVITY_INCREASER,
		BOOSTER_GRAVITY_DECREASER,
		BOOSTER_MOMENTUM,
		BOOSTER_TIMESLOW,
		BOOSTER_HOMING,
		BOOSTER_ANTITIMESLOW,
		BOOSTER_FREEFLIGHT,
		BOOSTER_Count,
	};

	PlayerBoosterEffectEmitter( Actor *p_player, int p_boosterType );
	ShapeParticle * CreateParticle(int index);
	void ActivateParticle(int index);
	int GetSpawnTTL();
	int GetSpawnTile();

	int boosterType;
	float boostPortion;
	Actor *player;
};

struct GlideEmitter : ShapeEmitter
{
	GlideEmitter(GameSession *p_owner);
	int GetSpawnTTL();
	int GetSpawnTile();
	GameSession *owner;
	ShapeParticle * CreateParticle(int index);
	void ActivateParticle(int index);
	bool IsDone();
};

#endif

#ifndef __PARTICLEEFFECTS_H__
#define __PARTICLEEFFECTS_H__

#include <SFML/Graphics.hpp>

struct ShapeEmitter;
struct Tileset;
struct ShapeParticle;
struct GameSession;
struct Session;

struct ShapeParticle
{
	enum Action
	{
		FADEIN,
		NORMAL,
		FADEOUT,
	};

	struct MyData
	{
		int action;
		sf::Vector2f pos;
		sf::Vector2f vel;
		float angle;
		float radius;
		int ttl;
		sf::Color color;
		int tileIndex;
		int fadeOutThresh;
		int fadeInThresh;
		int startAlpha;
		int maxTimeToLive;
		sf::Color startColor;
		sf::Color endColor;
	};

	MyData data;

	sf::Vertex *points;
	int numPoints;
	ShapeEmitter *emit;

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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);


	
};

struct ShapeEmitter
{
	enum ParticleType
	{
		PARTICLE_BOOSTER_GRAVITY_INCREASER,
		PARTICLE_BOOSTER_GRAVITY_DECREASER,
		PARTICLE_BOOSTER_MOMENTUM,
		PARTICLE_BOOSTER_TIMESLOW,
		PARTICLE_BOOSTER_HOMING,
		PARTICLE_BOOSTER_ANTITIMESLOW,
		PARTICLE_BOOSTER_FREEFLIGHT,
		PARTICLE_BOOSTER_Count,
	};

	struct MyData
	{
		bool active; //in session
		int prevID;
		int nextID;
		float lastCreationTime;
		sf::Vector2f pos;
		int ratePerSecond;
		int frame;
		int numActive;
		//bool active; //in session
		bool emitting;


		float boostPortion; //for booster particles. seems easiest to just store it here for now
	};

	MyData data;
	ShapeEmitter *prev;
	ShapeEmitter *next;
	int emitterID;
	int particleType;
	Tileset *ts;
	sf::Vertex *points;
	ShapeParticle **particles;
	int numShapesTotal;
	int numPoints;
	int pointsPerShape;
	Session *sess;



	
	
	ShapeEmitter(int p_particleType);
	ShapeEmitter(int p_particleType, int p_maxParticles );//int pointsPerShape,
		//int numShapes);
	
	void CreateParticles();
	~ShapeEmitter();

	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);

	void SetPos(sf::Vector2f &pos);
	void SetTileset(Tileset *ts);

	void SetRatePerSecond(int rate);

	void SetIDAndAddToAllEmittersVec();

	sf::Vector2f GetBoxSpawnPos(int width, int height);

	static float GetRandomAngle(float baseAngle,
		float angleRange);

	virtual void ActivateParticle(int index);
	ShapeParticle * CreateParticle(int index);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	void SetOn(bool on);
	virtual bool IsDone();

private:
	void Init();
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
	void ActivateParticle(int index);
};

struct Actor;
struct PlayerBoosterEffectEmitter : ShapeEmitter
{
	Actor *player;

	PlayerBoosterEffectEmitter( Actor *p_player, int p_particleType );
	void ActivateParticle(int index);
};
#endif
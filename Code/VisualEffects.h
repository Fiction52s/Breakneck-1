#ifndef __VISUAL_EFFECTS_H
#define __VISUAL_EFFECTS_H

#include "ObjectPool.h"
#include <SFML\Graphics.hpp>

struct Actor;
struct Tileset;
struct EffectPool;

enum EffectType
{
	FX_REGULAR,
	FX_RELATIVE,
	FX_IND,
	FX_Count
};




struct EffectInstance : PoolMember
{
	EffectInstance()
		:PoolMember( -1 ), parent(NULL)
	{}
	EffectInstance(EffectPool *parent, int index);
	virtual void Init(sf::Vector2f &pos, 
		sf::Transform &tr, int frameCount, 
		int animFactor, 
		int startTile );
	~EffectInstance();
	virtual	void InitFromParams(EffectInstance *ei);
	virtual void SetParams(
		sf::Vector2f &p_pos, 
		sf::Transform &p_tr, 
		int p_frameCount, 
		int p_animFactor, 
		int p_startTile);
	virtual void ApplyTransform();
	virtual void SetVelocityParams(sf::Vector2f &vel,
		sf::Vector2f &accel, float maxVelocity );
	virtual void Clear();
	virtual Tileset *GetTileset();
	virtual void SetSubRect(sf::IntRect &ir);
	virtual bool Update();
	virtual void SetColor(sf::Color &c);
	int frame;
	sf::Color color;
	int animFactor;
	//int startFrame;
	int frameCount;
	int startTile;
	sf::Vector2f vel;
	sf::Vector2f accel;
	sf::Transform tr; 
	sf::Vector2f pos;
	float maxVel;
	EffectPool *parent;
};

struct IndEffectInstance : EffectInstance
{
	IndEffectInstance();
	sf::Sprite sprite;
	void SetTileset(Tileset *ts);
	void ApplyTransform();
	Tileset *GetTileset();
	void SetColor(sf::Color &c);
	void Draw(sf::RenderTarget * target);
	void SetSubRect(sf::IntRect &ir);
	void Clear();
	Tileset *ts;
	//float depth;
};

struct RelEffectInstance : EffectInstance
{
	RelEffectInstance()
		:EffectInstance()
	{}
	RelEffectInstance( EffectPool *parent, int index);
	~RelEffectInstance();
	virtual bool Update() override;
	virtual void Init(sf::Vector2f &pos,
		sf::Transform &tr, int frameCount,
		int animFactor,
		int startTile,
		sf::Vector2<double> *lock, int detachFrames = -1 );
	virtual	void InitFromParams(EffectInstance *ei) override;
	virtual void SetParams(
		sf::Vector2f &p_pos,
		sf::Transform &p_tr,
		int p_frameCount,
		int p_animFactor,
		int p_startTile,
		sf::Vector2<double> *lockP, int detachFrames = -1 );
	void ClearLockPos();
	sf::Vector2<double> *lockPos;
	int detachFrames;
};

struct EffectPoolUpdater
{
	virtual void UpdateEffect(EffectInstance *ei) = 0;
};

struct RisingParticleUpdater : EffectPoolUpdater
{
	RisingParticleUpdater(Actor *p);
	Actor *parent;
	void UpdateEffect(EffectInstance *ei);
};

struct EffectPool : ObjectPool
{
	EffectPool( EffectType et, int maxNumFX, float depth = 1.f );
	virtual ~EffectPool();
	void Reset();
	void Update( EffectPoolUpdater *epu = NULL );
	void DeactivateMember(PoolMember *pm);
	void SetTileset(Tileset *ts);
	virtual EffectInstance * ActivateEffect( EffectInstance *params );
	virtual IndEffectInstance * ActivateIndEffect(EffectInstance *params,
		Tileset *ts );
	void Draw( sf::RenderTarget *target );
	Tileset *ts;
	sf::Vertex *va;
	sf::Shader *effectShader;
	float depth;
	EffectType eType;
	int maxNumFX;
};

struct VertexBufMember
{
	VertexBufMember()
	{
		Reset();
	}
	void Reset();
	sf::Vector2f pos;
	float angle;
	sf::Vector2f scale;
	int tileIndex;
	sf::Color color;
};

struct VertexBuf
{
	VertexBuf(int numVertices, sf::PrimitiveType pType );
	~VertexBuf();
	void SetScale(int index, sf::Vector2f &scale);
	void SetPosition(int index, sf::Vector2f &pos);
	void SetRotation(int index, double angle ); //ccw
	void RotateMemberCW( int index, float angle);
	const VertexBufMember const &GetMemberInfo( int index );
	void SetTileset(Tileset *p_ts);
	void Draw(sf::RenderTarget *rt, sf::Shader *shader = NULL );
	void UpdateVertices();
	void SetTile(int index, int tileIndex);
	void SetNumActiveMembers(int activeMembers);
	void SetColor(int index, sf::Color &color);
	void Reset();

	//int numVertices;
	//int numMembers;
	sf::Vertex *vb;
	Tileset *ts;
	sf::PrimitiveType pType;
	int verticesPerMember;
	int numMembers;
	int numActiveMembers;
	VertexBufMember *members;
	bool *dirty;
	bool dirtyTS;
};

#endif
#ifndef __VISUAL_EFFECTS_H
#define __VISUAL_EFFECTS_H

#include "ObjectPool.h"
#include <SFML\Graphics.hpp>


struct Tileset;
struct EffectPool;

enum EffectType
{
	FX_REGULAR,
	FX_RELATIVE,
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
	virtual	void InitFromParams(EffectInstance *ei);
	virtual void SetParams(
		sf::Vector2f &p_pos, 
		sf::Transform &p_tr, 
		int p_frameCount, 
		int p_animFactor, 
		int p_startTile);
	void Clear();
	virtual bool Update();
	int frame;
	int animFactor;
	int startFrame;
	int frameCount;
	int startTile;
	sf::Transform tr;
	sf::Vector2f pos;
	EffectPool *parent;
};

struct RelEffectInstance : EffectInstance
{
	RelEffectInstance()
		:EffectInstance()
	{}
	RelEffectInstance( EffectPool *parent, int index);
	virtual bool Update() override;
	virtual void Init(sf::Vector2f &pos,
		sf::Transform &tr, int frameCount,
		int animFactor,
		int startTile,
		sf::Vector2<double> *lock );
	virtual	void InitFromParams(EffectInstance *ei) override;
	virtual void SetParams(
		sf::Vector2f &p_pos,
		sf::Transform &p_tr,
		int p_frameCount,
		int p_animFactor,
		int p_startTile,
		sf::Vector2<double> *lockP );
	void ClearLockPos();
	sf::Vector2<double> *lockPos;
};

struct EffectPool : ObjectPool
{
	EffectPool( EffectType et, int maxNumFX, float depth = 1.f );
	void Reset();
	void Update();
	
	void SetTileset(Tileset *ts);
	virtual EffectInstance * ActivateEffect( EffectInstance *params );
	void Draw( sf::RenderTarget *target );
	Tileset *ts;
	sf::Vertex *va;
	sf::Shader *effectShader;
	float depth;
	EffectType eType;
	int maxNumFX;
};

struct VertexBufferMember
{
	VertexBufferMember()
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

struct VertexBuffer
{
	VertexBuffer(int numVertices, sf::PrimitiveType pType );
	~VertexBuffer();
	void SetScale(int index, sf::Vector2f &scale);
	void SetPosition(int index, sf::Vector2f &pos);
	void SetRotation(int index, double angle ); //ccw
	void RotateMemberCW( int index, float angle);
	const VertexBufferMember const &GetMemberInfo( int index );
	void SetTileset(Tileset *p_ts);
	void Draw(sf::RenderTarget *rt, sf::Shader *shader = NULL );
	void UpdateVertices();
	void SetTile(int index, int tileIndex);
	void SetNumActiveMembers(int activeMembers);
	void SetColor(int index, sf::Color &color);
	void Reset();

	int numVertices;
	sf::Vertex *vb;
	Tileset *ts;
	sf::PrimitiveType pType;
	int verticesPerMember;
	int numMembers;
	int numActiveMembers;
	VertexBufferMember *members;
	bool *dirty;
	bool dirtyTS;
};

#endif
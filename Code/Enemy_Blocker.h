#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"

struct BlockerChain;

struct CircleGroup
{
	CircleGroup(int num, int rad, sf::Color col, int pointsPerCircle );
	~CircleGroup();
	int numCircles;
	sf::Vertex *va;
	float radius;
	sf::Color color;
	void SetVisible(int index, bool vis);
	void SetPosition(int index, sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void ShowAll();
	void HideAll();
	int pointsPerCircle;
	int numVerts;
	sf::Vector2f *circleCenters;

};


struct Blocker : Enemy, QuadTreeEntrant
{
	enum Action
	{
		WAIT,
		MALFUNCTION,
		HITTING,
		EXPLODE,
		Count
	};

	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsFastDying();
	void ProcessState();
	Blocker(BlockerChain *bc, sf::Vector2i &pos, int index);
	void DrawMinimap(sf::RenderTarget *target);
	void ClearSprite();
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	void IHitPlayer(int index);

	int actionLength[Count];
	int animFactor[Count];

	Action action;

	int animationFactor;

	int minimapCirclePoints;
	int minimapCircleRadius;

	BlockerChain *bc;

	bool checkCol;

	int vaIndex;
};

struct BlockerChain : Enemy
{
	enum BlockerType
	{
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA
	};

	sf::Vertex *va;
	CircleGroup *circleGroup;
	BlockerChain(GameSession *owner,
		sf::Vector2i &pos, std::list<sf::Vector2i> &path,
		int bType, bool armored, int spacing = 0);
	~BlockerChain();
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void SetZone(Zone *p_zone);
	int GetNumCamPoints();
	V2d GetCamPoint(int index);
	void UpdatePhysics( int substep );
	void UpdatePrePhysics();
	void DebugDraw(sf::RenderTarget *target);
	void ProcessState();
	void UpdatePostPhysics();
	void UpdateEnemyPhysics();
	int animationFactor;
	int liveFrames;
	Blocker **blockers;
	int numBlockers;

	Tileset *ts;
	void ResetEnemy();

	bool checkCol;
	

	bool armored;
	BlockerType bType;
};

#endif
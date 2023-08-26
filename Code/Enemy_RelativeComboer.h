#ifndef __ENEMY_RELATIVECOMBOER_H__
#define __ENEMY_RELATIVECOMBOER_H__

#include "Enemy.h"

struct ComboObject;

struct RelativeComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_ATTACHEDWAIT,
		S_WAIT,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int numKilledTextNumber;
		int latchFrame;
		int specialPauseFrames;
		V2d velocity;
		int currHits;
		int numKilled;
		int waitFrame;
		bool latchedOn;
		V2d basePos;
		V2d offsetPos;
		V2d offsetDest;
	};
	MyData data;

	LineMovement *lineMovement;
	MovementSequence flySeq;
	bool limitedKills;
	sf::Text numKilledText;

	int maxLatchFrames;
	int hitLimit;
	Tileset *ts;
	bool detachOnKill;
	int maxKilled;
	int maxWaitFrames;
	double flySpeed;

	CubicBezier flyBez;
	
	bool CountsForEnemyGate() { return false; }
	RelativeComboer(
		ActorParams *ap);/*GameSession *owner, bool hasMonitor,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &path, int p_level,
		int juggleReps, bool detachOnKill*/
	~RelativeComboer();
	void SetLevel(int lev);
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	bool CanBeHitByComboer();
	void UpdateKilledNumberText(int reps);

	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void DirectKill();

	void Throw(double a, double strength);
	void Throw(V2d vel);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif
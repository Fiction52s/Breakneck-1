#ifndef __OBJECTPROPERTIES_H__
#define __OBJECTPROPERTIES_H__

#include "VectorMath.h"
#include "QuadTree.h"
#include "Physics.h"
#include "EnemyTypes.h"

struct Tileset;
struct Actor;
struct HitboxInfo;
struct Enemy;

struct Movable : QuadTreeCollider
{
	Movable();
	virtual void HandleEntrant(QuadTreeEntrant *qte);
	virtual void UpdatePrePhysics();
	virtual void Reset(
		V2d &pos);
	virtual void UpdatePhysics();
	virtual void UpdatePostPhysics();
	bool ResolvePhysics(
		V2d vel);
	virtual void HitTerrain();
	virtual void HitPlayer();
	virtual void IncrementFrame();

	CollisionBox physBody;
	CollisionBox hitBody;
	CollisionBox hurtBody;

	bool collideWithTerrain;
	bool collideWithPlayer;

	V2d position;
	V2d velocity;
	int slowCounter;
	int slowMultiple;

	bool col;
	Contact minContact;
	V2d tempVel;
	static GameSession *owner;
};



struct SlowableObject
{
	int slowCounter;
	int slowMultiple;


	SlowableObject()
	{
		isSlowable = true;
	}

	void SetSlowable(bool slowable)
	{
		isSlowable = slowable;
	}

	virtual bool IsSlowed(int playerIndex) = 0;
	void ResetSlow()
	{
		slowCounter = 1;
		slowMultiple = 1;
	}
	void SlowCheck(int playerIndex)
	{
		if (!isSlowable)
			return;

		if (IsSlowed(playerIndex))
		{
			if (slowMultiple == 1)
			{
				slowCounter = 1;
				slowMultiple = 5;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		}
	}
	bool UpdateAccountingForSlow()
	{
		if (slowCounter == slowMultiple)
		{
			slowCounter = 1;
			return true;
		}
		else
		{
			slowCounter++;
			return false;
		}
	}
private:
	bool isSlowable;
};

struct HittableObject
{
	HittableObject() :receivedHit(NULL) {}
	virtual HitboxInfo * IsHit(Actor *player) = 0;
	const bool ReceivedHit() { return receivedHit; }
	bool CheckHit(Actor *player, Enemy* e);
	virtual void ProcessHit() = 0;
	HitboxInfo *receivedHit;
	Enemy *comboHitEnemy;
	int numHealth;
	bool specterProtected;
};

struct CuttableObject
{
	CuttableObject();
	sf::Vertex quads[2 * 4];
	sf::Vector2f splitDir;
	int separateFrame;
	int totalSeparateFrames;
	float separateSpeed;
	bool active;
	void SetCutRootPos(sf::Vector2f &p_rPos);
	void Reset();
	bool DoneSeparatingCut();
	void IncrementFrame();
	void Setup(
		Tileset *ts, int frontIndex,
		int backIndex, double scale,
		double angle = 0, bool flipX = false, bool flipY = false);
	void SetFlipHoriz(bool h)
	{
		flipHoriz = h;
	}
	void SetFlipVert(bool v)
	{
		flipVert = v;
	}
	void UpdateCutObject(int slowCounter);
	void SetScale(float scale);
	int rectWidth;
	int rectHeight;

	bool flipHoriz;
	bool flipVert;
	void Draw(sf::RenderTarget *target);
	Tileset *ts;
	int tIndexFront;
	int tIndexBack;
	void SetSubRectFront(int fIndex);
	void SetSubRectBack(int bIndex);
	void SetTileset(Tileset *ts);
	void SetRotation(float rotate);
	float rotateAngle;
	float scale;

private:
	sf::Vector2f rootPos;
};

struct ComboObject
{
	ComboObject(Enemy *en)
		:enemyHitboxFrame(-1),
		nextComboObj(NULL),
		enemyHitboxInfo(NULL),
		enemy(en), enemyHitBody( CollisionBox::Hit )
	{
	}
	~ComboObject();
	void Reset();
	V2d GetComboPos();
	CollisionBody enemyHitBody;
	HitboxInfo *enemyHitboxInfo;
	int enemyHitboxFrame;
	ComboObject *nextComboObj;
	void ComboHit();
	void Draw(sf::RenderTarget *target);
	Enemy *enemy;
	bool active;
};

#endif
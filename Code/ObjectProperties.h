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

	Session *sess;
	//static GameSession *owner;
};

struct ChainableObject
{
	virtual void SetStartPosition(V2d &pos) = 0;
};

struct SlowableObject
{
	int slowCounter;
	int slowMultiple;
	bool isSlowable;

	SlowableObject()
	{
		isSlowable = true;
	}
	void SetSlowable(bool slowable)
	{
		isSlowable = slowable;
	}

	virtual bool IsSlowed(int playerIndex) { return false; }
	virtual int GetSlowFactor(int playerIndex) { return 1; }
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
				
				//GetBubbleTimeFactor()
				slowMultiple = GetSlowFactor(playerIndex);//5;
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
};

struct HittableObject
{
	HitboxInfo receivedHit;
	int receivedHitPlayerIndex;
	int comboHitEnemyID;
	int numHealth;
	bool specterProtected;

	HittableObject();
	virtual HitboxInfo * IsHit(int pIndex);
	const bool HasReceivedHit();
	bool CheckHit(Actor *player, Enemy* e);
	int GetReceivedHitPlayerIndex();
	virtual void ProcessHit();
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
	CollisionBody enemyHitBody;
	
	struct MyData
	{
		bool active;
		int enemyHitboxFrame;
		ComboObject *nextComboObj;
	};

	MyData data;
	HitboxInfo *enemyHitboxInfo;
	Enemy *enemy;
	int comboObjectID;

	ComboObject::ComboObject(Enemy *en);
	~ComboObject();
	void SetIDAndAddToAllComboObjectsVec();
	void Reset();
	V2d GetComboPos();
	void ComboHit();
	void Draw(sf::RenderTarget *target);
	
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif
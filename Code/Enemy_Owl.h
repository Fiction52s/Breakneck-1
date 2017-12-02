#ifndef __ENEMY_OWL_H__
#define __ENEMY_OWL_H__

#include "Enemy.h"

struct Owl : Enemy, LauncherEnemy
{
	enum Action
	{
		REST,
		GUARD,
		SPIN,
		FIRE
	};

	sf::CircleShape guardCircle;
	bool hasGuard;
	Owl(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos,
		int bulletSpeed,
		int framesBetween,
		bool facingRight);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ActionEnded();
	//void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	double ang;
	sf::Vector2<double> fireDir;


	Tileset *ts_bulletExplode;
	Action action;
	std::map<Action, int> actionLength;
	std::map<Action, int> animFactor;

	int bulletSpeed;
	int movementRadius;
	int retreatRadius;
	int shotRadius;
	int chaseRadius;
	int framesBetween;

	CubicBezier flyingBez;

	sf::Vector2i originalPos;

	sf::Vector2<double> velocity;
	double flySpeed;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	Tileset *ts_death;
	Tileset *ts_flap;
	Tileset *ts_spin;
	Tileset *ts_throw;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	Launcher *launcher;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;

	//Tileset *ts_testBlood;
	//Tileset *ts_blood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

#endif
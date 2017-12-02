#ifndef __ENEMY_GHOST_H__
#define __ENEMY_GHOST_H__

#include "Enemy.h"

struct Ghost : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		BITE,
		EXPLODE,
		Count
	};

	sf::Rect<double> detectionRect;
	Action action;
	std::map<Action, int> actionLength;
	std::map<Action, int> animFactor;

	double latchStartAngle;
	MovementSequence testSeq;
	Ghost(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos,
		float speed);

	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	//bool physicsOver;

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

	bool origFacingRight;
	int awakeFrames;
	int awakeCap;
	bool awake;

	sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	int frame;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;
	sf::Vector2<double> origOffset;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	CubicBezier approachAccelBez;


	sf::Vector2<double> offsetPlayer;
	sf::Vector2<double> origPosition;
	//double offsetRadius;
	bool latchedOn;

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
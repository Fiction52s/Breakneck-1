#ifndef __ENEMY_SHARK_H__
#define __ENEMY_SHARK_H__

#include "Enemy.h"

struct Shark : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		CIRCLE,
		FINALCIRCLE,
		RUSH,
	};
	Action action;

	int circleCounter;

	int wakeCounter;
	//int wakeCap;
	int wakeCap;

	int circleFrames;
	double attackAngle;
	//int attackCounter;

	std::map<Action, int> actionLength;
	std::map<Action, int> animFactor;

	double latchStartAngle;
	MovementSequence circleSeq;
	MovementSequence rushSeq;
	Shark(GameSession *owner, bool hasMonitor,
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

	sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	sf::Color testColor; //for temp anim

	int frame;

	double acceleration;
	double speed;

	//int approachFrames;
	//int totalFrame;
	sf::Vector2<double> origOffset;
	sf::Vector2<double> attackOffset;

	sf::Sprite sprite;
	Tileset *ts_circle;
	Tileset *ts_bite;
	Tileset *ts_death;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

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
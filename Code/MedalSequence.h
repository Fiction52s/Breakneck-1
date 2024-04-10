#ifndef __MEDAL_SEQUENCE_H__
#define __MEDAL_SEQUENCE_H__

#include "Sequence.h"

struct Medal;
struct ShardPopup;

struct MedalSequence : Sequence
{
	enum State
	{
		SHOW_MEDAL,
		AWARD_SHARD,
		END,
		Count
	};

	/*int enterTime;
	int exitTime;
	sf::Vector2<double> abovePlayer;
	sf::Vector2<double> origPlayer;
	sf::Vector2<double> attachPoint;
	MovementSequence shipMovement;
	MovementSequence center;
	Tileset *ts_ship;
	sf::Sprite shipSprite;*/

	sf::Vertex overlayRect[4];
	Medal *shownMedal;
	ShardPopup *shardPop;

	int shardWorld;
	int shardLocalIndex;

	MedalSequence();
	~MedalSequence();

	void StartGold(int world, int localIndex);
	void StartSilver();
	void StartBronze();
	void StartRunning();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddFlashes();
	void AddEnemies();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

#endif
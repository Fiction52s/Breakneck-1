#ifndef __RUSH_SCORE_DISPLAY_H__
#define __RUSH_SCORE_DISPLAY_H__

#include <SFML\Graphics.hpp>
#include "ScoreDisplay.h"
#include "Tileset.h"
#include <vector>

struct RushManager;
struct RushUpgradePopup;
struct Session;

struct RushScoreDisplay : ScoreDisplay
{
	enum Action
	{
		A_IDLE,
		A_SHOW,
		A_WAIT,
	};

	Tileset *ts_test;
	sf::Sprite testSpr;
	RushManager *rushManager;
	RushUpgradePopup *upgradePop;

	void CreateDescriptionTable();
	void SetTableEntry( int index, const std::string & s1, const std::string &s2);
	std::vector<std::pair<std::string,std::string>> upgradeDescriptionStringTable;

	RushScoreDisplay(RushManager *p_rushManager, sf::Font &f);
	~RushScoreDisplay();
	void Reset();
	void Activate();
	void Deactivate();
	void Confirm();
	void Update();
	bool IsActive();
	bool IsConfirmable();
	bool IsWaiting();
	bool IsIncludingExtraOptions();
	void SetSession(Session *sess);

	void Draw(sf::RenderTarget *target);

};

#endif
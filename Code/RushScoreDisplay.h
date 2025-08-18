#ifndef __RUSH_SCORE_DISPLAY_H__
#define __RUSH_SCORE_DISPLAY_H__

#include <SFML\Graphics.hpp>
#include "ScoreDisplay.h"
#include "Tileset.h"
#include <vector>
#include "Movement.h"
#include "AbsorbParticles.h"

struct RushManager;
struct RushUpgradePopup;
struct Session;
struct KinStore;
struct KinExperienceBar;
struct BackpackCounter;
struct GoalMedal;

struct RushScorePanel
{
	enum Action
	{
		A_HIDE,
		A_ENTER,
		A_SHOW,
		A_LEAVE,
		A_Count
	};

	int actionLength[A_Count];
	//int animFactor[A_Count];

	int action;
	int frame;

	sf::Vertex panelQuad[4];
	sf::Vector2f hidePos;
	sf::Vector2f showPos;
	CubicBezier showBez;
	CubicBezier hideBez;

	RushScorePanel(TilesetManager *tm);
	void Reset();
	void Update();
	void Enter();
	void Leave();
	bool IsHidden();
	bool IsShowing();
	void Draw(sf::RenderTarget *target);
};

struct RushScoreDisplay : ScoreDisplay, AbsorbReactor
{
	enum Action
	{
		A_IDLE,
		A_PANEL_ENTER,
		A_PANEL_SHOW,
		A_PANEL_HIDING,
		A_ENERGY_TRANSFER,
		A_WAIT_POST_ENERGY,
		A_WAIT,
	};
	GoalMedal *medal;

	Tileset *ts_test;
	sf::Sprite testSpr;
	RushManager *rushManager;
	RushUpgradePopup *upgradePop;
	KinStore *kinStore;
	RushScorePanel *scorePanel;
	BackpackCounter *backpackCounter;

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
	void ParticleDestroyed();
	void OpenStore();

	void Draw(sf::RenderTarget *target);

};

#endif
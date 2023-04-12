#ifndef __PRACTICEINVITEDISPLAY_H__
#define __PRACTICEINVITEDISPLAY_H__

#include "PlayerSkinShader.h"
#include <vector>
#include <SFML\Graphics.hpp>
#include "Input.h"

struct Tileset;
struct Session;
struct PracticePlayer;
struct PracticeInviteDisplay;

struct PracticePlayerDisp
{
	PlayerSkinShader playerSkinShader;
	sf::Sprite kinSprite;
	Tileset *ts_kin;
	sf::Vector2f topLeft;
	sf::Text nameText;
	PracticeInviteDisplay *disp;

	PracticePlayerDisp(PracticeInviteDisplay *p_disp );
	void Reset();
	void SetTopLeft(sf::Vector2f p_topLeft);
	void SetSkin(int index);
	void SetName(const std::string &name);
	void Draw(sf::RenderTarget *target);
	float GetKinBotPos();
};

struct PracticeUserBox
{
	enum Action
	{
		A_EMPTY,
		A_HAS_PLAYER,
		A_PLAYER_WANTS_TO_PLAY,
		//A_WAITING_FOR_CONFIRM,
		//A_RACE_CONFIRMED,
		A_RUNNING,
	};

	PracticePlayerDisp playerDisp;

	int action;

	sf::Vector2f topLeft;

	bool selected;

	int frame;

	Tileset *ts_raceButton;

	PracticePlayer *practicePlayer;
	PracticeInviteDisplay *disp;

	sf::Vertex raceButtonQuad[4];

	sf::Vertex bgQuad[4];

	PracticeUserBox(PracticeInviteDisplay *p_disp);
	bool IsEmpty();
	void UpdateBGQuad();
	void Reset();
	void SetTopLeft(sf::Vector2f & topLeft);
	void SetPlayer(PracticePlayer * pp);
	void Draw(sf::RenderTarget *target);
	void SetSelected(bool sel);
	void InvitePlayer();
	void RequestRace();
	void Update();
	void SetAction(int a);
};

struct PracticeRaceCountdownBox
{
	enum Action
	{
		A_ACTIVE,
		A_CANCELLED_BY_ME,
		A_CANCELLED_BY_OPPONENT,
		A_READY_TO_START,
	};

	int action;
	int totalFrames;
	int framesRemaining;
	sf::Vertex bgQuad[4];
	sf::Text timerText;
	PracticeInviteDisplay *disp;
	sf::Vector2f topLeft;
	sf::Vector2f size;
	sf::Vector2f defaultSize;
	sf::Text matchTitleText;

	sf::Text cancelText;

	sf::Vertex buttonQuads[4];

	PracticeRaceCountdownBox(PracticeInviteDisplay *p_disp);
	void Activate( int mode, const std::string &oppName );
	bool IsCancelledByMe();
	bool IsCancelledByOpponent();
	bool IsReadyToStart();
	void UpdateButtonIconsWhenControllerIsChanged();
	void SetTopLeft(sf::Vector2f p_topLeft);
	void SetCenter(sf::Vector2f p_center);
	void Update(const ControllerState & curr, const ControllerState &prev);
	void Draw(sf::RenderTarget *target);
};

struct PracticeRaceWaitingForConfirmBox
{
	PracticeInviteDisplay *disp;
	sf::Vertex bgQuad[4];
	sf::Text waitingText;
	sf::Vector2f size;

	PracticeRaceWaitingForConfirmBox(PracticeInviteDisplay *p_disp);
	void SetTopLeft(sf::Vector2f p_topLeft);
	void SetCenter(sf::Vector2f p_center);
	void Draw(sf::RenderTarget *target);
};

struct PracticeInviteDisplay
{
	enum Action
	{
		A_IDLE,
		A_SHOW_PLAYERS,
		A_WAITING_FOR_RACE_ACCEPT,
		//A_REQUESTING_TO_RACE,
		A_PREPARING_TO_LEAVE,
		A_RUN_GAME,
	};


	const static int NUM_BOXES = 3;

	PracticeRaceCountdownBox *countdownBox;
	PracticeRaceWaitingForConfirmBox *waitingMessage;

	int action;
	Session *sess;
	std::vector<PracticeUserBox*> userBoxVec;

	bool wantsToPlay;
	
	sf::Vector2f totalSize;
	sf::Vector2f boxSize;

	int selectedIndex;
	sf::Vertex selectedQuad[4];

	sf::Vertex startHostingQuad[4];	

	sf::Vertex buttonQuads[4 * 2];
	Tileset *ts_buttons;

	sf::Text availableText;
	sf::Text unavailableText;
	

	sf::Vector2f topLeft;

	bool isHosting;
	int hostMode;
	int hostNumMaxPlayers;
	int hostPowerMode;

	int frame;

	

	PracticeInviteDisplay();
	~PracticeInviteDisplay();
	void Reset();
	void SetAction(int a);
	bool Update(const ControllerState & curr, const ControllerState &prev);
	void PrepareToLeave();
	void UpdateButtonIconsWhenControllerIsChanged();

	void Populate();

	bool IsTryingToStartMatch();

	void SetTopLeft(sf::Vector2f & topLeft);
	void SetTopCenter(sf::Vector2f center);
	void SetCenter(sf::Vector2f center);
	void Draw(sf::RenderTarget *target);
};

#endif
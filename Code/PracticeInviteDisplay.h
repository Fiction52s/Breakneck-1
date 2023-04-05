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
	void Confirm();
	void Update();
	void SetAction(int a);
};

struct PracticeInviteDisplay
{
	enum Action
	{
		A_IDLE,
		A_SHOW_PLAYERS,
		A_HOST_SETUP,
		A_HOSTING,
	};


	const static int NUM_BOXES = 3;

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

	

	PracticeInviteDisplay();
	~PracticeInviteDisplay();
	void Reset();
	bool Update(const ControllerState & curr, const ControllerState &prev);
	void UpdateButtonIconsWhenControllerIsChanged();

	void Populate();

	bool IsTryingToStartMatch();

	void SetTopLeft(sf::Vector2f & topLeft);
	void SetTopCenter(sf::Vector2f center);
	void SetCenter(sf::Vector2f center);
	void Draw(sf::RenderTarget *target);
};

#endif
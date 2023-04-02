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

struct InvitePlayerBox
{
	enum Action
	{
		A_EMPTY,
		A_HAS_PLAYER,
		A_INVITED_PLAYER,
		A_PLAYER_HAS_INVITED_ME,
		A_READY_TO_RUN,
		A_RUNNING,
	};

	int action;

	PlayerSkinShader playerSkinShader;
	sf::Sprite kinSprite;

	Tileset *ts_kin;

	sf::Vector2f topLeft;

	sf::Text nameText;

	PracticePlayer *practicePlayer;
	PracticeInviteDisplay *disp;

	sf::Vertex buttonQuads[4 * 2];
	sf::Text inviteText;
	sf::Text acceptText;
	sf::Text runText;
	sf::Text waitingText;
	sf::Vertex modeQuad[4];

	Tileset *ts_buttons;
	//int boxIndex;

	InvitePlayerBox(PracticeInviteDisplay *p_disp);
	bool IsEmpty();
	void Reset();
	void SetTopLeft(sf::Vector2f & topLeft);
	void UpdateButtonIconsWhenControllerIsChanged();
	void UpdateButtonIconAndTextPositions();
	void SetPlayer(PracticePlayer * pp);
	void Draw(sf::RenderTarget *target);
	void InvitePlayer();
	void Confirm();
	void Update();
	void SetAction(int a);
private:
	void SetSkin(int index);
	void SetName(const std::string &name);
};

struct PracticeInviteDisplay
{
	enum Action
	{
		A_IDLE,
		A_SHOW_PLAYERS,
	};


	const static int NUM_BOXES = 3;

	int action;
	Session *sess;
	std::vector<InvitePlayerBox*> boxVec;
	sf::Vector2f boxSize;
	sf::Vector2f totalSize;
	sf::Vertex bgQuad[4];
	int selectedIndex;
	sf::Vertex selectedQuad[4];
	

	sf::Vector2f topLeft;

	

	PracticeInviteDisplay();
	~PracticeInviteDisplay();
	void Reset();
	bool Update(const ControllerState & curr, const ControllerState &prev);

	void Populate();

	bool IsTryingToStartMatch();

	void UpdateSelectedQuad();
	void SetTopLeft(sf::Vector2f & topLeft);
	void SetCenter(sf::Vector2f center);
	void Draw(sf::RenderTarget *target);
};

#endif
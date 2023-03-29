#ifndef __PRACTICEINVITEDISPLAY_H__
#define __PRACTICEINVITEDISPLAY_H__

#include "PlayerSkinShader.h"
#include <vector>
#include <SFML\Graphics.hpp>

struct Tileset;
struct Session;

struct InvitePlayerBox
{
	enum Action
	{
		A_EMPTY,
		A_HAS_PEER,
		A_INVITED_ME,
		A_HAS_ALREADY_BEEN_INVITED,
	};

	int action;

	PlayerSkinShader playerSkinShader;
	sf::Sprite kinSprite;

	Tileset *ts_kin;

	sf::Text nameText;
	

	

	InvitePlayerBox();
	void Reset();
	void SetTopLeft(sf::Vector2f & topLeft);
	void SetPlayer(const std::string &name, int skinIndex);
	void Draw(sf::RenderTarget *target);
private:
	void SetSkin(int index);
	void SetName(const std::string &name);
};

struct PracticeInviteDisplay
{
	const static int NUM_BOXES = 3;

	Session *sess;
	std::vector<InvitePlayerBox*> boxVec;
	sf::Vector2f totalSize;
	sf::Vertex bgQuad[4];

	PracticeInviteDisplay();
	~PracticeInviteDisplay();
	void Reset();

	void Populate();

	void SetTopLeft(sf::Vector2f & topLeft);
	void SetCenter(sf::Vector2f center);
	void Draw(sf::RenderTarget *target);
};

#endif
#include "PracticeInviteDisplay.h"
#include "Tileset.h"
#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "NetplayManager.h"

using namespace sf;
using namespace std;

InvitePlayerBox::InvitePlayerBox()
	:playerSkinShader( "player" )
{
	Session *sess = Session::GetSession();

	ts_kin = sess->GetPlayer(0)->STAND_GetTileset();

	kinSprite.setTexture(*ts_kin->texture);
	kinSprite.setTextureRect(ts_kin->GetSubRect(0));
	playerSkinShader.SetSubRect(ts_kin, ts_kin->GetSubRect(0));

	playerSkinShader.SetSkin(0);

	nameText.setFont(sess->mainMenu->arial);
	nameText.setFillColor(Color::White);
	nameText.setCharacterSize(20);
}

void InvitePlayerBox::Reset()
{
	action = A_EMPTY;
	nameText.setString("");
}

void InvitePlayerBox::SetPlayer(const std::string &name, int skinIndex)
{
	assert(action == A_EMPTY);

	action = A_HAS_PEER;
	SetName(name);
	SetSkin(skinIndex);
}

void InvitePlayerBox::SetSkin(int index)
{
	playerSkinShader.SetSkin(index);
}

void InvitePlayerBox::SetName(const std::string &name)
{
	nameText.setString(name);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);
}

void InvitePlayerBox::SetTopLeft(sf::Vector2f & topLeft)
{
	kinSprite.setPosition(topLeft);
	nameText.setPosition(topLeft);
}


void InvitePlayerBox::Draw(sf::RenderTarget *target)
{
	if (action != A_EMPTY)
	{
		target->draw(kinSprite, &playerSkinShader.pShader);
		target->draw(nameText);
	}
}

PracticeInviteDisplay::PracticeInviteDisplay()
{
	sess = Session::GetSession();

	boxVec.resize(NUM_BOXES);

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i] = new InvitePlayerBox;
	}

	totalSize.x = 500;
	totalSize.y = 200;

	SetCenter(Vector2f(960, 540 - 200));
}

PracticeInviteDisplay::~PracticeInviteDisplay()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		delete boxVec[i];
	}
}

void PracticeInviteDisplay::Reset()
{
	Populate();
}

void PracticeInviteDisplay::Populate()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->Reset();
	}

	int boxIndex = 0;
	assert(sess->netplayManager != NULL);
	for (int i = 0; i < NetplayManager::MAX_PRACTICE_PLAYERS; ++i)
	{
		if (sess->netplayManager->practicePlayers[i].isConnectedTo)
		{
			boxVec[boxIndex]->SetPlayer(sess->netplayManager->practicePlayers[i].name, sess->netplayManager->practicePlayers[i].skinIndex);
			++boxIndex;

			if (boxIndex == NUM_BOXES)
				break;
		}
	}
}

void PracticeInviteDisplay::SetTopLeft(sf::Vector2f & topLeft)
{
	Vector2f currTopLeft = topLeft;
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->SetTopLeft(currTopLeft);
		currTopLeft.x += 200;
	}

	SetRectTopLeft(bgQuad, totalSize.x, totalSize.y, topLeft);
	SetRectColor(bgQuad, Color(100, 100, 100, 100));
}

void PracticeInviteDisplay::SetCenter(sf::Vector2f center)
{
	SetTopLeft(Vector2f(center.x - totalSize.x / 2, center.y - totalSize.y / 2));
}

void PracticeInviteDisplay::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->Draw(target);
	}
}
#include "MainMenu.h"
#include "MenuPopup.h"

using namespace sf;
using namespace std;

MenuInfoPopup::MenuInfoPopup(MainMenu *mainMenu)
{
	size =Vector2f(500, 300);

	panel = new Panel("menuinfopopup", size.x, size.y, this, true);
	//SetRectColor(popupBGQuad, Color::Black);

	infoText = panel->AddLabel("infolabel", Vector2i(10, 10), 30, "");

	okButton = panel->AddButton("ok", Vector2i(10, 100), Vector2f(100, 30), "OK");
	panel->SetConfirmButton(okButton);
	panel->SetCancelButton(okButton);

	SetPos(Vector2f(960, 540));

	forcedStayOpenFrames = 0;
}

MenuInfoPopup::~MenuInfoPopup()
{
	delete panel;
}

void MenuInfoPopup::Pop(const std::string &str, int p_forcedStayOpenFrames )
{
	action = A_OPEN;
	forcedStayOpenFrames = p_forcedStayOpenFrames;
	SetText(str);
}

bool MenuInfoPopup::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	//cout << "starting mouse update\n";
	panel->MouseUpdate();

	//cout << "finished mouse update\n";

	if (forcedStayOpenFrames > 0)
	{
		--forcedStayOpenFrames;
		return false;
	}

	if ((currInput.A && !prevInput.A)
		|| (currInput.B && !prevInput.B)
		|| (currInput.Y && !prevInput.Y)
		|| (currInput.X && !prevInput.X)
		|| (currInput.rightShoulder && !prevInput.rightShoulder))
	{
		return true;
	}

	if (action == A_CLOSED)
	{
		return true;
	}

	return false;
}

void MenuInfoPopup::SetPos(sf::Vector2f &pos)
{
	position = pos;
	panel->SetCenterPos(Vector2i(pos));
	
	//SetRectCenter(popupBGQuad, size.x, size.y, pos);
	//text.setPosition(pos);
}

void MenuInfoPopup::SetText(const std::string &str)
{
	infoText->setString(str);
	//infoText->setOrigin(infoText->getLocalBounds().left + infoText->getLocalBounds().width / 2, 0);
		//infoText->getLocalBounds().top + infoText->getLocalBounds().height / 2);
	size.x = infoText->getGlobalBounds().width + 40;
	size.y = infoText->getCharacterSize() + infoText->getPosition().y + 20 + okButton->size.y + 10;//infoText->getGlobalBounds().height + 80;

	

	panel->SetSize(size);
	okButton->SetPos(Vector2i(size.x / 2 - okButton->size.x / 2, size.y - okButton->size.y - 10));
	//okButton->pos =

	SetPos(position);
}

void MenuInfoPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}

void MenuInfoPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MenuInfoPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		action = A_CLOSED;
	}
	
}
#include "QuickplaySearchScreen.h"
#include "MainMenu.h"
#include "LoadingBackpack.h"
#include "NetplayManager.h"

using namespace sf;
using namespace std;

QuickplaySearchScreen::QuickplaySearchScreen()
{
	//SetRectColor(bgQuad, Color::Red);
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	backpack = new LoadingBackpack(this);

	

	ts_bg = GetTileset( "Menu/Load/load_w2.png", 1920, 1080 );

	SetRectSubRect(bgQuad, ts_bg->GetSubRect(0));

	MainMenu *mm = MainMenu::GetInstance();

	searchingText.setFont(mm->arial);
	searchingText.setCharacterSize(100);
	searchingText.setFillColor(Color::White);
	searchingText.setOutlineColor(Color::Black);
	searchingText.setOutlineThickness(3);
	searchingText.setString("Searching...");
	auto lb = searchingText.getLocalBounds();
	searchingText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	cancelText.setFont(mm->arial);
	cancelText.setCharacterSize(40);
	cancelText.setString("Cancel:");
	

	sf::Vector2f textPos(960, 540 - 50);

	SetRectCenter(frameQuad, 550, 480, textPos + Vector2f( 0, 125 + 15 ));
	SetRectColor(frameQuad, Color(0, 0, 0, 150));

	cancelText.setPosition(textPos + Vector2f(-110, 305));

	searchingText.setPosition(textPos);

	backpack->SetPosition(textPos + Vector2f( 0, 200 ));

	UpdateButtonIconsWhenControllerIsChanged();

	SetRectCenter(buttonQuad, 64, 64, textPos + Vector2f(70, 335));

	Reset();
}

QuickplaySearchScreen::~QuickplaySearchScreen()
{
	delete backpack;
}

void QuickplaySearchScreen::Reset()
{
	action = A_SEARCHING;
	frame = 0;
}

bool QuickplaySearchScreen::IsCanceled()
{
	return action == A_CANCELED;
}

bool QuickplaySearchScreen::IsMatchFound()
{
	return action == A_FOUND;
}

void QuickplaySearchScreen::Update()
{
	backpack->Update();

	if (CONTROLLERS.ButtonPressed_B())
	{
		action = A_CANCELED;
		return;
	}
	//no update for now
}

void QuickplaySearchScreen::UpdateButtonIconsWhenControllerIsChanged()
{
	MainMenu *mm = MainMenu::GetInstance();

	int cType = mm->netplayManager->myControllerInput->GetControllerType();

	ts_buttons = mm->GetButtonIconTileset(cType);

	auto button = XBoxButton::XBOX_B;
	IntRect ir = mm->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(buttonQuad, ir);
}

void QuickplaySearchScreen::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads, ts_bg->texture);
	target->draw(frameQuad, 4, sf::Quads);
	backpack->Draw(target);
	target->draw(searchingText);
	target->draw(cancelText);

	target->draw(buttonQuad, 4, sf::Quads, ts_buttons->texture);
}
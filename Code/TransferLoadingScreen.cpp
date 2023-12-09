#include "TransferLoadingScreen.h"
#include "ShardAndLogDisplay.h"
#include "MainMenu.h"
#include "AdventureManager.h"

using namespace sf;

TransferLoadingScreen::TransferLoadingScreen()
{
	shardAndLogDisplay = new ShardAndLogDisplay(this);

	MainMenu *mm = MainMenu::GetInstance();

	shardAndLogDisplay = new ShardAndLogDisplay(this);
	shardAndLogDisplay->SetTopLeft(Vector2f(779, 902));

	levelNameText.setFont(mm->arial);
	levelNameText.setCharacterSize(60);
	levelNameText.setFillColor(Color::White);
}

TransferLoadingScreen::~TransferLoadingScreen()
{
	delete shardAndLogDisplay;
}

void TransferLoadingScreen::SetLevel(Level *lev)
{
	MainMenu *mm = MainMenu::GetInstance();
	level = lev;

	shardAndLogDisplay->SetLevel(level);

	levelNameText.setString(mm->adventureManager->adventureFile.GetMap(level->index).name);

	auto lb = levelNameText.getLocalBounds();

	levelNameText.setOrigin(lb.left + lb.width / 2, lb.height / 2);
	levelNameText.setPosition(960, 800);
}

void TransferLoadingScreen::DrawLevelInfo(sf::RenderTarget *target)
{
	target->draw(levelNameText);
	shardAndLogDisplay->Draw(target);
}
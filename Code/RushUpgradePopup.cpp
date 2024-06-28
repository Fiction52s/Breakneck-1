#include "RushUpgradePopup.h"
#include "MainMenu.h"
#include "TutorialBox.h"
#include "RushManager.h"
#include "RushScoreDisplay.h"

using namespace sf;
using namespace std;

RushUpgradePopup::RushUpgradePopup( RushManager *rm )
{
	upgradeIndex = -1;

	rushManager = rm;

	nameText.setCharacterSize(50);
	nameText.setFillColor(Color::Red);
	nameText.setFont(MainMenu::GetInstance()->arial);

	SetRectColor(bgQuad, Color(0, 0, 0, 200));

	Color borderColor = Color(100, 100, 100, 100);
	SetRectColor(topBorderQuad, borderColor);
	SetRectColor(powerBorderQuad, borderColor);

	ts_upgrade = rushManager->GetSizedTileset("Enemies/General/poweritem_128x128.png");
	upgradeSpr.setTexture(*ts_upgrade->texture);

	borderHeight = 2;

	width = 1400;

	powerBorder = 20;

	descBorder = Vector2f(10, 10);

	nameHeight = nameText.getFont()->getLineSpacing(nameText.getCharacterSize());

	tutBox = new TutorialBox(40, Vector2f(0, 0), Color::Transparent, Color::White, 0);

	float descLineHeight = tutBox->text.getFont()->getLineSpacing(tutBox->text.getCharacterSize());

	float extraHeight = 10;

	powerSize = 128;

	height = nameHeight + borderHeight + descLineHeight * 4 + descBorder.y * 2 + extraHeight;

	powerRel = Vector2f(powerBorder, nameHeight + borderHeight + powerBorder);
}

RushUpgradePopup::~RushUpgradePopup()
{
	delete tutBox;
}

void RushUpgradePopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void RushUpgradePopup::Update()
{
	tutBox->UpdateButtonIconsWhenControllerIsChanged();
	++frame;
}

void RushUpgradePopup::SetName(const std::string &name)
{
	nameText.setString(name);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);
}

void RushUpgradePopup::SetToMostRecentUpgrade()
{
	assert(rushManager->kinUpgradesInOrder.size() > 0);

	int finalUpgrade = rushManager->kinUpgradesInOrder.back();
	SetUpgrade(finalUpgrade);
}

void RushUpgradePopup::SetUpgrade(int index)
{
	upgradeIndex = index;

	upgradeSpr.setTextureRect(ts_upgrade->GetSubRect(0));

	auto &entry = rushManager->rushScoreDisplay->upgradeDescriptionStringTable[index];
	SetName(entry.first);
	tutBox->SetText(entry.second);
}

void RushUpgradePopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	SetRectTopLeft(bgQuad, width, height, topLeft);
	SetRectTopLeft(topBorderQuad, width, borderHeight, topLeft + Vector2f(0, nameHeight));

	upgradeSpr.setPosition(topLeft + powerRel);

	float remaining = height - nameHeight;

	float powerBorderLeft = powerBorder * 2 + powerSize;

	SetRectTopLeft(powerBorderQuad, borderHeight, remaining, topLeft + Vector2f(powerBorderLeft, nameHeight + borderHeight));

	Vector2f center = topLeft + Vector2f(width / 2, height / 2);

	nameText.setPosition(center.x, topLeft.y);

	tutBox->SetTopLeft(topLeft + Vector2f(powerBorderLeft + borderHeight + descBorder.x, nameHeight + borderHeight + descBorder.y));
}

void RushUpgradePopup::SetCenter(sf::Vector2f &pos)
{
	SetTopLeft(Vector2f(pos.x - width / 2, pos.y - height / 2));
}

void RushUpgradePopup::Draw(RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(topBorderQuad, 4, sf::Quads);
	target->draw(powerBorderQuad, 4, sf::Quads);

	target->draw(upgradeSpr);
	target->draw(nameText);

	tutBox->Draw(target);
}
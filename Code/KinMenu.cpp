#include <sstream>
#include "GameSession.h"
#include "MainMenu.h"
#include <iostream>
#include "SaveFile.h"
#include <fstream>
#include "ShardMenu.h"
#include "MusicSelector.h"
#include "ControlProfile.h"
#include "ColorShifter.h"
#include "MapHeader.h"
#include "Config.h"
#include "KinMenu.h"
#include "Actor.h"
#include "PauseMenu.h"
#include "TutorialBox.h"


using namespace sf;
using namespace std;

KinMenu::KinMenu( TilesetManager *p_tm )
	:playerSkinShader( "player")
{
	tm = p_tm;
	game = NULL;
	
	MainMenu *mm = MainMenu::GetInstance();

	ts_powers = tm->GetSizedTileset("Menu/power_icon_128x128.png");

	description.setFont(mm->arial);
	description.setCharacterSize(24);
	description.setFillColor(Color::White);

	int tutWidth = 1220;
	int tutHeight = 320;
	ts_tutorial[0] = tm->GetSizedTileset("Menu/tut_jump_1220x320.png");
	ts_tutorial[1] = tm->GetSizedTileset("Menu/tut_attack_1220x320.png");
	ts_tutorial[2] = tm->GetSizedTileset("Menu/tut_sprint_1220x320.png");
	ts_tutorial[3] = tm->GetSizedTileset("Menu/tut_dash_1220x320.png");
	ts_tutorial[4] = tm->GetSizedTileset("Menu/tut_walljump_1220x320.png");
	ts_tutorial[5] = tm->GetSizedTileset("Menu/tut_speed_1220x320.png");
	ts_tutorial[6] = tm->GetSizedTileset("Menu/tut_health_1220x320.png");
	ts_tutorial[7] = tm->GetSizedTileset("Menu/tut_survival_1220x320.png");
	ts_tutorial[8] = tm->GetSizedTileset("Menu/tut_key_1220x320.png");
	ts_tutorial[9] = tm->GetSizedTileset("Menu/tut_airdash_1220x320.png");
	

	SetRectColor(descriptionBox, Color(0, 0, 0, 255));
	SetRectCenter(descriptionBox, tutWidth, 90, Vector2f(1122, 439));//topleft is 512,394

	description.setPosition(512 + 10, 394 + 10);

	//90
	Vector2f tutBoxSize(1220, 160);
	tutBox = new TutorialBox(40, tutBoxSize, Color::Black, Color::White, 30);

	for (int i = 0; i < 10; ++i)
	{
		SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i));
		SetRectSubRect(powerQuadsBG + i * 4, ts_powers->GetSubRect(i));
	}

	int waitFrames[3] = { 60, 30, 20 };
	int waitModeThresh[2] = { 2, 2 };
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 2, 0);

	Tileset *ts_selector = tm->GetSizedTileset("Menu/power_icon_border_160x160.png");
	selectorSpr.setTexture(*ts_selector->texture);
	selectorSpr.setOrigin(selectorSpr.getLocalBounds().width / 2, selectorSpr.getLocalBounds().height / 2);

	ts_kin = tm->GetSizedTileset("Menu/pause_kin_400x836.png");
	ts_aura1A = tm->GetSizedTileset("Menu/pause_kin_aura_1a_400x836.png");
	ts_aura1B = tm->GetSizedTileset("Menu/pause_kin_aura_1b_400x836.png");
	ts_aura2A = tm->GetSizedTileset("Menu/pause_kin_aura_2a_400x836.png");
	ts_aura2B = tm->GetSizedTileset("Menu/pause_kin_aura_2b_400x836.png");
	ts_veins = tm->GetSizedTileset("Menu/pause_kin_veins_400x836.png");

	if (!scrollShader1.loadFromFile("Resources/Shader/menuauraslide.frag", sf::Shader::Fragment))
	{
		cout << "menu aura sliding SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	scrollShader1.setUniform("u_texture", sf::Shader::CurrentTexture);
	scrollShader1.setUniform("blendColor", ColorGL(Color::White));

	if (!scrollShader2.loadFromFile("Resources/Shader/menuauraslide.frag", sf::Shader::Fragment))
	{
		cout << "menu aura sliding SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	scrollShader2.setUniform("u_texture", sf::Shader::CurrentTexture);
	scrollShader2.setUniform("blendColor", ColorGL(Color::White));
	

	kinSpr.setTexture(*ts_kin->texture);
	aura1ASpr.setTexture(*ts_aura1A->texture);
	aura1BSpr.setTexture(*ts_aura1B->texture);
	aura2ASpr.setTexture(*ts_aura2A->texture);
	aura2BSpr.setTexture(*ts_aura2B->texture);
	veinSpr.setTexture(*ts_veins->texture);


	playerSkinShader.SetSubRect(ts_kin, ts_kin->GetSubRect(0));

	/*aura1ASpr.setColor(Color::Red);
	aura1BSpr.setColor(Color::Green);
	aura2ASpr.setColor(Color::Yellow);
	aura2BSpr.setColor(Color::Magenta);*/
	Image palette;
	bool loadPalette = palette.loadFromFile("Resources/Menu/pause_kin_aura_color.png");
	assert(loadPalette);

	Image powerPalette;
	bool loadPowerPalette = powerPalette.loadFromFile("Resources/Menu/power_icons_palette.png");

	aura1AShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura1BShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura2AShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura2BShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	bgShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	selectedShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 3);
	//selectedShifter->SetColors(palette, 0);
	//selectedShifter->SetColorIndex(0, Color::Cyan);
	//selectedShifter->SetColorIndex(1, Color::Cyan);
	//selectedShifter->SetColorIndex(0, Color::Black);

	selectedShifter->SetColors(powerPalette, 0);

	bgShifter->SetColors(palette, 0);
	aura1AShifter->SetColors(palette, 1);
	aura1BShifter->SetColors(palette, 2);
	aura2AShifter->SetColors(palette, 3);
	aura2BShifter->SetColors(palette, 4);
	/*aura1AShifter->SetColorIndex(0, Color::Red);
	aura1AShifter->SetColorIndex(1, Color::Cyan);
	aura1BShifter->SetColorIndex(0, Color::Yellow);
	aura1BShifter->SetColorIndex(1, Color::White);
	aura2AShifter->SetColorIndex(0, Color::Blue);
	aura2AShifter->SetColorIndex(1, Color::Black );
	aura2BShifter->SetColorIndex(0, Color::Green);
	aura2BShifter->SetColorIndex(1, Color::Red);
	bgShifter->SetColorIndex(0, Color::Magenta);
	bgShifter->SetColorIndex(1, Color::Black);*/

	aura1AShifter->Reset();
	aura1BShifter->Reset();
	aura2AShifter->Reset();
	aura2BShifter->Reset();
	bgShifter->Reset();
	selectedShifter->Reset();

	//SetRectColor(kinBG, Color(Color::Cyan));

	SetTopLeft(Vector2f(50, 50));

	frame = 0;

	powerDescriptions[0] = "Press JUMP to launch yourself into the air,or press JUMP while aerial to\n"
							"double jump. Hold the JUMP button down longer for extra height!";
	powerDescriptions[1] = "Press ATTACK to release an energy burst in front of you capable of\n"
							"destroying enemies. Hold up or down while in the air to do a\n"
							"directional attack.";
	powerDescriptions[2] = "When moving on a slope, hold diagonally Up/Down and foward to\n"
							"accelerate. Hold Up or down to slide with low friction. On steep\n"
							"slopes hold down to slide down even faster.";
	powerDescriptions[3] = "Press DASH to quickly start moving in the direction you are facing\n"
							"while grounded. Tap DASH quickly while ascending a steep slope\n"
							"to climb your way up.";
	powerDescriptions[4] = "Hold towards a wall to wall cling and descend slowly. Tap away\n"
							"from a wall to wall jump. Hold down and away from the wall to drift\n"
							"away from the wall without wall jumping";
	powerDescriptions[5] = "Move fast and kill enemies to build up your speed meter. There are\n"
							"3 different meter levels. With each speed level, your attacks get\n"
							"bigger and your dash is more powerful.";
	powerDescriptions[6] = "The TRANSCEND absorbs all energy within its territory, even yours!\n"
							"Kill enemies and steal their energy to bolster your health while\n"
							"you search for the NODE.";
	powerDescriptions[7] = "When Kin runs out of energy, the core of the BREAKNECK suit becomes\n"
							"unstable and enters SURVIVAL MODE. You have 5 seconds to kill an enemy\n"
							"or destroy the NODE before the BREAKNECK self destructs.";
	powerDescriptions[8] = "Certain enemies have a special ABSORPTION HEART which supports\n"
							"the NODE and VEINS. When you clear enough of them from an area,\n"
							"the nearby VEINS will weaken, allowing you to break through them.";
	powerDescriptions[9] = "Press DASH while in the air to HOVER for a short period. Hold a direction\n"
							"to AIRDASH in 1 of 8 directions. You can change your AIRDASH\n"
							"direction at any time. Kin gets 1 AIRDASH per air time.";
	//UpdateDescription();
	//UpdateSelector();
	//UpdatePowerSprite();
	//UpdateTutorial();

}

KinMenu::~KinMenu()
{
	delete tutBox;

	delete xSelector;
	delete ySelector;

	delete aura1AShifter;
	delete aura1BShifter;
	delete aura2AShifter;
	delete aura2BShifter;

	delete bgShifter;

	delete selectedShifter;
}

void KinMenu::SetGame(GameSession *p_game)
{
	game = p_game;
	tutBox->sess = p_game;
	UpdateSelector();
	UpdatePowerSprite();
	UpdateTutorial();
	UpdateDescription();
}

void KinMenu::SetTopLeft(sf::Vector2f &pos)
{

	Vector2f powerPos(0, 0);
	Vector2f powerSpacing(30, 20);

	Vector2f tutBoxTopLeft(512, 394);
	tutBoxTopLeft += pos;

	tutBox->SetTopLeft(tutBoxTopLeft);

	Vector2f tutBoxSize(1220, 160);

	Vector2f bottomLeftTutBox = tutBoxTopLeft + tutBoxSize;

	Vector2f powersOffset(512, bottomLeftTutBox.y + 20);//495 + 45);
	powersOffset += pos;

	for (int i = 0; i < 10; ++i)
	{
		SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i));
		SetRectSubRect(powerQuadsBG + i * 4, ts_powers->GetSubRect(i));
		//powerPos.x = powersOffset.x + (128 + powerSpacing.x) * (i%9) + 64;
		powerPos.x = powersOffset.x + (128 + powerSpacing.x) * (i % 8) + 64;
		powerPos.y = powersOffset.y + (128 + powerSpacing.y) * (i / 8) + 64;
		SetRectCenter(powerQuads + i * 4, 128, 128, powerPos);
		SetRectCenter(powerQuadsBG + i * 4, 128, 128, powerPos);
	}

	Vector2f tutPos(512, 74);
	tutorialSpr.setPosition(tutPos + pos);


	Vector2f offset(72, 74);

	offset += pos;

	kinSpr.setPosition(offset);
	aura1ASpr.setPosition(offset);
	aura1BSpr.setPosition(offset);
	aura2ASpr.setPosition(offset);
	aura2BSpr.setPosition(offset);
	veinSpr.setPosition(offset);
	veinSpr.setColor(Color::Transparent);

	SetRectCenter(kinBG, aura1ASpr.getGlobalBounds().width, aura1ASpr.getGlobalBounds().height,
		Vector2f(offset.x + 200, offset.y + 418));
}

void KinMenu::UpdateSelector()
{
	selectorSpr.setPosition(powerQuads[GetCurrIndex() * 4].position + Vector2f(64, 64));
	//SetRectCenter(powerQuads + i * 4, 128, 128, powerPos);
}

void KinMenu::UpdateTutorial()
{
	tutorialSpr.setTexture(*ts_tutorial[GetCurrIndex()]->texture);
}

void KinMenu::UpdatePowers(Actor *player)
{
	if (player->HasUpgrade( Actor::UPGRADE_POWER_AIRDASH ) )
	{
		secondRowMax = 1;
	}
	else
	{
		secondRowMax = 0;
	}
}

void KinMenu::UpdateCommandButton()
{
	//SetRectSubRect(commandRect, game->GetButtonIconTile( 0, ControllerSettings::BUTTONTYPE_JUMP));
	

	//GameController *con = game->GetController(0);
	//ts_currentButtons = NULL;
	//ControllerType cType = con->GetCType();
	//switch (cType)
	//{
	//case CTYPE_XBOX:
	//	ts_currentButtons = ts_xboxButtons;
	//	break;
	//case CTYPE_GAMECUBE:
	//	ts_currentButtons = ts_xboxButtons;
	//	break;
	//case CTYPE_PS4:
	//	ts_currentButtons = ts_xboxButtons;
	//	break;
	//case CTYPE_NONE:
	//	ts_currentButtons = ts_xboxButtons;
	//	//assert(0);
	//	break;
	//default:
	//	//assert(0);
	//	ts_currentButtons = ts_xboxButtons;
	//}

	//int index = GetCurrIndex();
	//IntRect sub;
	//if (index == 0)
	//	sub = ts_currentButtons->GetSubRect(csm->GetFilteredButton(cType, ControllerSettings::BUTTONTYPE_JUMP) - 1);
	//else if (index == 1)
	//	sub = ts_currentButtons->GetSubRect(csm->GetFilteredButton(cType, ControllerSettings::BUTTONTYPE_ATTACK) - 1);
	//else if (index == 3 || index == 9)
	//{
	//	sub = ts_currentButtons->GetSubRect(csm->GetFilteredButton(cType, ControllerSettings::BUTTONTYPE_DASH) - 1);
	//}

	//commandSpr.setTexture(*ts_currentButtons->texture);
	//commandSpr.setTextureRect(sub);
}

void KinMenu::Update(ControllerState &curr, ControllerState &prev)
{
	int xchanged;
	int ychanged;


	xchanged = xSelector->UpdateIndex(curr.LLeft() || curr.PLeft(), curr.LRight() || curr.PRight());
	ychanged = ySelector->UpdateIndex(curr.LUp() || curr.PUp(), curr.LDown() || curr.PDown());

	assert(game != NULL);

	if (xchanged != 0 || ychanged != 0)
	{
		game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("pause_change"));

		if (ySelector->currIndex == 1 && xSelector->currIndex > secondRowMax)
		{
			if (ychanged != 0)
			{
				ySelector->currIndex = 0;
			}
			else if (xchanged != 0)
			{
				xSelector->currIndex = 0;
			}

		}

		UpdateDescription();
		UpdateSelector();

		UpdateTutorial();
		UpdateCommandButton();
	}

	UpdatePowerSprite();


	int scrollFrames1 = 120;
	int scrollFrames2 = 240;
	float portion1 = ((float)(frame % scrollFrames1)) / scrollFrames1;
	float portion2 = ((float)(frame % scrollFrames2)) / scrollFrames2;
	scrollShader1.setUniform("quant", portion1);
	scrollShader2.setUniform("quant", portion2);

	int breatheFrames = 180;
	int breatheWaitFrames = 120;
	int bTotal = breatheFrames + breatheWaitFrames;
	float halfBreathe = breatheFrames / 2;
	int f = frame % (bTotal);
	float alpha;
	if (f <= halfBreathe)
	{
		alpha = f / halfBreathe;
	}
	else if (f <= breatheFrames)
	{
		f -= halfBreathe;
		alpha = 1.f - f / halfBreathe;
	}
	else
	{
		alpha = 0;
	}
	//cout << "alpha: " << alpha << endl;
	veinSpr.setColor(Color(255, 255, 255, alpha * 100));

	aura1AShifter->Update();
	aura1BShifter->Update();
	aura2AShifter->Update();
	aura2BShifter->Update();
	bgShifter->Update();
	selectedShifter->Update();
	//Color c = aura1AShifter->GetCurrColor();
	//cout << "c: " << c.a << ", " << c.g << ", " << c.b << endl;


	++frame;
}

int KinMenu::GetCurrIndex()
{
	return ySelector->currIndex * 8 + xSelector->currIndex % 8;
}

void KinMenu::UpdateDescription()
{
	description.setString(powerDescriptions[GetCurrIndex()]);
	tutBox->SetText(powerDescriptions[GetCurrIndex()]);

	/*sf::FloatRect textRect = description.getLocalBounds();
	description.setOrigin(textRect.left + textRect.width / 2.0f,
	textRect.top + textRect.height / 2.0f);
	description.setPosition(Vector2f(1122, 439));*/
	//text.setPosition(sf::Vector2f(SCRWIDTH / 2.0f, SCRHEIGHT / 2.0f));
}

void KinMenu::UpdatePowerSprite()
{
	int maxShowIndex = 8 + secondRowMax;
	int currIndex = GetCurrIndex();
	for (int i = 0; i < 10; ++i)
	{
		if (i > maxShowIndex)
		{
			SetRectSubRect(powerQuads + i * 4, FloatRect(0, 0, 0, 0));
			SetRectColor(powerQuadsBG + i * 4, Color(Color::Transparent));
		}
		else
		{
			SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 0));
			if (currIndex == i)
			{
				//SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 16));
				SetRectColor(powerQuadsBG + i * 4, selectedShifter->GetCurrColor());//Color(0, 0, 0, 255));
			}
			else
			{
				//SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 0));
				SetRectColor(powerQuadsBG + i * 4, Color(0, 0, 0, 255));
			}
		}
	}
}

void KinMenu::Draw(sf::RenderTarget *target)
{
	SetRectColor(kinBG, bgShifter->GetCurrColor());
	target->draw(kinBG, 4, sf::Quads);
	scrollShader1.setUniform("blendColor", ColorGL(aura1AShifter->GetCurrColor()));
	target->draw(aura1ASpr, &scrollShader1);
	scrollShader1.setUniform("blendColor", ColorGL(aura1BShifter->GetCurrColor()));
	target->draw(aura1BSpr, &scrollShader1);
	scrollShader2.setUniform("blendColor", ColorGL(aura2AShifter->GetCurrColor()));
	target->draw(aura2ASpr, &scrollShader2);
	scrollShader2.setUniform("blendColor", ColorGL(aura2BShifter->GetCurrColor()));
	target->draw(aura2BSpr, &scrollShader2);
	target->draw(kinSpr, &playerSkinShader.pShader);
	target->draw(veinSpr);

	target->draw(powerQuadsBG, 10 * 4, sf::Quads);
	target->draw(powerQuads, 10 * 4, sf::Quads, ts_powers->texture);


	target->draw(tutorialSpr);
	//target->draw(descriptionBox, 4, sf::Quads);

	int index = GetCurrIndex();
	if (index == 0 || index == 1 || index == 3 || index == 9)
	{
		//Tileset *ts_buttons = game->GetButtonIconTileset( 0 );
		//target->draw(commandRect, 4, sf::Quads, *ts_buttons);
	}

	//target->draw(description);

	tutBox->Draw(target);

	target->draw(selectorSpr);
}
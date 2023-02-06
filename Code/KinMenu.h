#ifndef __KINMENU_H__
#define __KINMENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"
#include "PlayerSkinShader.h"

struct Actor;
struct GameSession;
struct ShardMenu;
struct ColorShifter;
struct SingleAxisSelector;
struct TutorialBox;

struct KinMenu
{
	TutorialBox *tutBox;
	PlayerSkinShader playerSkinShader;
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	int GetCurrIndex();
	~KinMenu();
	GameSession *game;
	sf::Sprite selectorSpr;
	void UpdatePowers(Actor *player);
	int secondRowMax;
	KinMenu( GameSession *p_game);
	sf::Vertex powerQuads[10 * 4];
	sf::Vertex powerQuadsBG[10 * 4];
	ColorShifter *selectedShifter;
	ColorShifter *aura1AShifter;
	ColorShifter *aura1BShifter;
	ColorShifter *aura2AShifter;
	ColorShifter *aura2BShifter;
	ColorShifter *bgShifter;
	Tileset *ts_powers;
	Tileset *ts_kin;
	Tileset *ts_aura1A;
	Tileset *ts_aura1B;
	Tileset *ts_aura2A;
	Tileset *ts_aura2B;
	Tileset *ts_veins;
	int frame;
	sf::Shader scrollShader1;
	sf::Shader scrollShader2;
	//sf::Shader scrollShader1;
	sf::Color Get1AColor();
	sf::Color Get1BColor();
	sf::Color Get2AColor();
	sf::Color Get2BColor();
	sf::Sprite tutorialSpr;
	sf::Vertex descriptionBox[4];
	void UpdateCommandButton();
	Tileset *ts_tutorial[10];
	sf::Sprite kinSpr;
	sf::Sprite veinSpr;
	sf::Sprite aura1ASpr;
	sf::Sprite aura1BSpr;
	sf::Sprite aura2ASpr;
	sf::Sprite aura2BSpr;
	sf::Vertex kinBG[4];
	void Update(ControllerState &curr,
		ControllerState &prev);
	void UpdateDescription();
	void UpdateSelector();
	void UpdateTutorial();
	void UpdatePowerSprite();
	std::string powerDescriptions[10];
	void Draw(sf::RenderTarget *target);
	sf::Text description;
};

#endif
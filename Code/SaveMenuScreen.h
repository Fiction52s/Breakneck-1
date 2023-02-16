#ifndef __SAVEMENUSCREEN_H__
#define __SAVEMENUSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "PlayerSkinShader.h"
#include "Input.h"

struct MainMenu;
struct SaveFile;
struct WorldMap;
struct SkinMenu;

struct MessagePopup;
struct ConfirmPopup;
struct AdventurePlanet;
struct GamePopup;

struct SaveFileDisplay
{
	SaveFileDisplay(sf::Font &f);
	void SetPosition(sf::Vector2f &pos);
	void InitText(sf::Text &text);
	bool Contains(sf::Vector2f &pos);
	sf::Text completedWorldsText;
	sf::Text completeLevelsText;
	sf::Text capturedShardsText;
	sf::Text totalTime;

	sf::Vector2f pos;
	
	sf::Text blankText;
	
	bool blankMode;
	sf::Color fillColor;
	sf::Color lineColor;
	sf::Font &font;
	void SetValues(SaveFile *sf, AdventurePlanet *adventurePlanet );
	void Draw(sf::RenderTarget *target);
};


struct SaveMenuScreen : TilesetManager
{
	enum Action
	{
		WAIT,
		SELECT,
		TRANSITION,
		TRANSITIONMOVIE,
		TRANSITIONTUTORIAL,
		FADEIN,
		SKINMENU,
		CONFIRMDELETE,
		CONFIRMDELETE2,
		CONFIRMCOPY,
		INFOPOP,
		COPY,
		ASKTUTORIAL,
		Count
	};

	int copiedIndex;
	GamePopup *decisionPopup;
	//MessagePopup *messagePopup;
	//ConfirmPopup *confirmPopup;

	bool startWithTutorial;
	int currSkin;
	bool defaultFiles[6];
	Action action;
	int actionLength[Count];
	int frame;
	SkinMenu * skinMenu;
	SaveMenuScreen();
	~SaveMenuScreen();
	Tileset *ts_background;//ts_saveMenuBG;
	Tileset *ts_selectSlot;//ts_saveMenuSelect;
	Tileset *ts_kinFace;//ts_saveMenuKinFace;
	Tileset *ts_skinButton;
	sf::Vector2f GetTopLeftSaveSlot(int index);
	void SaveSelectedFile();
	void SetSkin(int index);
	void SaveCurrSkin();
	bool Update();
	void Draw(sf::RenderTarget *target);
	void Reset();
	void SelectedIndexChanged();
	void UnlockSkin(int skinIndex);
	bool IsSkinUnlocked(int skinIndex);
	void ChangeIndex(bool down, bool up, bool left, bool right);
	void UpdateSelectedIndex();
	bool HandleEvent(sf::Event ev);
	void SetSelectedIndex(int index);


	MainMenu *mainMenu;
	sf::Vector2f menuOffset;
	sf::Sprite background;
	sf::Sprite selectSlot;
	sf::Sprite kinFace;
	PlayerSkinShader playerSkinShader;
	PlayerSkinShader maskPlayerSkinShader;
	float transparency;
	float fadeOut;
	sf::Vertex blackQuad[4];
	sf::Vertex actionButtonIcons[4 * 4];

	Tileset *ts_buttons;

	Tileset *ts_kinJump[6];
	Tileset *ts_kinClouds;
	Tileset *ts_kinWindow;
	Tileset *ts_kinSky;

	sf::Sprite kinClouds;
	sf::Sprite kinJump;
	sf::Sprite kinWindow;
	sf::Sprite kinSky;
	sf::Sprite skinButtonSpr;
	sf::Sprite skinButtonIconSpr;

	Tileset *ts_starBackground;
	sf::Sprite starBackground;

	Tileset *ts_world;
	sf::Sprite world;
	int asteroidScrollFrames0;
	int asteroidScrollFrames1;
	int asteroidScrollFrames2;
	int asteroidFrameBack;
	int asteroidFrameFront;

	sf::Sprite asteroid0;
	sf::Sprite asteroid1;
	sf::Sprite asteroid2;
	sf::Vector2f a0start;
	sf::Vector2f a0end;
	sf::Vector2f a1start;
	sf::Vector2f a1end;
	sf::Vector2f a2start;
	sf::Vector2f a2end;
	void UpdateClouds();
	int cloudFrame;
	int cloudLoopLength;
	int cloudLoopFactor;

	int kinFaceTurnLength;

	int saveJumpFactor;
	int saveJumpLength;

	SaveFileDisplay *fileDisplay[6];
};

#endif
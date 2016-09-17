#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"

struct GameSession;

struct OptionSelector
{
	static sf::Font *font;	
	OptionSelector( sf::Vector2f &pos,
		int optionCount,
		std::string *options ); 

	void Right();
	void Left();
	void Stop();
	const std::string & GetString();
	void Update();
	void Draw( sf::RenderTarget *target );

	std::string *options;
	int optionCount;
	int currentIndex;
	sf::Text currentText;
	sf::Vector2f pos;

	int maxWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int maxMomentum;
	int currWaitFrames;
	CubicBezier accelBez;
	int momentum; //left is -1
	//right is 1, none is 0
};

struct PauseMenu
{
	enum Tab
	{
		MAP,
		KIN,
		SHARDS,
		OPTIONS,
		PAUSE,
		Count
	};

	PauseMenu( GameSession *owner );
	~PauseMenu();
	void Draw( sf::RenderTarget *target );
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();

	enum UpdateResponse
	{
		R_NONE,
		R_P_RESUME,
		R_P_RESPAWN,
		R_P_EXITLEVEL,
		R_P_EXITTITLE,
		R_P_EXITGAME
	};

	UpdateResponse Update( ControllerState &currInput,
		ControllerState &prevInput );
	Tileset *ts_background[Count];
	Tileset *ts_select;
	sf::Sprite bgSprite;
	sf::Sprite selectSprite;
	GameSession *owner;
	Tab currentTab;

	//bool show;
	//if you tab away it resets

	OptionSelector **videoSelectors;
	int numVideoOptions;
	//map tab
	sf::Vector2f mapCenter;
	float mapZoomFactor;

	int pauseSelectIndex;

	int optionSelectorIndex;

	OptionSelector **currentSelectors;
	int numCurrentSelectors;
};

#endif
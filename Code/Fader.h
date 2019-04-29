#ifndef __FADE_H__
#define __FADE_H__

#include <SFML/Graphics.hpp>

struct Fader
{
	Fader();
	sf::RectangleShape fadeRect;
	int fadeFrame;
	int fadeLength;
	bool fadingIn;
	bool fadingOut;
	int fadeAlpha;
	bool fadeSkipKin;
	int crossFadeFrame;
	int crossPauseLength;
	int crossFadeInLength;
	int crossFadeOutLength;
	bool crossFading;
	bool crossSkipKin;
	sf::Color crossColor;
	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, int fadeInFrames,
		sf::Color c, bool skipKin = false);
	bool IsFullyFadedOut();
	bool IsFading();
	void Update();
	void Clear();
	void Reset();
	void Draw(sf::RenderTarget *target);
};

struct MainMenu;
struct Tileset;

struct Swiper
{
	enum SwipeType
	{
		W1
	};

	struct SwipeTypeInfo
	{
		SwipeTypeInfo();
		int numImages;
		int numTiles;
		int name;
		bool loaded;
		int fullWipeIndex;
		void Load( MainMenu *mm);
		std::string baseName;
		Tileset **tilesets;
		sf::IntRect GetSubRect(int index);
		bool IsPostWipe(int index);
		Tileset *GetTileset(int index);
	};

	bool IsPostWipe();
	SwipeTypeInfo *currInfo;
	static void LoadSwipeType( MainMenu *mm, SwipeType st);
	static std::map<SwipeType, SwipeTypeInfo> swipeTypeMap;

	int numCoverTiles;
	Tileset *ts_swipe;

	SwipeType swipeType;
	bool skipKin;
	Swiper();
	int swipeFrame;
	int swipeLength;
	int animFactor;
	bool swiping;
	bool IsSwiping();
	void Swipe( SwipeType t, int animFactor,
		bool ignorePlayer = false);
	void Update();
	void Reset();
	sf::Sprite swipeSpr;
	void Draw(sf::RenderTarget *target);
};

#endif
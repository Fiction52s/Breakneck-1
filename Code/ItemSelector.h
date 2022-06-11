#ifndef __ITEM_SELECTOR_H__
#define __ITEM_SELECTOR_H__
#include <SFML\Graphics.hpp>
#include "Input.h"

struct FrameWaiter
{
	FrameWaiter(int p_numWaitFramesLevels,
		int *p_waitFrames,
		int p_numWaitModeThreshLevels,
		int *p_waitModeThresh);
	~FrameWaiter();
	void Reset();
	int *waitFrames;// [3];
	int *waitModeThresh;//[2];
	int numWaitModeThreshLevels;
	int numWaitFramesLevels;
	int framesWaiting;
	int currWaitLevel;
	int currWaitThreshLevel;
	int currWaitThreshLevelCounter;
	bool Hold();
};

struct SingleAxisSelector
{
	SingleAxisSelector(int numWaitFramesLevels,
		int*p_waitFrames,
		int numWaitModeThreshLevels,
		int *p_waitModeThresh,
		int totalItems,
		int startIndex,
		bool loop = true);
	~SingleAxisSelector();
	void ResetCounters();
	void Reset();
	void SetIndex(int ind);
	int *waitFrames;// [3];
	int *waitModeThresh;//[2];
	int framesWaiting;

	int numWaitFramesLevels;
	int numWaitModeThreshLevels;

	//0 for none, 1 for inc, -1 for dec
	int UpdateIndex(bool dec, bool inc);
	int currWaitLevel;
	int flipCounterUp;
	int flipCounterDown;
	bool loop;
	int currIndex;
	int totalItems;
	int oldCurrIndex;
	int startIndex;
	void SetTotalSize( int total );
};

//struct DoubleAxisSelector
//{
//	DoubleAxisSelector(int numWaitFramesLevels,
//		int*p_waitFrames,
//		int numWaitModeThreshLevels,
//		int *p_waitModeThresh,
//		int totalItemsX, int totalItemsY,
//		int startIndex,
//		bool loop = true);
//	~DoubleAxisSelector();
//	void ResetCounters();
//	void Reset();
//	int *waitFrames;// [3];
//	int *waitModeThresh;//[2];
//	int framesWaiting;
//	//0 for none, 1 for inc, -1 for dec
//	int UpdateIndex(bool left, bool right, bool dec, bool inc);
//	int currWaitLevel;
//	int flipCounterUp;
//	int flipCounterDown;
//	int flipCounterLeft;
//	int flipCounterRight;
//	bool loop;
//	int currIndexX;
//	int currIndexY;
//	int totalItemsX;
//	int totalItemsY;
//	int oldCurrIndexX;
//	int oldCurrIndexY;
//	int startIndexX;
//	int startIndexY;
//	void SetTotalSize(int totalX, int totalY);
//};


struct VertSlider
{
	VertSlider();
	void Setup(sf::Vector2f &pos, sf::Vector2f &barSize,
		sf::Vector2f &selectorSize);
	void SetSlider(float f);
	void Draw(sf::RenderTarget *target);
	sf::Vector2f pos;
	sf::Vector2f barSize;
	sf::Vector2f selectorSize;
	sf::Vertex barVA[8];
};

struct SingleAxisSlider
{
	//eventually can optimize the drawing of this with a va
	SingleAxisSlider(sf::Vector2f &p_topMid,
		int numOptions, int startIndex,
		int width, int height);
	~SingleAxisSlider();
	int Update(ControllerState &currInput, ControllerState &prevInput);
	void UpdateSliderPos();
	void Draw(sf::RenderTarget *target);
	SingleAxisSelector *saSelector;

	sf::RectangleShape sliderRect;
	float leftPos;
	sf::RectangleShape scopeRect;
	sf::Vector2f topMid;
	sf::Vector2f size;
};

#endif
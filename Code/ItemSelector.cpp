#include "ItemSelector.h"

using namespace std;
using namespace sf;

SingleAxisSelector::SingleAxisSelector(int numWaitFramesLevels, int*p_waitFrames, int numWaitModeThreshLevels,
	int *p_waitModeThresh, int p_totalItems, int p_startIndex, bool p_loop)
	:startIndex( p_startIndex ), loop(p_loop), currIndex(p_startIndex), totalItems(p_totalItems)
{
	waitFrames = new int[numWaitFramesLevels];
	for (int i = 0; i < numWaitFramesLevels; ++i)
	{
		waitFrames[i] = p_waitFrames[i];
	}

	waitModeThresh = new int[numWaitModeThreshLevels];
	for (int i = 0; i < numWaitModeThreshLevels; ++i)
	{
		waitModeThresh[i] = p_waitModeThresh[i];
	}

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;
	oldCurrIndex = startIndex;
}

SingleAxisSelector::~SingleAxisSelector()
{
	delete[] waitFrames;
	delete[] waitModeThresh;
}

int SingleAxisSelector::UpdateIndex(bool dec, bool inc)
{
	oldCurrIndex = currIndex;
	if (inc)
	{
		if (flipCounterDown == 0
			|| (flipCounterDown > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterDown == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterDown;

			if (flipCounterDown == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterUp = 0;
			framesWaiting = 0;

			if (currIndex < totalItems - 1)
			{
				currIndex++;
			}
			else
			{
				if (!loop)
					return 0;

				currIndex = 0;
			}
			return 1;
		}
		else
		{
			++framesWaiting;
			return 0;
		}

	}
	else if (dec)
	{
		if (flipCounterUp == 0
			|| (flipCounterUp > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterUp == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterUp;

			if (flipCounterUp == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterDown = 0;
			framesWaiting = 0;
			if (currIndex > 0)
			{
				currIndex--;
			}
			else
			{
				if (!loop)
					return 0;

				currIndex = totalItems - 1;
			}
			return -1;
		}
		else
		{
			++framesWaiting;
			return 0;
		}

	}
	else
	{
		flipCounterUp = 0;
		flipCounterDown = 0;
		currWaitLevel = 0;
		framesWaiting = 0;
		return 0;
	}
}

void SingleAxisSelector::Reset()
{
	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;
	oldCurrIndex = startIndex;
	currIndex = startIndex;
}

VertSlider::VertSlider()
{
}

void VertSlider::Setup(Vector2f &p_pos, Vector2f &p_barSize, Vector2f &p_selectorSize)
{
	pos = p_pos;
	barSize = p_barSize;
	selectorSize = p_selectorSize;

	barVA[0].position = Vector2f(pos.x, pos.y);
	barVA[1].position = Vector2f(pos.x, pos.y) + Vector2f(selectorSize.x, 0);
	barVA[2].position = Vector2f(pos.x, pos.y) + selectorSize;
	barVA[3].position = Vector2f(pos.x, pos.y) + Vector2f(0, selectorSize.y);

	Color cbar = Color::Yellow;
	barVA[0].color = cbar;
	barVA[1].color = cbar;
	barVA[2].color = cbar;
	barVA[3].color = cbar;

	Color cSel = Color::Green;
	barVA[4].color = cSel;
	barVA[5].color = cSel;
	barVA[6].color = cSel;
	barVA[7].color = cSel;

	SetSlider(0);
}

void VertSlider::SetSlider(float f)
{
	float yDiff = (selectorSize.y - barSize.y) * f;
	barVA[4].position = Vector2f(pos.x, pos.y) + Vector2f(0, yDiff);
	barVA[5].position = Vector2f(pos.x, pos.y) + Vector2f(barSize.x, 0) + Vector2f(0, yDiff);
	barVA[6].position = Vector2f(pos.x, pos.y) + barSize + Vector2f(0, yDiff);
	barVA[7].position = Vector2f(pos.x, pos.y) + Vector2f(0, barSize.y) + Vector2f(0, yDiff);
}

void VertSlider::Draw(RenderTarget *target)
{
	target->draw(barVA, 8, sf::Quads);
}

SingleAxisSlider::SingleAxisSlider(Vector2f &p_topMid, int numOptions, int startIndex,
	int width, int height)
	:topMid(p_topMid)
{
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numOptions, startIndex, false);
	leftPos = topMid.x - (float)width / 2;

	size = Vector2f(width, height);

	scopeRect.setSize(Vector2f(width, height));
	scopeRect.setOrigin(scopeRect.getLocalBounds().width / 2, 0);
	scopeRect.setPosition(topMid);
	scopeRect.setFillColor(Color::Red);

	float sectionWidth = (float)size.x / (saSelector->totalItems - 1);

	sliderRect.setSize(Vector2f(sectionWidth, height));
	sliderRect.setOrigin(sliderRect.getLocalBounds().width / 2, 0);
	sliderRect.setFillColor(Color::Blue);

	UpdateSliderPos();
}

int SingleAxisSlider::Update(ControllerState &currInput, ControllerState &prevInput)
{
	int currIndex = saSelector->currIndex;

	bool left = currInput.LLeft();
	bool right = currInput.LRight();

	int changed = saSelector->UpdateIndex(left, right);
	int cIndex = saSelector->currIndex;

	bool inc = changed > 0;
	bool dec = changed < 0;

	if (changed != 0)
	{
		UpdateSliderPos();
	}

	return changed;
}

void SingleAxisSlider::UpdateSliderPos()
{
	float sectionWidth = (float)size.x / (saSelector->totalItems - 1);
	sliderRect.setPosition(Vector2f(leftPos + saSelector->currIndex * sectionWidth, topMid.y));
}

void SingleAxisSlider::Draw(sf::RenderTarget *target)
{
	target->draw(scopeRect);
	target->draw(sliderRect);
}

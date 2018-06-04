#ifndef __COLORSHIFTER_H__
#define __COLORSHIFTER_H__

#include <string>
#include <sfml/Graphics.hpp>

struct ColorShifter
{
	enum ShifterType
	{
		SEQUENTIAL,
		RANDOM,
	};

	ShifterType sType;
	ColorShifter(ShifterType type, int tFrames,
		int numColors );
	~ColorShifter();
	void SetColorIndex(int index,
		const sf::Color &c );
	void SetColors(sf::Image &palette, int row);
	void Update();
	void Reset();
	void SetState(float f,
		int pastIndex, int futureIndex);
	int numColors;
	sf::Color *colors;
	int transitionFrames;
	int pastIndex;
	int futureIndex;
	sf::Color GetCurrColor();
	int frame;
};

#endif
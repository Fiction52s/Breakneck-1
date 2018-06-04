#include <random>
#include "ColorShifter.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sf;

ColorShifter::ColorShifter(ShifterType type, int tFrames, int p_numColors )
	:sType(type), transitionFrames(tFrames), numColors( p_numColors )
{
	colors = new Color[numColors];
	Reset();
}

ColorShifter::~ColorShifter()
{
	delete[] colors;
}

void ColorShifter::SetColors(sf::Image &palette, int row )
{
	for (int i = 0; i < numColors; ++i)
	{
		colors[i] = palette.getPixel(i, row);
	}
}

void ColorShifter::Update()
{
	if (frame >= transitionFrames)
	{
		frame = 0;
		switch (sType)
		{
		case SEQUENTIAL:
			pastIndex++;
			if (pastIndex == numColors)
			{
				pastIndex = 0;
			}
			futureIndex++;
			if (futureIndex == numColors)
			{
				futureIndex = 0;
			}
			break;
		case RANDOM:
			break;
		}
	}
	else
	{
		frame++;
	}
}

void ColorShifter::SetColorIndex(int index,
	const sf::Color &c)
{
	colors[index] = c;
}

void ColorShifter::SetState(float f,
	int p_pastIndex, int p_futureIndex)
{
	frame = f * transitionFrames;
	pastIndex = p_pastIndex;
	futureIndex = p_futureIndex;
}

void ColorShifter::Reset()
{
	switch (sType)
	{
	case SEQUENTIAL:
		pastIndex = 0;
		if (numColors == 1)
		{
			futureIndex = 0;
		}
		else
		{
			futureIndex = 1;
		}
		break;
	case RANDOM:
	{
		pastIndex = rand() % numColors;
		int future = rand() % (numColors - 1);
		if (future >= pastIndex)
		{
			future++;
		}
		break;
	}
	}

	frame = 0;
}

sf::Color ColorShifter::GetCurrColor()
{
	Color pastColor = colors[pastIndex];//palette.getPixel(pastIndex, 0);
	Color futureColor = colors[futureIndex];//palette.getPixel(futureIndex, 0)
	float f = (float)frame / transitionFrames;
	float fr = 1.f - f;
	//cout << "f << " << f << ", fr: " << fr << endl;
	return Color(
		pastColor.r * fr + futureColor.r * f,
		pastColor.g * fr + futureColor.g * f,
		pastColor.b * fr + futureColor.b * f,
		pastColor.a * fr + futureColor.a * f);
}
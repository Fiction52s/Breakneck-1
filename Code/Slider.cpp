#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;

Slider::Slider(const std::string &n, sf::Vector2i &p_pos, int width, sf::Font &f,
	int p_min, int p_max, int p_defaultNum, Panel *p)
	:PanelMember(p), pos(p_pos), clickedDown(false), characterHeight(20), name(n),
	myFont(f), minValue(p_min), maxValue(p_max), defaultValue(p_defaultNum)
{
	floatSlider = false;
	size.y = 20;
	size.x = width;
	circleRad = 13;
	currValue = defaultValue;
	innerDisplayMode = false;

	displayText.setFont(f);
	displayText.setCharacterSize(characterHeight);
	displayText.setFillColor(Color::White);
	displayText.setOutlineThickness(2);
	displayText.setOutlineColor(Color::Black);

	SetRectColor(displayRect, Color(Color::Black));

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, Color(Color::Black));

	selectCircle.setFillColor(Color::Red);
	selectCircle.setRadius(circleRad);
	selectCircle.setOrigin(selectCircle.getLocalBounds().width / 2,
		selectCircle.getLocalBounds().height / 2);

	SetCurrValue(currValue);
}

void Slider::SetFloatMode(float p_min, float p_step)
{
	floatSlider = true;
	step = p_step;
	minDec = p_min;
	SetCurrValue(currValue);
}

void Slider::SetInnerDisplayMode(bool on)
{
	innerDisplayMode = on;
	SetCurrValue(currValue);
}

void Slider::Deactivate()
{
	clickedDown = false;
	panel->SendEvent(this);
}

void Slider::SetCircle(int x)
{
	selectCircle.setPosition(Vector2f(x, pos.y + size.y / 2));
}

void Slider::SetToFactor(float factor)
{
	currValue = CalcCurrValue(factor);

	if (!floatSlider)
	{
		displayText.setString(to_string(currValue));
	}
	else
	{
		float currFloat = GetCurrValueF();
		string floatStr = to_string(currFloat);

		displayText.setString(floatStr.substr(0, floatStr.find('.') + 3));
	}

	int currX = GetCurrX(factor);
	auto textBounds = displayText.getLocalBounds();
	displayText.setOrigin(textBounds.left + textBounds.width / 2,
		textBounds.top + textBounds.height / 2);
	//displayText.setPosition(Vector2f(currX, pos.y - textBounds.height / 2 - 10));
	if (innerDisplayMode)
	{
		displayText.setPosition(Vector2f(currX, pos.y + textBounds.height / 2));
	}
	else
	{
		displayText.setPosition(Vector2f(currX, pos.y - textBounds.height / 2 - 10));
	}
	SetRectCenter(displayRect, textBounds.width + 10, textBounds.height + 10, Vector2f(displayText.getPosition()));
	SetCircle(currX);
}

bool Slider::IsPointOnRect(sf::Vector2f &point)
{
	return QuadContainsPoint(mainRect, point);
}

float Slider::GetCurrValueF()
{
	assert(floatSlider);
	int offset = currValue - minValue;
	float curr = minDec + step * offset;
	return curr;
}

float Slider::GetCurrFactor(const sf::Vector2i &mousePos)
{
	Vector2i adjustedPos = mousePos - pos;
	if (adjustedPos.x >= size.x)
	{
		return 1.f;
	}
	else if (adjustedPos.x <= 0)
	{
		return 0.f;
	}
	else
	{
		float len = size.x;
		float curr = adjustedPos.x;

		float factor = curr / len;
		return factor;

		/*float totalDiff = maxValue - minValue;
		float fValue = minValue + (totalDiff * factor);
		int value = fValue;
		return value;*/
	}
}

void Slider::SetCurrValueF(float v)
{
	int curr = (v - minDec) / step;
	SetCurrValue(curr);
	//int currPos = (v - minDec)
}

void Slider::SetCurrValue(int v)
{
	currValue = v;
	float totalDiff = maxValue - minValue;
	float factor = (currValue - minValue) / totalDiff;

	if (!floatSlider)
	{
		displayText.setString(to_string(currValue));
	}
	else
	{
		float currFloat = GetCurrValueF();
		string floatStr = to_string(currFloat);

		displayText.setString(floatStr.substr(0, floatStr.find('.') + 3));
	}


	int currX = GetCurrX(factor);
	auto textBounds = displayText.getLocalBounds();
	displayText.setOrigin(textBounds.left + textBounds.width / 2,
		textBounds.top + textBounds.height / 2);
	if (innerDisplayMode)
	{
		displayText.setPosition(Vector2f(currX, pos.y + textBounds.height / 2));
	}
	else
	{
		displayText.setPosition(Vector2f(currX, pos.y - textBounds.height / 2 - 10));
	}
	
	SetRectCenter(displayRect, textBounds.width + 10, textBounds.height + 10, Vector2f(displayText.getPosition()));
	SetCircle(currX);
}

int Slider::GetCurrValue()
{
	return currValue;
}

int Slider::CalcCurrValue(float factor)
{
	float totalDiff = maxValue - minValue;
	float fValue = minValue + (totalDiff * factor);
	int value = fValue;
	return value;
}

int Slider::GetCurrX(float factor)
{
	float currXF = factor * size.x + pos.x;
	int currX = currXF;
	return currX;
}


bool Slider::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);
	bool mouseDown = MOUSE.IsMouseDownLeft();
	if (mouseDown && (IsPointOnRect(point) || clickedDown))
	{
		clickedDown = true;
		float currFactor = GetCurrFactor(mousePos);
		SetToFactor(currFactor);
		panel->SendEvent(this);
	}
	else if (!mouseDown)
	{
		clickedDown = false;
	}

	return true;
}

void Slider::Draw(sf::RenderTarget *target)
{
	if (hidden)
		return;

	target->draw(mainRect, 4, sf::Quads);
	target->draw(underRect, 4, sf::Quads);
	target->draw(selectCircle);
	if (!innerDisplayMode)
	{
		target->draw(displayRect, 4, sf::Quads);
	}
	
	target->draw(displayText);
}
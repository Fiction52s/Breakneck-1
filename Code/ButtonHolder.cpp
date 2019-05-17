#include "ButtonHolder.h"
#include <iostream>
using namespace std;

ButtonHolder::ButtonHolder(int m_holdLength)
{
	holdLength = m_holdLength;
}

void ButtonHolder::Update(bool held)
{
	if (held)
	{
		if (holdFrame < 0)
		{
			holdFrame = 1;
		}
		else
		{
			if (holdFrame == holdLength)
			{
			}
			else
			{
				++holdFrame;
			}
			
		}
	}
	else if( !IsHoldComplete() )
	{
		holdFrame = -1;
	}
}

void ButtonHolder::Reset()
{
	holdFrame = -1;
}

bool ButtonHolder::IsHoldComplete()
{
	return holdFrame == holdLength;
}
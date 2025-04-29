#include "ObjectProperties.h"

SlowableObject::SlowableObject()
{
	isSlowable = true;
}

void SlowableObject::SetSlowable(bool slowable)
{
	isSlowable = slowable;
}

bool SlowableObject::IsSlowed(int playerIndex) 
{ 
	return false;
}

int SlowableObject::GetSlowFactor(int playerIndex)
{
	return 1;
}

void SlowableObject::HandleSlowed()
{
}

void SlowableObject::ResetSlow()
{
	slowCounter = 1;
	slowMultiple = 1;
}

void SlowableObject::SlowCheck(int playerIndex)
{
	if (!isSlowable)
		return;

	if (IsSlowed(playerIndex))
	{
		if (slowMultiple > 1)
		{
			int newMult = GetSlowFactor(playerIndex);;
			/*if (slowCounter >= newMult)
			{
				slowCounter = 1;
			}*/

			slowMultiple = newMult;
		}

		if (slowMultiple == 1)
		{
			slowCounter = 1;

			//GetBubbleTimeFactor()
			slowMultiple = GetSlowFactor(playerIndex);//5;
		}
	}
	else
	{
		slowCounter = 1;
		slowMultiple = 1;
	}
}

bool SlowableObject::UpdateAccountingForSlow()
{
	if (slowCounter >= slowMultiple)
	{
		slowCounter = 1;
		return true;
	}
	else
	{
		slowCounter++;
		return false;
	}
}
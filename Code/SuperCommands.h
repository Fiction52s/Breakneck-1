#ifndef __SUPERCOMMANDS_H__
#define __SUPERCOMMANDS_H__

struct BirdCommand
{
	int action;
	bool facingRight;
	BirdCommand()
		:action(0),facingRight(true)
	{

	}
};

#endif
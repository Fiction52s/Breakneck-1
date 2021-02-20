#ifndef __BOSSCOMMAND_H__
#define __BOSSCOMMAND_H__

struct BossCommand
{
	BossCommand()
		:action(0), facingRight(true)
	{}
	int action;
	bool facingRight;
};

#endif
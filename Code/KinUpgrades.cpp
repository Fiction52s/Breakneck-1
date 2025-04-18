#include "KinUpgrades.h"
#include <string>

UpgradeLevels::UpgradeLevels()
{
	Clear();
}

void UpgradeLevels::Clear()
{
	for (int i = 0; i < UPGRADE_Count; ++i)
	{
		upgradeLevel[i] = 0;
	}
}

int UpgradeLevels::GetUpgradeLevel(int up)
{
	return upgradeLevel[up];
}


bool UpgradeLevels::HasUpgradeLevel(int up, int lvl )
{
	return upgradeLevel[up] >= lvl;
}

void UpgradeLevels::SetUpgradeLevel(int up, int lvl)
{
	upgradeLevel[up] = lvl;
}

void UpgradeLevels::Set(UpgradeLevels *ul)
{
	for (int i = 0; i < UPGRADE_Count; ++i)
	{
		upgradeLevel[i] = ul->upgradeLevel[i];
	}
}

bool UpgradeLevels::Load(std::istream &is)
{
	int lev;
	for (int i = 0; i < UPGRADE_Count; ++i)
	{
		is >> lev;
		upgradeLevel[i] = lev;
	}
	return true;
}

void UpgradeLevels::Save(std::ofstream &of)
{
	for (int i = 0; i < UPGRADE_Count; ++i)
	{
		of << int(upgradeLevel[i]) << "\n";
	}
}

bool UpgradeLevels::LoadBinary(std::istream &is)
{
	is.read((char*)upgradeLevel, sizeof(upgradeLevel[0]) * UPGRADE_Count);
	return true;
}

void UpgradeLevels::SaveBinary(std::ofstream &of)
{
	of.write((char*)upgradeLevel, sizeof(upgradeLevel[0]) * UPGRADE_Count);
}

#include "PlayerVisualInfo.h"

using namespace std;

PlayerVisualInfo::PlayerVisualInfo()
{
	Reset();
}

void PlayerVisualInfo::Reset()
{
	skinIndex = 0;
}

void PlayerVisualInfo::Save(ofstream &of)
{
	of << skinIndex << "\n";
}

void PlayerVisualInfo::Load(ifstream &is)
{
	is >> skinIndex;
}
#ifndef __PRACTICE_LOBBY_H__
#define __PRACTICE_LOBBY_H__

#include <list>


struct PracticeLobbyPlayer
{

};

struct PracticeLobbyDisplay
{
	enum PracticeLobbyType
	{
		PRACTICE_LOBBY_NONE,
		PRACTICE_LOBBY_PARALLEL_RACE,
		PRACTICE_LOBBY_CUSTOM_LOBBY,
	};

	int maxMembers;
	int practiceLobbyType;


	PracticeLobbyDisplay();

	void Clear();
	void Setup(int lobType, int maxMemb);
	void UpdateMembers();
};

#endif
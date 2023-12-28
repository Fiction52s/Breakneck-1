#include "ParallelPracticeLobbyHUD.h"
#include "Session.h"
#include "NetplayManager.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

ParallelPracticeLobbyHUD::ParallelPracticeLobbyHUD()
{
	MainMenu *mm = MainMenu::GetInstance();

	for (int i = 0; i < 4; ++i)
	{
		memberNameText[i].setFillColor(Color(255, 255, 255, 200));
		memberNameText[i].setFont(mm->arial);
		memberNameText[i].setCharacterSize(20);
		//memberNameText[i].setPosition(1920 - 10, 1080 - 30 - 30 * (3 - i ));
	}
}

void ParallelPracticeLobbyHUD::Update()
{
	Session *sess = Session::GetSession();
	
	//PracticePlayer *testPlayer;
	
	/*for (int i = 0; i < 4; ++i)
	{
		testPlayer = &sess->netplayManager->practicePlayers[i];
		if (testPlayer->isConnectedTo)
		{
			memberNameText.
		}
	}
	*/

	for (int i = 0; i < 4; ++i)
	{
		memberNameText[i].setString("");
	}

	int index = 0;
	auto &memberList = sess->netplayManager->lobbyManager->currentLobby.memberList;
	for (auto it = memberList.begin(); it != memberList.end(); ++it)
	{
		memberNameText[index].setString((*it).name);
		auto lb = memberNameText[index].getLocalBounds();
		memberNameText[index].setOrigin(lb.left + lb.width, 0);
		memberNameText[index].setPosition(1920 - 10, 1080 - 30 - 30 * ((memberList.size()-1) - index));

		index++;

		if (index == 4) //shouldn't happen but who knows
			break;
	}

	//for (int i = 1; i < 4; ++i)
	//{
	//	testPlayer = &sess->netplayManager->practicePlayers[i-1];
	//	//auto memberList = sess->netplayManager->lobbyManager->currentLobby.memberList;

	//	
	//	//if( true )
	//	if (testPlayer->isConnectedTo)
	//	{
	//		//memberNameText[i].setString("Fiction52s" + to_string(i));
	//		memberNameText[i].setString(testPlayer->name);
	//		auto lb = memberNameText[i].getLocalBounds();
	//		memberNameText[i].setOrigin(lb.left + lb.width, 0);
	//		memberNameText[i].setPosition(1920 - 10, 1080 - 30 - 30 * (3 - i));
	//	}
	//	else
	//	{
	//		memberNameText[i].setString("");
	//	}
	//}

}

void ParallelPracticeLobbyHUD::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < 4; ++i)
	{
		target->draw(memberNameText[i]);
	}
}
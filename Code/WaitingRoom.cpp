#include "WaitingRoom.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyManager.h"

using namespace std;
using namespace sf;

void PlayerBox::Init(WaitingRoom *wr, int p_index)
{
	index = p_index;

	playerNameStr = "";
	playerName = wr->panel->AddHyperLink("p" + to_string(index) + "link", Vector2i( 0, 0 ), 30, "", "");

	waitingRoom = wr;

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2i(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	Show();
}
void PlayerBox::Show()
{
	show = true;
	playerName->SetString(playerNameStr);
	SetTopLeft(topLeft);
}
void PlayerBox::Hide()
{
	show = false;
	playerName->SetString("");
}

void PlayerBox::SetTopLeft(sf::Vector2i &pos)
{
	topLeft = pos;

	Vector2i centerX(topLeft.x + waitingRoom->playerBoxWidth / 2, topLeft.y + waitingRoom->playerBoxHeight / 2);

	Vector2i namePos(waitingRoom->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, waitingRoom->playerBoxWidth, waitingRoom->playerBoxHeight, Vector2f(topLeft));

	playerName->SetPos(pos + namePos);
	auto &bounds = playerName->text.getLocalBounds();
	playerName->SetPos(Vector2i(playerName->pos.x - (bounds.left + bounds.width / 2), playerName->pos.y));
}

void PlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerName->SetString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void PlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);
}

WaitingRoom::WaitingRoom()
{
	TilesetManager::SetGameResourcesMode(false);

	panel = new Panel("waitingroom", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	startButton = panel->AddButton("start", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "START");
	leaveButton = panel->AddButton("leave", Vector2i(20 + 200, panel->size.y - 100), Vector2f(100, 40), "LEAVE");

	panel->ReserveTextRects(4);




	//panel->SetAutoSpacing(true, false, Vector2i(10, 500), Vector2i(0, 20));
	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i].Init(this, i);
		//players[i].playerName = panel->AddHyperLink("p" + to_string(i) + "link", Vector2i(0, 0), 30, "", "");
		//memberNameRects[i] = panel->AddTextRect(ChooseRect::I_LOBBY_MEMBER, Vector2f(), sf::Vector2f(200, 40), "");
		//memberNameRects[i]->Init();
		//memberNameRects[i]->SetShown(false);

	}

	Vector2f previewSize(912, 492);
	Vector2f previewPos(960 - previewSize.x / 2, 200);
	SetRectTopLeft(previewQuad, previewSize.x, previewSize.y, previewPos);

	previewBottomLeft = previewPos + Vector2f(0, previewSize.y);

	panel->StopAutoSpacing();

	panel->SetCancelButton(leaveButton);

	ts_preview = NULL;

	playerBoxWidth = 300;
	playerBoxHeight = 300;
	playerBoxSpacing = 100;
	
}

WaitingRoom::~WaitingRoom()
{
	delete panel;
}

void WaitingRoom::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	switch (action)
	{
	case A_WAITING_FOR_MEMBERS:
	{
		if (netplayManager->lobbyManager->GetNumMembers() == 2) //fix this later for more members
		{
			if (netplayManager->IsHost())
			{
				startButton->ShowMember();
			}
			action = A_READY_TO_START;
		}
		else
		{
			if (action == A_READY_TO_START)
			{
				action = A_WAITING_FOR_MEMBERS;
				startButton->HideMember();
			}
		}
		break;
	}
	case A_READY_TO_START:
	{
		break;
	}
	case A_STARTING:
	{
		break;
	}

	}

	

	panel->MouseUpdate();
}

void WaitingRoom::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i].Draw(target);
	}

	panel->Draw(target);
	
	if (ts_preview != NULL)
	{
		target->draw(previewQuad, 4, sf::Quads, ts_preview->texture);
	}
}

void WaitingRoom::SetPreview(const std::string &previewPath)
{
	ClearPreview();
	
	ts_preview = GetTileset(previewPath);

	if (ts_preview == NULL)
	{
		cout << "preview path failed: " << previewPath << endl;
		return;
	}

	SetRectSubRect(previewQuad, ts_preview->GetSubRect(0));

	//cout << "setting waiting room preview: " << previewPath << endl;

	if (ts_preview != NULL)
	{
	//	cout << "preview successfully set" << endl;
	}
}

void WaitingRoom::ClearPreview()
{
	if (ts_preview != NULL)
	{
		DestroyTileset(ts_preview);
		ts_preview = NULL;
	}
}

void WaitingRoom::SetMaxPlayers(int n)
{
	maxPlayers = n;

	for (int i = maxPlayers; i < 4; ++i)
	{
		playerBoxes[i].Hide();
	}

	Vector2i left(960, previewBottomLeft.y + 50);//150
	if (maxPlayers % 2 == 0)
	{
		left.x -= playerBoxSpacing / 2 + playerBoxWidth + (playerBoxSpacing + playerBoxWidth) * (maxPlayers / 2 - 1);
	}
	else
	{
		left.x -= playerBoxWidth / 2 + (playerBoxSpacing + playerBoxWidth) * (maxPlayers / 2);
	}

	//left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
	for (int i = 0; i < maxPlayers; ++i)
	{
		playerBoxes[i].SetTopLeft(left + Vector2i((playerBoxSpacing + playerBoxWidth) * i, 0));
		playerBoxes[i].Show();
	}

	/*Vector2i centerOrigin(960, previewBottomLeft.y + 100);
	if (maxPlayers == 2)
	{
		playerBoxes[0].SetTopLeft(centerOrigin + Vector2i(-playerBoxSpacing / 2 - playerBoxWidth, 0));
		playerBoxes[1].SetTopLeft(centerOrigin + Vector2i(playerBoxSpacing / 2 , 0));
	}
	else if (maxPlayers == 3)
	{
		playerBoxes[0].SetTopLeft(centerOrigin + Vector2i(-playerBoxWidth, 0));
		playerBoxes[1].SetTopLeft(centerOrigin + Vector2i(playerBoxSpacing / 2, 0));
		playerBoxes[1].SetTopLeft(centerOrigin + Vector2i(playerBoxSpacing / 2, 0));
	}*/
	
	//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));
}

void WaitingRoom::OpenPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	assert(netplayManager->lobbyManager->IsInLobby());

	netplayManager->lobbyManager->currWaitingRoom = this;

	startButton->HideMember();

	SetMaxPlayers(netplayManager->lobbyManager->currentLobby.data.maxMembers);

	//SetMaxPlayers(netplayManager->lobbyManager);

	UpdateMemberList();

	ownerID = netplayManager->GetHostID();

	/*if (netplayManager->IsHost())
	{
		startButton->ShowMember();
	}
	else
	{
		startButton->HideMember();
	}*/

	/*if (netplayManager->IsHost())
	{
		startButton->ShowMember();
	}
	else
	{

	}*/

	SetAction(A_WAITING_FOR_MEMBERS);
}

void WaitingRoom::ClosePopup()
{

}

void WaitingRoom::SetAction(Action a)
{
	action = a;
	frame = 0;

}

bool WaitingRoom::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void WaitingRoom::ChooseRectEvent(ChooseRect *cr, int eventType)
{

}

void WaitingRoom::ButtonCallback(Button *b, const std::string & e)
{
	if (b == startButton)
	{
		SetAction(A_STARTING);
	}
	else if (b == leaveButton)
	{
		SetAction(A_LEAVE_ROOM);
		//go back
	}
}

void WaitingRoom::PanelCallback(Panel *p, const std::string & e)
{

}

void WaitingRoom::CancelCallback(Panel *p)
{
	SetAction(A_LEAVE_ROOM);
}

void WaitingRoom::UpdateMemberList()
{
	for (int i = 0; i < 4; ++i)
	{
	//	memberNameRects[i]->SetShown(false);
		playerBoxes[i].playerName->SetLinkURL("");
		playerBoxes[i].playerName->SetString("");
	}

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	auto &memberList = netplayManager->lobbyManager->currentLobby.memberList;
	int index = 0;
	for (auto it = memberList.begin();
		it != memberList.end(); ++it)
	{
		playerBoxes[index].playerName->SetLinkURL("https://steamcommunity.com/profiles/" + to_string((*it).id.ConvertToUint64()));
		playerBoxes[index].SetName((*it).name);
		//memberNameRects[index]->SetText((*it).name);
		//memberNameRects[index]->SetShown(true);
		++index;
	}
}

void WaitingRoom::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	if (pCallback->m_ulSteamIDUserChanged == ownerID.ConvertToUint64())
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
		if (netplayManager->IsConnectedToHost() || netplayManager->IsHost() )
		{
			return;
		}


		uint32 flags = pCallback->m_rgfChatMemberStateChange;
		if ((flags & k_EChatMemberStateChangeLeft) || (flags & k_EChatMemberStateChangeDisconnected))
		{
			cout << "HOST LEFT THE LOBBY, SO I'LL LEAVE" << endl;
			SetAction(A_LEAVE_ROOM);
		}
	}
	else
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

		//assert(netplayManager->lobbyManager->IsInLobby());

		//netplayManager->lobbyManager->currWaitingRoom = this;
		netplayManager->OnLobbyChatUpdateCallback(pCallback);
	}
}

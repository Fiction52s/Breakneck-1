#include "ConnectionManager.h"

#include <iostream>
#include <string>

using namespace std;

ConnectionManager::ConnectionManager()
{
	SteamNetworkingUtils()->InitRelayNetworkAccess();

	connectionOwner = false;

	listenSocket = 0;

	connected = false;
}

void ConnectionManager::CreateListenSocket()
{
	listenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, NULL);
}

void ConnectionManager::OnMessagesSessionFailedCallback(SteamNetworkingMessagesSessionFailed_t *pCallback)
{
	cout << "messages session failed callback" << endl;

	SteamNetConnectionInfo_t info = pCallback->m_info;

}

void ConnectionManager::CloseConnection()
{	
	if (connected)
	{
		SteamNetworkingSockets()->CloseConnection(connection, 0, NULL, false);
		connected = false;
	}
	else if (listenSocket != 0)
	{
		//this destroys ALL connections. needs to be handled differently for more than 2 players
		SteamNetworkingSockets()->CloseListenSocket(listenSocket);
		listenSocket = 0;
	}
}

void ConnectionManager::Update()
{
	//int numMsges = NET()->ReceiveMessagesOnChannel(0, messages, 5);
	int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(connection, messages, 5);

	if (numMsges > 0)
	{
		cout << "got the message successfully" << endl;


		for (int i = 0; i < numMsges; ++i)
		{
			string s = (char*)(messages[i]->GetData());
			cout << "message " << i << ": " << s << endl;
		}
	}
}

void ConnectionManager::ConnectToID(CSteamID id)
{
	CSteamID myId = SteamUser()->GetSteamID();

	SteamNetworkingIdentity identity;
	identity.SetSteamID(id);

	connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);

	cout << "testing" << endl;
}
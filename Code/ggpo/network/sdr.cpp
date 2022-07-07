/* -----------------------------------------------------------------------
* GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
*
* Use of this software is governed by the MIT license that can be found
* in the LICENSE file.
*/

#include "types.h"
#include "sdr.h"
#include "udp.h"
#include <iostream>
#include "Session.h"
#include "NetplayManager.h"

using namespace std;

Sdr::Sdr() :
	listenConnection(0),
	_callbacks(NULL)
{
	sess = Session::GetSession();
}

Sdr::~Sdr(void)
{
}

void Sdr::Init(Poll *poll, Callbacks *callbacks)
{
	_callbacks = callbacks;

	_poll = poll;
	_poll->RegisterLoop(this);

	Log("initializing sdr instance\n");
}

void Sdr::SendTo(char *buffer, int len, int flags, HSteamNetConnection p_connection)
{
	UdpMsg *msg = (UdpMsg*)buffer;
	//cout << "sending msg thru sdr: " << msg->hdr.type << endl;
	EResult res = SteamNetworkingSockets()->SendMessageToConnection(p_connection, buffer, len, k_EP2PSendReliable, NULL);

	if (res == k_EResultOK)
	{
		Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
	}
	else
	{
		cout << "failed to send packet" << endl;
		//if you are failing to send, the connectionManager should notice!


		//currently this happens when the other player leaves the lobby. fix that.
		//Log("unknown error in sendto (erro: %d).\n", res);
		//ASSERT(FALSE && "Unknown error in sendto");
	}
}

bool Sdr::OnLoopPoll(void *cookie)
{
	if (listenConnection > 0) //make cleaner later
	{
		SteamNetworkingMessage_t *messages[1];

		for (;;)
		{
			for (auto it = sess->netplayManager->ggpoMessageQueue.begin(); it != sess->netplayManager->ggpoMessageQueue.end();)
			{
				
				if ((*it)->GetConnection() == listenConnection)
				{
					UdpMsg *msg = (UdpMsg *)(*it)->GetData();

					cout << "processing queued message: " << (int)msg->hdr.type << endl;

					if (msg->IsGameMsg())
					{
						sess->HandleMessage(listenConnection, (*it));

						//sess has to release its own messages
					}
					else
					{
						_callbacks->OnMsg(listenConnection, msg, (*it)->GetSize());

						(*it)->Release();
					}

					
					it = sess->netplayManager->ggpoMessageQueue.erase(it);
				}
				else
				{
					++it;
				}
			}


			int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(listenConnection, messages, 1);

			if (numMsges == 1)
			{
				Log("recvfrom returned (len:%d from %d).\n", messages[0]->GetSize(), listenConnection);
				UdpMsg *msg = (UdpMsg *)messages[0]->GetData();

				if (msg->IsGameMsg() )
				{
					sess->HandleMessage(listenConnection, messages[0]);
				}
				else
				{
					_callbacks->OnMsg(listenConnection, msg, messages[0]->GetSize());

					messages[0]->Release();
				}

				
			}
			else
			{
				break;
			}
		}

	}
	return true;
}

void Sdr::SetListenConnection(HSteamNetConnection p_connection)
{
	listenConnection = p_connection;
}


void Sdr::Log(const char *fmt, ...)
{
	char buf[1024];
	size_t offset;
	va_list args;

	strcpy_s(buf, "udp | ");
	offset = strlen(buf);
	va_start(args, fmt);
	vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
	buf[ARRAY_SIZE(buf) - 1] = '\0';
	::Log(buf);
	va_end(args);
}

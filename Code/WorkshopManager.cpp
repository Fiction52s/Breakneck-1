#include "WorkshopManager.h"
#include <iostream>

using namespace std;

void WorkshopTester::OnItemUpdatesSubmitted(SubmitItemUpdateResult_t *callback)
{
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "item updated successfully" << endl;
		break;
	default:
		cout << "failed to update item: " << (int)(callback->m_eResult) << endl;
		break;
	}
}

void WorkshopTester::OnCreatedItem(CreateItemResult_t *callback, bool bIOFailure)
{
	//char rgchString[256];
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "created item successfully" << endl;
		break;
	default:
		cout << "failed to create item: " << (int)(callback->m_eResult) << endl;
		break;
	}





	if (callback->m_eResult == k_EResultOK)
	{
		uploadedID = callback->m_nPublishedFileId;

		cout << "need legal agreement? " << (int)(callback->m_bUserNeedsToAcceptWorkshopLegalAgreement) << endl;

		UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), uploadedID);

		SteamUGC()->SetItemTitle(updateHandle, "b01");
		SteamUGC()->SetItemDescription(updateHandle, "test description");

		SteamUGC()->SetItemContent(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b01");
		SteamUGC()->SetItemPreview(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b01\\b01.png");
		SteamUGC()->SetItemVisibility(updateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

		SteamAPICall_t itemUpdateStatus = SteamUGC()->SubmitItemUpdate(updateHandle, NULL);

		OnSubmitItemUpdateResultCallResult.Set(itemUpdateStatus, this, &WorkshopTester::OnItemUpdated);
	}
	//sprintf_safe(rgchString, "SteamServerConnectFailure_t: %d\n", callback->m_eResult);
}

void WorkshopTester::OnItemUpdated(SubmitItemUpdateResult_t *callback, bool bIOFailure)
{
	//char rgchString[256];
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "edited item successfully" << endl;
		break;
	default:
		cout << "failed to edit item: " << (int)(callback->m_eResult) << endl;
		break;
	}
}

void WorkshopTester::OnQueryCompleted(SteamUGCQueryCompleted_t *callback, bool bIOFailure)
{
	//char rgchString[256];



	switch (callback->m_eResult)
	{
	case k_EResultOK:
	{
		cout << "query success. " << callback->m_unTotalMatchingResults << " items available" << endl;

		int numResultsReturned = callback->m_unNumResultsReturned;

		//
		for (int i = 0; i < numResultsReturned; ++i)
		{
			SteamUGCDetails_t details;
			bool success = SteamUGC()->GetQueryUGCResult(callback->m_handle, i, &details);
			if (success && details.m_eResult == EResult::k_EResultOK)
			{
				cout << i << "- " << details.m_rgchTitle << ": " << details.m_rgchDescription << endl;

				uint32 itemState = SteamUGC()->GetItemState(details.m_nPublishedFileId);

				if ((itemState & k_EItemStateSubscribed))
				{
					cout << "item is already subbed to" << endl;
				}
				else
				{
					cout << "subbing to item" << endl;
					SteamUGC()->SubscribeItem(details.m_nPublishedFileId);
				}

				if (itemState & k_EItemStateDownloading)
				{
					cout << "item is downloading" << endl;
				}
				else if (itemState & k_EItemStateInstalled)
				{
					cout << "item is already installed" << endl;
					//uint64 fileSize;
					//char path[1024];
					//uint32 timestamp;
					//cout << SteamUGC()->GetItemInstallInfo(details.m_nPublishedFileId, &fileSize, path, 1024, &timestamp);

					//cout << path << endl;

					//cout << details.

					//cout << "details: " << details.

					LoadWorkshopItem(details.m_nPublishedFileId);
				}
			}


		}


		break;
	}
	default:
		cout << "query failed" << endl;
		break;
	}

	SteamUGC()->ReleaseQueryUGCRequest(callback->m_handle);
}

//return true on success
bool WorkshopTester::LoadWorkshopItem(PublishedFileId_t workshopItemID)
{
	uint32 unItemState = SteamUGC()->GetItemState(workshopItemID);

	if (!(unItemState & k_EItemStateInstalled))
		return false;

	uint32 unTimeStamp = 0;
	uint64 unSizeOnDisk = 0;
	char szItemFolder[1024] = { 0 };

	if (!SteamUGC()->GetItemInstallInfo(workshopItemID, &unSizeOnDisk, szItemFolder, sizeof(szItemFolder), &unTimeStamp))
		return false;


	//is.open(folder

	cout << "folder: " << szItemFolder << endl;
}
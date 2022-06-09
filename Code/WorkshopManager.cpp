#include "WorkshopManager.h"
#include <iostream>

using namespace std;

//struct WorkshopManager
//{
//	WorkshopManager();
//	void UploadMap();
//	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
//	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
//	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
//	bool LoadWorkshopItem(PublishedFileId_t workshopItemID);
//private:
//
//	STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
//	//STEAM_CALLBACK(WorkshopManager, OnCreatedItem, CreateItemResult_t);
//
//};
//struct WorkshopManager
//{
//	WorkshopManager();
//	void UploadMap();
//	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
//	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
//	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
//	bool LoadWorkshopItem(PublishedFileId_t workshopItemID);
//private:
//	STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
//	//STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
//	//STEAM_CALLBACK(WorkshopManager, OnCreatedItem, CreateItemResult_t);
//
//};

//maybe put these in the struct?
CCallResult<WorkshopManager, CreateItemResult_t> OnCreateItemResultCallResult;
CCallResult<WorkshopManager, SubmitItemUpdateResult_t> OnSubmitItemUpdateResultCallResult;
CCallResult<WorkshopManager, SteamUGCQueryCompleted_t> OnQueryCompletedCallResult;

WorkshopManager::WorkshopManager()
{

}

void WorkshopManager::UploadMap()
{
	SteamAPICall_t hSteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity);
	OnCreateItemResultCallResult.Set(hSteamAPICall, this, &WorkshopManager::OnCreatedItem);
}

void WorkshopManager::OnItemUpdatesSubmitted(SubmitItemUpdateResult_t *callback)
{
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		//EItemUpdateStatus GetItemUpdateProgress(UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64*punBytesTotal);
		//EItemUpdateStatus itemStatus = SteamUGC()->GetItemUpdateProgress()
		cout << "item updated successfully" << endl;
		break;
	default:
		cout << "failed to update item: " << (int)(callback->m_eResult) << endl;
		break;
	}
}

void WorkshopManager::OnCreatedItem(CreateItemResult_t *callback, bool bIOFailure)
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
		cout << "need legal agreement? " << (int)(callback->m_bUserNeedsToAcceptWorkshopLegalAgreement) << endl;

		UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), callback->m_nPublishedFileId);

		SteamUGC()->SetItemTitle(updateHandle, "b01");
		SteamUGC()->SetItemDescription(updateHandle, "test description");

		SteamUGC()->SetItemContent(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b01");
		SteamUGC()->SetItemPreview(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b01\\b01.png");
		SteamUGC()->SetItemVisibility(updateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

		SteamAPICall_t itemUpdateStatus = SteamUGC()->SubmitItemUpdate(updateHandle, NULL);

		OnSubmitItemUpdateResultCallResult.Set(itemUpdateStatus, this, &WorkshopManager::OnItemUpdated);
	}
}

void WorkshopManager::OnItemUpdated(SubmitItemUpdateResult_t *callback, bool bIOFailure)
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

void WorkshopManager::OnQueryCompleted(SteamUGCQueryCompleted_t *callback, bool bIOFailure)
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
bool WorkshopManager::LoadWorkshopItem(PublishedFileId_t workshopItemID)
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

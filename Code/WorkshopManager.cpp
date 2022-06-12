
#include "WorkshopManager.h"
#include <iostream>
#include "MapBrowser.h"

using namespace std;

//maybe put these in the struct?
CCallResult<WorkshopManager, CreateItemResult_t> OnCreateItemResultCallResult;
CCallResult<WorkshopManager, SubmitItemUpdateResult_t> OnSubmitItemUpdateResultCallResult;
CCallResult<WorkshopManager, SteamUGCQueryCompleted_t> OnQueryCompletedCallResult;
CCallResult<WorkshopManager, HTTPRequestCompleted_t> OnHTTPRequestCompletedCallResult;

WorkshopManager::WorkshopManager()
{
	queryType = Q_TEST;
	queryState = QS_NOT_QUERYING;
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

void WorkshopManager::OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback, bool bIOFailure)
{
	if (callback->m_bRequestSuccessful)
	{
		uint32 bodySize;
		SteamHTTP()->GetHTTPResponseBodySize(callback->m_hRequest, &bodySize);

		uint8 *buffer = new uint8[bodySize];

		SteamHTTP()->GetHTTPResponseBodyData(callback->m_hRequest, buffer, bodySize);

		MapNode *matchingNode = NULL;
		for (auto it = queryResults->begin(); it != queryResults->end(); ++it)
		{
			if ((*it)->previewRequestHandle == callback->m_hRequest)
			{
				matchingNode = (*it);
				break;
			}
		}

		if (matchingNode == NULL)
		{
			cout << "no matching node!" << endl;
			assert(0);
		}
		else
		{
			matchingNode->previewTex = new sf::Texture;
			matchingNode->previewTex->loadFromMemory(buffer, bodySize);
			matchingNode->checkingForPreview = false;
			cout << "http query successful" << endl;
		}

		delete[] buffer;

		
	}
	else
	{
		cout << "failed to query http" << endl;
	}

	bool allChecksCompleted = true;
	for (auto it = queryResults->begin(); it != queryResults->end(); ++it)
	{
		if ((*it)->checkingForPreview )
		{
			allChecksCompleted = false;
			break;
		}
	}

	if (allChecksCompleted)
	{
		queryState = QS_NOT_QUERYING;
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

	bool querySuccess = false;
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "query success. " << callback->m_unTotalMatchingResults << " items available" << endl;
		querySuccess = true;
		break;
	default:
		cout << "query failed" << endl;
		querySuccess = false;
		break;
	}

	if (querySuccess)
	{
		if (queryType == Q_TEST)
		{
			int numResultsReturned = callback->m_unNumResultsReturned;
			queryResults->reserve(numResultsReturned);

			char urlTest[1024];

			for (int i = 0; i < numResultsReturned; ++i)
			{
				SteamUGCDetails_t details;
				bool success = SteamUGC()->GetQueryUGCResult(callback->m_handle, i, &details);
				if (success && details.m_eResult == EResult::k_EResultOK)
				{
					cout << i << "- " << details.m_rgchTitle << ": " << details.m_rgchDescription << endl;

					uint32 itemState = SteamUGC()->GetItemState(details.m_nPublishedFileId);

					MapNode *newNode = new MapNode;
					newNode->mapName = details.m_rgchTitle;
					newNode->description = details.m_rgchDescription;
					
					bool result = SteamUGC()->GetQueryUGCPreviewURL(callback->m_handle, 
						i, urlTest, 1024);
					if (result)
					{
						newNode->previewURL = urlTest;
					}

					queryResults->push_back(newNode);

					//details.m_hPreviewFile
					//MapNode *newNode = LoadWorkshopItem(details);

					
				}
			}
		}
		else
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

						//LoadWorkshopItem(details.m_nPublishedFileId);
					}
				}


			}
		}
	}

	SteamUGC()->ReleaseQueryUGCRequest(callback->m_handle);

	queryState = QS_NOT_QUERYING;
}

//return true on success
MapNode * WorkshopManager::LoadWorkshopItem(SteamUGCDetails_t &details)
{
	uint32 unItemState = SteamUGC()->GetItemState(details.m_nPublishedFileId);

	if (!(unItemState & k_EItemStateInstalled))
		return NULL;

	uint32 unTimeStamp = 0;
	uint64 unSizeOnDisk = 0;
	char szItemFolder[1024] = { 0 };

	if (!SteamUGC()->GetItemInstallInfo(details.m_nPublishedFileId, &unSizeOnDisk, szItemFolder, sizeof(szItemFolder), &unTimeStamp))
		return NULL;

	MapNode *newNode = new MapNode;
	newNode->folderPath = szItemFolder;
	newNode->mapName = details.m_rgchTitle;
	newNode->type = MapNode::FILE;
	newNode->filePath = string(szItemFolder) + "\\" + string(details.m_rgchTitle);
	//is.open(folder

	cout << "folder: " << szItemFolder << endl;

	return newNode;
}

void WorkshopManager::Query(std::vector<MapNode*> *p_queryResults)
{
	queryType = Q_TEST;
	queryState = QS_WAITING_FOR_RESULTS;
	queryResults = p_queryResults;
	auto queryHandle = SteamUGC()->CreateQueryAllUGCRequest(EUGCQuery::k_EUGCQuery_RankedByLastUpdatedDate,
		EUGCMatchingUGCType::k_EUGCMatchingUGCType_Items, SteamUtils()->GetAppID(),
		SteamUtils()->GetAppID(), 1);
	SteamUGC()->SetMatchAnyTag(queryHandle, true);
	auto sendRequestAPICall = SteamUGC()->SendQueryUGCRequest(queryHandle);

	OnQueryCompletedCallResult.Set(sendRequestAPICall, this, &WorkshopManager::OnQueryCompleted);
}

void WorkshopManager::DownloadPreviewFiles(std::vector<MapNode*> *p_previewResults)
{
	queryResults = p_previewResults;
	queryState = QS_WAITING_FOR_RESULTS;
	for (auto it = queryResults->begin(); it != queryResults->end(); ++it)
	{
		(*it)->checkingForPreview = false;
		if ((*it)->previewURL == "")
		{
			continue;
		}

		HTTPRequestHandle rh = SteamHTTP()->CreateHTTPRequest(
			EHTTPMethod::k_EHTTPMethodGET,
			(*it)->previewURL.c_str());

		

		if (rh == INVALID_HTTPREQUEST_HANDLE)
		{
			cout << "http request failed" << endl;
			
		}
		else
		{
			(*it)->previewRequestHandle = rh;
			(*it)->checkingForPreview = true;
			//SteamHTTP()->SendHTTPRequest( rh,)
			SteamAPICall_t call;
			bool httpResult = SteamHTTP()->SendHTTPRequest(rh, &call);
			if (!httpResult)
			{
				cout << "failed send" << endl;
			}
			else
			{
				OnHTTPRequestCompletedCallResult.Set(call, this,
					&WorkshopManager::OnHTTPRequestCompleted);
				cout << "send successful" << endl;
			}

		}
	}
	
}

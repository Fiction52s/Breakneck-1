
#include "WorkshopManager.h"
#include <iostream>
#include "MapBrowser.h"
#include "EditSession.h"
#include "MapPublishPopup.h"
#include "MapPostPublishPopup.h"
#include "MapPublishFailurePopup.h"
#include "MapPublishLoadingPopup.h"
#include "globals.h"

using namespace std;

//maybe put these in the struct?



WorkshopManager::WorkshopManager()
{
	queryType = Q_TEST;
	queryState = QS_NOT_QUERYING;
	queryResults = NULL;
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

void WorkshopManager::OnPersonaStateChange(PersonaStateChange_t *callback)
{
	if (callback->m_nChangeFlags & k_EPersonaChangeName)
	{
		if (queryResults == NULL)
		{
			return;
		}

		for (auto it = queryResults->begin(); it != queryResults->end(); ++it)
		{
			if ((*it)->creatorId == callback->m_ulSteamID)
			{
				if ((*it)->checkingForCreatorName)
				{
					(*it)->checkingForCreatorName = false;
					(*it)->creatorName = SteamFriends()->GetFriendPersonaName(callback->m_ulSteamID);
				}
			}
		}
	}
	else
	{
		int x = 5;
	}
	//callback->
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

		//OnSubmitItemUpdateResultCallResult.Set(itemUpdateStatus, this, &WorkshopManager::OnItemUpdated);
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

	queryTotalItems = -1;

	if (querySuccess)
	{
		if (queryType == Q_TEST)
		{

			int numResultsReturned = callback->m_unNumResultsReturned;
			queryTotalItems = callback->m_unTotalMatchingResults;
			queryResults->reserve(numResultsReturned);

			char urlTest[1024];
			char metaData[1024];

			for (int i = 0; i < numResultsReturned; ++i)
			{
				SteamUGCDetails_t details;
				bool success = SteamUGC()->GetQueryUGCResult(callback->m_handle, i, &details);
				if (success && details.m_eResult == EResult::k_EResultOK)
				{
					cout << i << "- " << details.m_rgchTitle << ": " << details.m_rgchDescription << endl;

					uint32 itemState = SteamUGC()->GetItemState(details.m_nPublishedFileId);

					bool getMetadataSuccess = SteamUGC()->GetQueryUGCMetadata(callback->m_handle, i, metaData, 1024);

					string fileName = metaData;

					if (fileName == "")
					{
						fileName = details.m_rgchTitle;
						cout << "metadata failed" << endl;
					}
					
					
					
					//SteamFriends()->GetFriendPersonaName

					//details.m_ulSteamIDOwner
					//details.m_rtimeUpdated

					MapNode *newNode = new MapNode;

					newNode->creatorId = details.m_ulSteamIDOwner;

					queryResults->push_back(newNode);

					bool needsToRequestInfo = SteamFriends()->RequestUserInformation(details.m_ulSteamIDOwner, true);

					if (needsToRequestInfo)
					{
						newNode->checkingForCreatorName = true;
					}
					else
					{
						newNode->creatorNameRetrieved = true;
						newNode->creatorName = SteamFriends()->GetFriendPersonaName(details.m_ulSteamIDOwner);
					}

					newNode->nodeName = details.m_rgchTitle;//fileName;
					newNode->fullMapName = details.m_rgchTitle;
					newNode->fileName = fileName;
					newNode->description = details.m_rgchDescription;
					newNode->publishedFileId = details.m_nPublishedFileId;
					newNode->mapDownloaded = itemState & k_EItemStateInstalled; //check if subscribed or not?
					newNode->isWorkshop = true;

					if (newNode->mapDownloaded)
					{
						uint64 fileSize;
						char path[1024];
						uint32 timestamp;
						cout << SteamUGC()->GetItemInstallInfo(details.m_nPublishedFileId, &fileSize, path, 1024, &timestamp);

						newNode->filePath = string(path) + "\\" + newNode->fileName + MAP_EXT;
						newNode->folderPath = path;
					}
					
					bool result = SteamUGC()->GetQueryUGCPreviewURL(callback->m_handle, 
						i, urlTest, 1024);
					if (result)
					{
						newNode->previewURL = urlTest;
					}

					
				}
			}
		}
	}

	SteamUGC()->ReleaseQueryUGCRequest(callback->m_handle);

	queryState = QS_NOT_QUERYING;
}

void WorkshopManager::Query(std::vector<MapNode*> *p_queryResults, int page)
{
	queryType = Q_TEST;
	queryState = QS_WAITING_FOR_RESULTS;
	queryResults = p_queryResults;
	auto queryHandle = SteamUGC()->CreateQueryAllUGCRequest(EUGCQuery::k_EUGCQuery_RankedByLastUpdatedDate,
		EUGCMatchingUGCType::k_EUGCMatchingUGCType_Items, SteamUtils()->GetAppID(),
		SteamUtils()->GetAppID(), page);
	SteamUGC()->SetMatchAnyTag(queryHandle, true);
	SteamUGC()->SetReturnMetadata(queryHandle, true);
	auto sendRequestAPICall = SteamUGC()->SendQueryUGCRequest(queryHandle);

	OnQueryCompletedCallResult.Set(sendRequestAPICall, this, &WorkshopManager::OnQueryCompleted);
}

void WorkshopManager::DownloadPreviewFiles(std::vector<MapNode*> *p_previewResults)
{
	queryResults = p_previewResults;
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
				(*it)->OnHTTPRequestCompletedCallResult.Set(call, (*it),
					&MapNode::OnHTTPRequestCompleted);
				cout << "send successful" << endl;
			}

		}
	}
	
}

void WorkshopManager::OnDownloadItemResult(DownloadItemResult_t *callback)
{
	if (callback->m_unAppID == SteamUtils()->GetAppID())
	{
		for (auto it = queryResults->begin(); it != queryResults->end(); ++it)
		{
			if ((*it)->publishedFileId == callback->m_nPublishedFileId)
			{
				(*it)->downloadResult = callback->m_eResult;
				break;
			}
		}
		/*if (callback->m_eResult == k_EResultNoConnection)
		{
			cout << "download no connection" << endl;
		}
		else if (callback->m_eResult == k_EResultTimeout)
		{
			cout << "download timeout" << endl;
		}
		else if (callback->m_eResult == k_EResultOK)
		{
			cout << "download success" << endl;
		}
		else
		{
			cout << "download result: " << (int)callback->m_eResult << endl;
		}*/
	}
}

WorkshopUploader::WorkshopUploader()
{
	edit = EditSession::GetSession();
	
	publishPopup = new MapPublishPopup;

	postPublishSuccessPopup = new MapPostPublishPopup;

	failurePopup = new MapPublishFailurePopup;

	loadingPopup = new MapPublishLoadingPopup;

	maxUploadAttempts = 50;
	currUploadAttempt = 0;

	assert(edit != NULL);
}

WorkshopUploader::~WorkshopUploader()
{
	delete publishPopup;
	delete postPublishSuccessPopup;
	delete failurePopup;
	delete loadingPopup;
}

void WorkshopUploader::PublishMap()
{
	SteamAPICall_t hSteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity);
	OnCreateItemResultCallResult.Set(hSteamAPICall, this, &WorkshopUploader::OnCreatedItem);

	
	currUploadAttempt = 0;
}

void WorkshopUploader::ActivatePublishPopup()
{
	publishPopup->Activate();
}

void WorkshopUploader::OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure)
{	
	//char rgchString[256];
	switch (pCallback->m_eResult)
	{
	case k_EResultOK:
		cout << "created item successfully" << endl;
		break;
	default:
		cout << "failed to create item: " << (int)(pCallback->m_eResult) << endl;
		break;
	}
	if (pCallback->m_eResult == k_EResultOK)
	{
		loadingPopup->Activate();
		loadingPopup->SetAttempt(0, maxUploadAttempts);
		TryUpdateItem( !pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement, pCallback->m_nPublishedFileId);
	}
	else
	{
		failurePopup->Activate(currUploadID, pCallback->m_eResult, true);
		//failure
	}
	//sprintf_safe(rgchString, "SteamServerConnectFailure_t: %d\n", pCallback->m_eResult);
}

void WorkshopUploader::TryUpdateItem( bool agreementSigned, PublishedFileId_t uploadId )
{
	cout << "need legal agreement? " << (int)(agreementSigned) << endl;

	UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), uploadId);

	string mapName = edit->filePath.stem().string();

	string publishTitle = publishPopup->mapNameTextBox->GetString();
		
	SteamUGC()->SetItemTitle(updateHandle, publishTitle.c_str()); //publishPopup->//mapName.c_str());//"b02");
	//SteamUGC()->SetItemTitle(updateHandle, mapName.c_str());//"b02");
	SteamUGC()->SetItemDescription(updateHandle, publishPopup->descriptionTextBox->GetString().c_str());
	//edit->mapHeader->description.c_str());//"test description 2");

	bool submitMetadataSuccess = SteamUGC()->SetItemMetadata(updateHandle, mapName.c_str());

	uploadFolder = boost::filesystem::current_path().append("\\testpublish");

	if (boost::filesystem::exists(uploadFolder))
	{
		try
		{
			boost::filesystem::remove_all(uploadFolder);
		}
		catch (const boost::filesystem::filesystem_error& e)
		{
			cout << "remove error: " << e.what() << endl;

			return;
		}
	}

	boost::filesystem::create_directory("testpublish");

	boost::filesystem::path previewUploadPath = uploadFolder.string()
		+ "\\" + mapName + ".png";

	boost::filesystem::path previewPath = edit->filePath.parent_path().string() 
		+ "\\" + edit->filePath.stem().string() + ".png";


	boost::filesystem::path uploadFilePath = uploadFolder.string() + "\\" + mapName + MAP_EXT;

	//eventually these need to be in try-catches because they can force the function to return
	//early
	try
	{
		boost::filesystem::copy_file(edit->filePath, uploadFilePath);
		boost::filesystem::copy_file(previewPath, previewUploadPath);
	}
	catch (const boost::filesystem::filesystem_error& e)
	{
		cout << "copy error: " << e.what() << endl;

		return;
	}
		
	SteamUGC()->SetItemContent( updateHandle, uploadFolder.string().c_str());
	SteamUGC()->SetItemPreview(updateHandle, previewUploadPath.string().c_str());
	SteamUGC()->SetItemVisibility(updateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

	SteamAPICall_t itemUpdateStatus = SteamUGC()->SubmitItemUpdate(updateHandle, NULL);

	OnSubmitItemUpdateResultCallResult.Set(itemUpdateStatus, this, &WorkshopUploader::OnItemUpdated);

	//bool signedAgreement = agreementSigned;//!(pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);

	currAgreementSigned = agreementSigned;
	currUploadID = uploadId;
}

void WorkshopUploader::OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure)
{
	if (boost::filesystem::exists(uploadFolder))
	{
		try
		{
			boost::filesystem::remove_all(uploadFolder);
		}
		catch (const boost::filesystem::filesystem_error& e)
		{
			cout << "remove error: " << e.what() << endl;

			return;
		}

	}
	//char rgchString[256];
	switch (pCallback->m_eResult)
	{
	case k_EResultOK:
		cout << "edited item successfully" << endl;
		loadingPopup->ClosePopup();
		postPublishSuccessPopup->Activate(currAgreementSigned, currUploadID);
		break;
	default:
		cout << "failed to edit item: " << (int)(pCallback->m_eResult) << endl;

		if (currUploadAttempt == maxUploadAttempts)
		{
			failurePopup->Activate(currUploadID, pCallback->m_eResult, false);
			loadingPopup->ClosePopup();

			SteamUGC()->DeleteItem(currUploadID);
		}
		else
		{
			++currUploadAttempt;
			loadingPopup->SetAttempt(currUploadAttempt, maxUploadAttempts);
			TryUpdateItem(currAgreementSigned, currUploadID);
			cout << "attempting to edit again: " << currUploadAttempt << endl;
		}
		break;
	}
}
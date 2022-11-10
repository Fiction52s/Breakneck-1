//#ifndef __WORKSHOPMANAGER_H__
//#define __WORKSHOPMANAGER_H__
#pragma once

#include "steam/steam_api.h"
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
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

struct MapNode;
struct MapBrowser;
struct EditSession;
struct MapPublishPopup;
struct MapPostPublishPopup;
struct MapPublishFailurePopup;
struct MapPublishLoadingPopup;

struct WorkshopManager
{
	enum QueryType
	{
		Q_TEST,
	};

	enum QueryState
	{
		QS_NOT_QUERYING,
		QS_WAITING_FOR_RESULTS,
	};

	QueryType queryType;
	QueryState queryState;
	std::vector<MapNode*> *queryResults;
	//MapBrowser *mapBrowser;
	int queryTotalItems;

	
	
	CCallResult<WorkshopManager, SteamUGCQueryCompleted_t> OnQueryCompletedCallResult;

	WorkshopManager();
	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback, bool bIOFailure);
	//void OnQueryAllCompleted(SteamUGCQueryCompleted_t *callback, bool bIOFailure);
	MapNode * LoadWorkshopItem(SteamUGCDetails_t &details );
	void Query(std::vector<MapNode*> *p_queryResults, int page );
	void DownloadPreviewFiles(std::vector<MapNode*> *p_previewResults);
private:
	STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
	STEAM_CALLBACK(WorkshopManager, OnPersonaStateChange, PersonaStateChange_t);
	STEAM_CALLBACK(WorkshopManager, OnDownloadItemResult, DownloadItemResult_t);
	//STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
	//STEAM_CALLBACK(WorkshopManager, OnCreatedItem, CreateItemResult_t);

};

struct WorkshopUploader
{
	CCallResult<WorkshopUploader, CreateItemResult_t> OnCreateItemResultCallResult;
	CCallResult<WorkshopUploader, SubmitItemUpdateResult_t> OnSubmitItemUpdateResultCallResult;

	EditSession *edit;

	boost::filesystem::path uploadFolder;

	bool currAgreementSigned;
	PublishedFileId_t currUploadID;


	MapPublishPopup *publishPopup;
	MapPostPublishPopup *postPublishSuccessPopup;
	MapPublishFailurePopup *failurePopup;
	MapPublishLoadingPopup *loadingPopup;

	int maxUploadAttempts;
	int currUploadAttempt;

	//sf::Sprite uploadingSprite;
	//Tileset *ts_upload;

	WorkshopUploader();
	~WorkshopUploader();
	void PublishMap();
	void ActivatePublishPopup();
	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
	void TryUpdateItem(bool agreementSigned, PublishedFileId_t id);
};



//#endif
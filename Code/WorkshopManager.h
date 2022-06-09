//#ifndef __WORKSHOPMANAGER_H__
//#define __WORKSHOPMANAGER_H__
#pragma once

#include "steam/steam_api.h"

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

struct WorkshopManager
{
	WorkshopManager();
	void UploadMap();
	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
	bool LoadWorkshopItem(PublishedFileId_t workshopItemID);
private:
	STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
	//STEAM_CALLBACK(WorkshopManager, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
	//STEAM_CALLBACK(WorkshopManager, OnCreatedItem, CreateItemResult_t);

};



//#endif
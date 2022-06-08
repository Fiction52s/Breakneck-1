#ifndef __WORKSHOPMANAGER_H__
#define __WORKSHOPMANAGER_H__

#include "steam/steam_api.h"

struct WorkshopTester
{
	PublishedFileId_t uploadedID;
	WorkshopTester();
	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
	bool LoadWorkshopItem(PublishedFileId_t workshopItemID);
private:
	//STEAM_CALLBACK(WorkshopTester, OnCreatedItem, CreateItemResult_t);
	STEAM_CALLBACK(WorkshopTester, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
};

//maybe put these in the struct?
CCallResult<WorkshopTester, CreateItemResult_t> OnCreateItemResultCallResult;
CCallResult<WorkshopTester, SubmitItemUpdateResult_t> OnSubmitItemUpdateResultCallResult;
CCallResult<WorkshopTester, SteamUGCQueryCompleted_t> OnQueryCompletedCallResult;

#endif
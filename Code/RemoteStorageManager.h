#ifndef __REMOTE_STORAGE_H__
#define __REMOTE_STORAGE_H__

#include "steam\steam_api.h"
#include <string>

struct SaveFile;

struct RemoteStorageResultHandler
{
	virtual void OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure) {}
};

struct RemoteStorageManager
{
	ISteamUser *m_pSteamUser;
	ISteamRemoteStorage *m_pSteamRemoteStorage;

	bool m_bFinished;
	int32 m_nNumFilesInCloud;
	uint64 m_ulBytesQuota;
	uint64 m_ulAvailableBytes;
	std::string destPath;

	RemoteStorageResultHandler *currHandler;

	RemoteStorageManager();
	void LoadAll();
	void Test();
	void GetFileStats();
	bool DownloadAndSave(const std::string &file);
	bool Upload(const std::string &file);
	bool UploadAsync(const std::string &file, RemoteStorageResultHandler *resultHandler );
	bool Upload(SaveFile *saveFile);
	SteamAPICall_t FileShare(const std::string &localPath);
	std::string GetRemotePath(const std::string &file);
private:
	CCallResult<RemoteStorageManager,
		RemoteStorageFileWriteAsyncComplete_t> onRemoteStorageFileWriteAsyncCompleteCallResult;

	void OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure);
};

#endif
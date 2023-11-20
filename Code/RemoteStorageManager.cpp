#include "RemoteStorageManager.h"
#include "boost/filesystem.hpp"
#include "globals.h"
#include <fstream>
#include <iostream>
#include "SaveFile.h"

using namespace std;

RemoteStorageManager::RemoteStorageManager()
{
	m_pSteamRemoteStorage = SteamRemoteStorage();

	destPath = "Resources\\remotestoragetesting\\";

	currHandler = NULL;

	GetFileStats();
}

void RemoteStorageManager::GetFileStats()
{
	m_ulBytesQuota = 0;
	m_ulAvailableBytes = 0;
	m_nNumFilesInCloud = m_pSteamRemoteStorage->GetFileCount();
	m_pSteamRemoteStorage->GetQuota(&m_ulBytesQuota, &m_ulAvailableBytes);
}

void RemoteStorageManager::Test()
{
	//Upload("globalsave.kingsave");
}

void RemoteStorageManager::LoadAll()
{
	if (boost::filesystem::exists(destPath))
	{
		boost::filesystem::remove_all(destPath);
	}
	boost::filesystem::create_directory(destPath);

	DownloadAndSave("globalsave" + string(GLOBAL_SAVE_EXT));

	/*string saveFileName = "blue";

	DownloadAndSave(saveFileName + string(SAVE_EXT));

	boost::filesystem::path saveFileReplayPath = path + saveFileName;

	if (boost::filesystem::exists(saveFileReplayPath))
	{
	boost::filesystem::remove_all(saveFileReplayPath);
	}
	boost::filesystem::create_directory(saveFileReplayPath);*/





	//	return;
}

bool RemoteStorageManager::DownloadAndSave(const std::string &file)
{
	if (!m_pSteamRemoteStorage->FileExists(file.c_str()))
		return false;

	int32 fileSize = m_pSteamRemoteStorage->GetFileSize(file.c_str());

	char *bytes = new char[fileSize];
	int32 bytesRead = m_pSteamRemoteStorage->FileRead(file.c_str(), bytes, sizeof(char) * fileSize - 1);

	string fullPath = destPath + file;

	FILE *filePtr = fopen(fullPath.c_str(), "wb");
	fwrite(bytes, 1, bytesRead, filePtr);
	fclose(filePtr);

	delete[] bytes;

	if (bytesRead > 0)
	{
		cout << "successfully downloaded: " << file << endl;
		return true;
	}
	else
	{
		cout << "failed to download " << file << endl;
		return false;
	}
}

bool RemoteStorageManager::Upload(const std::string &file)
{
	std::ifstream is;
	is.open(file);

	assert(is.is_open());
	std::string content((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	is.close();

	bool bRet = m_pSteamRemoteStorage->FileWrite(file.c_str(), content.c_str(), content.size());

	GetFileStats();

	if (bRet)
	{
		cout << "successfully uploaded: " << file << endl;
		return true;
	}
	else
	{
		cout << "failed to upload " << file << endl;
		return false;
	}
}

bool RemoteStorageManager::UploadAsync(const std::string &filePath, const std::string &cloudPath, RemoteStorageResultHandler *handler )
{
	currHandler = handler;

	std::ifstream is(filePath, ios_base::binary);
	//is.open(filePath);

	assert(is.is_open());
	std::string content((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	is.close();

	int size = content.size();

	//string cloudFilePath = GetRemotePath(cloudPath);
	SteamAPICall_t call = m_pSteamRemoteStorage->FileWriteAsync(cloudPath.c_str(), content.c_str(), size);
	onRemoteStorageFileWriteAsyncCompleteCallResult.Set(call, this, &RemoteStorageManager::OnRemoteStorageFileWriteAsyncComplete);


	GetFileStats();

	if (call != k_uAPICallInvalid)
	{
		cout << "async upload started: " << filePath << "\n";
		return true;
	}
	else
	{
		cout << "async upload failed to start: " << filePath << "\n";
		return false;
	}
}

bool RemoteStorageManager::Upload(SaveFile *saveFile)
{
	return Upload(saveFile->fileName);
}

SteamAPICall_t RemoteStorageManager::FileShare(const std::string &cloudPath)
{
	//string remotePath = GetRemotePath(localPath);
	return SteamRemoteStorage()->FileShare(cloudPath.c_str());
}

void RemoteStorageManager::OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure)
{
	assert(currHandler != NULL);
	currHandler->OnRemoteStorageFileWriteAsyncComplete(callback, bIOFailure);
}

std::string RemoteStorageManager::GetRemotePath( const std::string &file )
{
	string userAccountIDStr = to_string(SteamUser()->GetSteamID().GetAccountID());
	return userAccountIDStr + "\\" + file;
}
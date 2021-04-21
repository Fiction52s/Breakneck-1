#ifndef __CUSTOMMAPCLIENT_H__
#define __CUSTOMMAPCLIENT_H__

#ifdef __AWS__ON__

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <aws/cognito-identity/CognitoIdentityClient.h>
#include <aws/cognito-idp/CognitoIdentityProviderClient.h>

#include <aws/core/auth/AWSCredentials.h>

#include "nlohmann\json.hpp"

#include <Windows.h>
#include <winhttp.h>

//weird windows defines that mess with aws
#undef DELETE
#undef GetObject
#undef GetMessage

using json = nlohmann::json;

struct CustomMapEntry
{
	int id;
	std::string name;
	std::string creatorName;

	CustomMapEntry();
	void Set(const json &j);
	std::string GetMapFileName();
	std::string CreateKey();
};

namespace HttpVerb
{
	static LPCWSTR GET = L"GET";
	static LPCWSTR POST = L"POST";
	static LPCWSTR PUT = L"PUT";
	static LPCWSTR DELETE = L"DELETE";
}

struct ServerConnection
{
	HINTERNET myConnection;
	HINTERNET mySession;
	HINTERNET myRequest;
	std::string sessionHeaderName;
	LPCWSTR ContentType_JSON;
	std::wstring localHostRestBase;
	std::wstring publicServerRestBase;
	bool local;

	ServerConnection();
	HINTERNET OpenRequest(
		LPCWSTR verb, 
		LPCWSTR path);
	const std::wstring &GetRESTBase();
	DWORD GetRequestStatusCode();
	std::string GetRequestData();
	std::string GetRequestHeaders();
	void CleanupServerConnection();
	bool ConnectToServer(bool p_local);
	bool AddHeaderContentTypeJSON();
	bool AddHeaderSessionToken(
		const std::string &accessToken);
	bool SendRequestWithMessage(
		const std::string &message);
	bool SendRequest();
	bool RequestMapUpload(const std::string &mapName, 
		const std::string &accessToken,
		bool overwriteIfExists );
	bool RequestMapDeletion(int id, 
		const std::string & accessToken);
	bool RequestMapDownload(int id);
	bool RequestGetMapList(
		std::vector<CustomMapEntry> &entryVec);
};

struct S3Interface
{
	Aws::S3::S3Client *s3Client;
	std::string bucketName;
	static Aws::String downloadDest;

	S3Interface();

	void InitWithCredentials(
		const Aws::Auth::AWSCredentials &creds);
	void UploadObject(
		const Aws::String &path, 
		const Aws::String &file, 
		const Aws::String &username);
	void DownloadObject(
		const Aws::String &downloadPath, 
		const Aws::String &key, 
		const Aws::String &file);
};

struct CognitoInterface
{
	std::shared_ptr<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> identityProviderClient;
	std::shared_ptr<Aws::CognitoIdentity::CognitoIdentityClient> identityClient;
	bool isLoggedIn;
	std::string tokenType;
	std::string accessToken;
	std::string IDToken;
	std::string refreshToken;
	std::string username;
	Aws::Auth::AWSCredentials currCreds;

	CognitoInterface();
	const char * const &GetAccessToken();
	void InitWithCredentials(
		const Aws::Auth::AWSCredentials &creds);
	bool TryLogIn(
		const std::string &user, 
		const std::string &pass);
};

struct CustomMapClient
{
	CustomMapClient();
	~CustomMapClient();
	void AnonymousInit();
	void Cleanup();
	bool AttemptDeleteMapFromServer(
		CustomMapEntry &entry);
	bool AttemptUploadMapToServer(
		const std::string &path, 
		const std::string &mapName, 
		bool overwriteIfExists = false );
	bool AttemptDownloadMapFromServer(
		const std::string &downloadPath, 
		CustomMapEntry &entry);
	bool AttempGetMapListFromServer();
	void PrintMapEntries();
	bool AttemptUserLogin(
		const std::string &user, 
		const std::string &pass);
	bool IsLoggedIn();

	std::vector<CustomMapEntry> mapEntries;

private:
	S3Interface s3Interface;
	ServerConnection serverConn;
	CognitoInterface cognitoInterface;
	Aws::SDKOptions AWSSDKOptions;

};

#endif

#endif
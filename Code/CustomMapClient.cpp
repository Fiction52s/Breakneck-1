#include "AppClientID.h"
#include "CustomMapClient.h"

#include <aws/identity-management/auth/CognitoCachingCredentialsProvider.h>

#include <aws/core/utils/logging/ConsoleLogSystem.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <fstream>

#include <aws/cognito-idp/model/InitiateAuthRequest.h>
#include <aws/cognito-idp/model/ChangePasswordRequest.h>
#include <aws/cognito-idp/model/RespondToAuthChallengeRequest.h>

#include <aws/cognito-identity/model/GetIdRequest.h>
#include <aws/cognito-identity/model/GetCredentialsForIdentityRequest.h>

using namespace std;
using namespace Aws;
using namespace Aws::CognitoIdentity::Model;

CustomMapEntry::CustomMapEntry()
{

}

void CustomMapEntry::Set(const json &j)
{
	id = j["id"];
	name = j["name"];
	creatorName = j["creatorName"];
}

std::string CustomMapEntry::GetMapFileName()
{
	return string(name + ".brknk");
}

std::string CustomMapEntry::CreateKey()
{
	return string(creatorName + "/" + GetMapFileName());
}

ServerConnection::ServerConnection()
{
	sessionHeaderName = "Session-Token:";
	ContentType_JSON = L"Content-Type:application/json";
	myConnection = NULL;
	mySession = NULL;
	myRequest = NULL;

	localHostRestBase = L"/MapServer/rest/";

	publicServerRestBase = L"/rest/";

	local = true;
}

HINTERNET ServerConnection::OpenRequest(LPCWSTR verb, LPCWSTR path)
{
	HINTERNET req = NULL;
	if (myConnection != NULL)
		req = WinHttpOpenRequest(myConnection, verb, path,
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	return req;
}

const wstring &ServerConnection::GetRESTBase()
{
	if (local)
	{
		return localHostRestBase;
	}
	else
	{
		return publicServerRestBase;
	}
}

DWORD ServerConnection::GetRequestStatusCode()
{
	DWORD statusCode = 0;
	DWORD statusCodeSize = sizeof(DWORD);

	//get the status code
	if (!WinHttpQueryHeaders(myRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode, &statusCodeSize,
		WINHTTP_NO_HEADER_INDEX))
	{
		DWORD error = HRESULT_FROM_WIN32(::GetLastError());
		cout << "Error getting error code: " << error << endl;
		return error;
	}
	else
	{
		//cout << "status code: " << statusCode << endl;
		return statusCode;
	}
}

string ServerConnection::GetRequestData()
{
	DWORD dwSize = 0;
	LPSTR pszOutBuffer = NULL;
	DWORD dwDownloaded = 0;
	string response;

	do
	{
		// Check for available data.
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(myRequest, &dwSize))
			printf("Error %u in WinHttpQueryDataAvailable.\n",
				GetLastError());

		if (!dwSize)
		{
			break;
		}

		// Allocate space for the buffer.
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer)
		{
			printf("Out of memory\n");
			dwSize = 0;
		}
		else
		{
			// Read the data.
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (!WinHttpReadData(myRequest, (LPVOID)pszOutBuffer,
				dwSize, &dwDownloaded))
			{
				printf("Error %u in WinHttpReadData.\n", GetLastError());
				return response;
			}
			else
			{
				//printf("%s", pszOutBuffer);
				response = response + string(pszOutBuffer);
			}

			// Free the memory allocated to the buffer.
			delete[] pszOutBuffer;
		}
	} while (dwSize > 0);


	return response;
}

string ServerConnection::GetRequestHeaders()
{
	DWORD dwSize = 0;
	LPCWSTR lpOutBuffer = NULL;
	BOOL bResults;
	string result;

	WinHttpQueryHeaders(myRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX, NULL,
		&dwSize, WINHTTP_NO_HEADER_INDEX);

	// Allocate memory for the buffer.
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];

		// Now, use WinHttpQueryHeaders to retrieve the header.
		bResults = WinHttpQueryHeaders(myRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			(LPVOID)lpOutBuffer, &dwSize,
			WINHTTP_NO_HEADER_INDEX);

		if (bResults)
		{
			wstring wideBufferStr(lpOutBuffer);
			result = string(wideBufferStr.begin(), wideBufferStr.end());
		}

		//printf("Header contents: \n%S", lpOutBuffer);

		delete[] lpOutBuffer;
	}

	return result;
}

void ServerConnection::CleanupServerConnection()
{
	if (myConnection != NULL) WinHttpCloseHandle(myConnection);
	if (mySession != NULL) WinHttpCloseHandle(mySession);

	myConnection = NULL;
	mySession = NULL;
}

bool ServerConnection::ConnectToServer( bool p_local )
{
	local = p_local;

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	BOOL  bResults = FALSE;

	assert(mySession == NULL);
	assert(myConnection == NULL);

	mySession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (mySession != NULL)
	{
		if (local)
		{
			myConnection = WinHttpConnect(mySession, L"localhost",
				8080, 0);
		}
		else
		{
			myConnection = WinHttpConnect(mySession, 
				L"www.please.eba-gkrhenjs.us-west-2.elasticbeanstalk.com",
				80, 0);
		}
	}

	if (myConnection == NULL)
	{
		CleanupServerConnection();
		mySession = NULL;
		return false;
	}
	else
	{
		return true;
	}
}

bool ServerConnection::AddHeaderContentTypeJSON()
{
	return WinHttpAddRequestHeaders(myRequest, ContentType_JSON, -1, WINHTTP_ADDREQ_FLAG_ADD);
}

bool ServerConnection::AddHeaderSessionToken(const string &accessToken)
{
	string sessionHeader = sessionHeaderName + accessToken;
	wstring wideSessionHeader = wstring(sessionHeader.begin(), sessionHeader.end());
	LPCWSTR wsh = wideSessionHeader.c_str();

	return WinHttpAddRequestHeaders(myRequest, wsh, -1, WINHTTP_ADDREQ_FLAG_ADD);
}

bool ServerConnection::SendRequestWithMessage(const string &message)
{
	const LPSTR messageBuf = (LPTSTR)message.c_str();
	return WinHttpSendRequest(myRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, messageBuf, strlen(messageBuf), strlen(messageBuf), 0);
}

bool ServerConnection::SendRequest()
{
	return WinHttpSendRequest(myRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0,
		0, 0);
}

bool ServerConnection::RequestMapUpload(const string &mapName, const string &accessToken,
	bool overwriteIfExists )
{
	if (myConnection == NULL)
	{
		return false;
	}

	wstring path = GetRESTBase() + L"maps";
	myRequest = OpenRequest(HttpVerb::POST, path.c_str());

	bool okay = false;
	if (myRequest != NULL)
	{
		AddHeaderContentTypeJSON();
		AddHeaderSessionToken(accessToken);

		json j;
		j["name"] = mapName;
		string message = j.dump();

		/*string message = "{"
		"\"name\":\"" + mapName + "\""
		"}";*/

		if (SendRequestWithMessage(message))
		{
			if (WinHttpReceiveResponse(myRequest, NULL))
			{
				int statusCode = GetRequestStatusCode();
				//string headers = GetRequestHeaders();
				//string data = GetRequestData();

				//cout << "status code: " << statusCode << endl;
				//cout << "headers: " << endl;
				//cout << headers << endl;

				//cout << "return data:" << endl;
				//cout << data << endl;

				if (statusCode == 200)
				{
					cout << "you are allowed to upload the map." << endl;
					//string fullPath = username + "/" + mapName;
					//Aws::String awsFullPath(fullPath.c_str());
					okay = true;
				}
				else if (statusCode == 403) //not modified for now
				{
					cout << "you aren't allowed to upload the map. Username is invalid" << endl;
				}
				else if (statusCode == 302)
				{
					cout << "map already exists";
					
					if (overwriteIfExists)
					{
						//NOTE: this is only for development and shouldn't be in the real game.
						okay = true;
					}
					
				}
				else
				{
					cout << "status unexpected while uploading: " << statusCode << endl;
				}
				//process POST result here to see if its okay to upload
			}
			else
			{
				cout << "response not received." << endl;
			}
		}
		else
		{
			cout << "sending create map request failed" << endl;
		}

		WinHttpCloseHandle(myRequest);
		myRequest = NULL;
	}
	else
	{
		cout << "failed to create request" << endl;
	}

	return okay;
}

bool ServerConnection::RequestMapDeletion(int id, const string & accessToken)
{
	if (myConnection == NULL)
	{
		return false;
	}

	wstring path = GetRESTBase() + L"maps/" + to_wstring(id);
	myRequest = OpenRequest(HttpVerb::DELETE, path.c_str());

	bool okay = false;

	if (myRequest != NULL)
	{
		AddHeaderSessionToken(accessToken);

		if (SendRequest())
		{
			if (WinHttpReceiveResponse(myRequest, NULL))
			{
				int statusCode = GetRequestStatusCode();
				if (statusCode == 200)
				{
					cout << "map has been deleted" << endl;
					okay = true;
				}
				else if (statusCode == 302)
				{
					cout << "you do not have permission to delete the map. status code: " << statusCode << endl;
				}
				else
				{
					cout << "error trying to delete map. status code: " << statusCode << endl;
				}
			}
		}
		else
		{
			cout << "sending create map request failed" << endl;
		}

		WinHttpCloseHandle(myRequest);
		myRequest = NULL;
	}
	else
	{
		cout << "failed to create request" << endl;
	}

	return okay;
}

bool ServerConnection::RequestMapDownload(int id)
{
	if (myConnection == NULL)
	{
		return false;
	}

	wstring path = GetRESTBase() + L"maps/" + to_wstring( id );
	myRequest = OpenRequest(HttpVerb::GET, path.c_str());

	bool found = false;

	if (myRequest != NULL)
	{
		if (SendRequest())
		{
			if (WinHttpReceiveResponse(myRequest, NULL))
			{
				int statusCode = GetRequestStatusCode();
				if (statusCode == 200)
				{
					cout << "map exists. you can download it." << endl;
					found = true;
				}
				else if (statusCode == 404)
				{
					cout << "map doesn't exist. you can't download it." << endl;
				}
				else
				{
					cout << "error checking for map existence. status code: " << statusCode << endl;
				}
			}
		}
		else
		{
			cout << "sending get request failed" << endl;
		}

		WinHttpCloseHandle(myRequest);
		myRequest = NULL;
	}
	else
	{
		cout << "failed to create request" << endl;
	}

	return found;
}

bool ServerConnection::RequestGetMapList(std::vector<CustomMapEntry> &entryVec)
{
	//myRequest = OpenRequest(HttpVerb::GET, L"/MapServer/rest/maps");
	wstring path = GetRESTBase() + L"maps";
	//if( local)
	myRequest = OpenRequest(HttpVerb::GET, path.c_str());

	if (myRequest != NULL)
	{
		//AddHeaderSessionToken();

		if (SendRequest())
		{
			if (WinHttpReceiveResponse(myRequest, NULL))
			{
				int statusCode = GetRequestStatusCode();
				string data = GetRequestData();

				/*string headers = GetRequestHeaders();


				cout << "status code: " << statusCode << endl;
				cout << "headers: " << endl;
				cout << headers << endl;

				cout << "return data:" << endl;
				cout << data << endl;*/

				auto mapListJSON = json::parse(data);
				int numEntries = mapListJSON.size();
				entryVec.resize(numEntries);
				for (int i = 0; i < numEntries; ++i)
				{
					entryVec[i].Set(mapListJSON[i]);
				}

				return true;
			}
		}
		else
		{
			cout << "sending get request failed" << endl;
		}

		WinHttpCloseHandle(myRequest);
		myRequest = NULL;
	}
	else
	{
		cout << "failed to create request" << endl;
	}

	return false;
}

Aws::String S3Interface::downloadDest = "";

S3Interface::S3Interface()
{
	bucketName = "breakneckmaps";
	s3Client = NULL;
}

void S3Interface::InitWithCredentials(const Aws::Auth::AWSCredentials &creds)
{
	if (s3Client != NULL)
	{
		Aws::Delete(s3Client);
		s3Client = NULL;
	}
	s3Client = Aws::New<Aws::S3::S3Client>("s3client", creds);
}

void S3Interface::UploadObject(const Aws::String &path, const Aws::String &file, const Aws::String &username)
{
	//must be logged in as a user to upload

	//if (!s_IsLoggedIn)
	//{
	//	cout << "tried to upload, but aren't logged in" << endl;
	//	return;
	//}

	//mapName = map;
	cout << "uploading: " << file << endl;

	Aws::String uploadPath = Aws::String(username.c_str()) + "/" + file;

	Aws::String filePath = path + file;

	Aws::S3::Model::PutObjectRequest putReq;
	putReq.WithBucket(bucketName.c_str());
	putReq.WithKey(uploadPath);

	auto fileToUpload = Aws::MakeShared<Aws::FStream>("uploadstream", filePath.c_str(), std::ios_base::in | std::ios_base::binary);

	putReq.SetBody(fileToUpload);
	//putReq.SetKey("test/" + file);
	auto outcome = s3Client->PutObject(putReq);

	if (outcome.IsSuccess())
	{
		cout << "upload " << file << " sucess!" << endl;
	}
	else
	{
		std::cout << "PutObject error: " <<
			outcome.GetError().GetExceptionName() << " " <<
			outcome.GetError().GetMessage() << std::endl;
	}
}

void S3Interface::DownloadObject(const Aws::String &downloadPath, const Aws::String &key, const Aws::String &file)
{
	//assumes its a map
	downloadDest = downloadPath + file;
	cout << "downloading: " << file << endl;

	string *test = new string;

	Aws::S3::Model::GetObjectRequest getReq;
	getReq.WithBucket(bucketName.c_str());
	getReq.WithKey(key);//"gateblank9.brknk");
	getReq.SetResponseStreamFactory([]() {return Aws::New<Aws::FStream>("mapfstream", downloadDest.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary); });

	auto outcome = s3Client->GetObject(getReq);

	if (outcome.IsSuccess())
	{
		cout << "download sucess!" << endl;
	}
	else
	{
		std::cout << "GetObject error: " <<
			outcome.GetError().GetExceptionName() << " " <<
			outcome.GetError().GetMessage() << std::endl;
	}
}




CognitoInterface::CognitoInterface()
{
	isLoggedIn = false;
}

const char * const &CognitoInterface::GetAccessToken()
{
	assert(isLoggedIn);
	return accessToken.c_str();
}

void CognitoInterface::InitWithCredentials(const Aws::Auth::AWSCredentials &creds)
{
	if (identityProviderClient == NULL)
	{
		currCreds = creds;
		Aws::Client::ClientConfiguration clientConfiguration;
		clientConfiguration.region = Aws::Region::US_EAST_1;

		if (identityProviderClient != NULL)
		{
			identityProviderClient = NULL; //deletes because its a shared_ptr
		}
		identityProviderClient = Aws::MakeShared<Aws::CognitoIdentityProvider::
			CognitoIdentityProviderClient>("CognitoIdentityProviderClient", currCreds, clientConfiguration);

		identityClient = Aws::MakeShared<Aws::CognitoIdentity::CognitoIdentityClient>("clienttest", currCreds, clientConfiguration);
	}
	else
	{
		cout << "cognito interface already initialized!";
		assert(0);
	}
}

bool CognitoInterface::TryLogIn(const std::string &user, const std::string &pass)
{
	if (isLoggedIn)
	{
		assert(0);
		return false;
	}

	Aws::Http::HeaderValueCollection authParameters{
		{ "USERNAME", user.c_str() },
		{ "PASSWORD", pass.c_str() }
	};

	Aws::CognitoIdentityProvider::Model::InitiateAuthRequest initiateAuthRequest;
	initiateAuthRequest.SetClientId(APP_CLIENT_ID);
	initiateAuthRequest.SetAuthFlow(Aws::CognitoIdentityProvider::Model::AuthFlowType::USER_PASSWORD_AUTH);
	initiateAuthRequest.SetAuthParameters(authParameters);
	Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome initiateAuthOutcome{ identityProviderClient->InitiateAuth(initiateAuthRequest) };

	if (initiateAuthOutcome.IsSuccess())
	{
		Aws::CognitoIdentityProvider::Model::InitiateAuthResult initiateAuthResult{ initiateAuthOutcome.GetResult() };
		auto challengeName = initiateAuthResult.GetChallengeName();
		cout << "challengeName: " << (int)challengeName << endl;
		if (challengeName == Aws::CognitoIdentityProvider::Model::ChallengeNameType::NOT_SET)
		{
			// for this code sample, this is what we expect, there should be no further challenges
			// there are more complex options, for example requiring the user to reset the password the first login
			// or using a more secure password transfer mechanism which will be covered in later examples
			Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult = initiateAuthResult.GetAuthenticationResult();
			cout << endl << "Congratulations, you have successfully signed in as user: " << user << endl;
			/*cout << "\tToken Type: " << authenticationResult.GetTokenType() << endl;
			cout << "\tAccess Token: " << authenticationResult.GetAccessToken().substr(0, 20) << " ..." << endl;
			cout << "\tExpires in " << authenticationResult.GetExpiresIn() << " seconds" << endl;
			cout << "\tID Token: " << authenticationResult.GetIdToken().substr(0, 20) << " ..." << endl;
			cout << "\tRefresh Token: " << authenticationResult.GetRefreshToken().substr(0, 20) << " ..." << endl;*/

			isLoggedIn = true;
			tokenType = authenticationResult.GetTokenType().c_str();
			accessToken = authenticationResult.GetAccessToken().c_str();
			IDToken = authenticationResult.GetIdToken().c_str();
			refreshToken = authenticationResult.GetRefreshToken().c_str();

			Aws::CognitoIdentity::Model::GetIdRequest idreq;
			idreq.AddLogins("cognito-idp.us-east-1.amazonaws.com/us-east-1_6v9AExXS8", IDToken.c_str());
			idreq.SetAccountId("942521585968");
			idreq.SetIdentityPoolId("us-east-1:e8840b78-d9e3-4c03-8d6b-a9bdd5833fbd");
			auto getidoutcome = identityClient->GetId(idreq);
			Aws::String identityID;
			if (getidoutcome.IsSuccess())
			{
				auto idresult = getidoutcome.GetResult();
				identityID = idresult.GetIdentityId();
			}
			else
			{
				cout << "GET ID OUTCOME FAILED" << endl;
			}

			Aws::CognitoIdentity::Model::GetCredentialsForIdentityRequest cred_request;

			cred_request.AddLogins("cognito-idp.us-east-1.amazonaws.com/us-east-1_6v9AExXS8", IDToken.c_str());
			cred_request.SetIdentityId(identityID);

			auto response = identityClient->GetCredentialsForIdentity(cred_request);

			auto temp = response.GetResultWithOwnership().GetCredentials();
			Aws::Auth::AWSCredentials creds(temp.GetAccessKeyId(), temp.GetSecretKey(), temp.GetSessionToken());
			currCreds = creds;

			username = user;
			return true;
		}
		else if (challengeName == Aws::CognitoIdentityProvider::Model::ChallengeNameType::NEW_PASSWORD_REQUIRED)
		{
			Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeRequest challengeResponse;
			challengeResponse.SetChallengeName(challengeName);
			challengeResponse.SetClientId(APP_CLIENT_ID);
			challengeResponse.SetSession(initiateAuthResult.GetSession());
			challengeResponse.AddChallengeResponses("USERNAME", username.c_str());
			challengeResponse.AddChallengeResponses("NEW_PASSWORD", pass.c_str());
			auto responseOutcome = identityProviderClient->RespondToAuthChallenge(challengeResponse);
			if (responseOutcome.IsSuccess())
			{
				cout << "response succeeded" << endl;
			}
			else
			{
				cout << "response failed" << endl;
			}
		}
	}
	else
	{
		Aws::Client::AWSError<Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors> error = initiateAuthOutcome.GetError();
		cout << "Error logging in: " << error.GetMessage() << endl << endl;
	}

	return false;
}


CustomMapClient::CustomMapClient()
{
	Aws::Utils::Logging::LogLevel logLevel{ Aws::Utils::Logging::LogLevel::Trace };
	//options.loggingOptions.logLevel = logLevel;
	AWSSDKOptions.loggingOptions.logger_create_fn = [logLevel] {return std::make_shared<Aws::Utils::Logging::ConsoleLogSystem>(logLevel); };
	Aws::InitAPI(AWSSDKOptions);
}

CustomMapClient::~CustomMapClient()
{
	Cleanup();
}

void CustomMapClient::AnonymousInit()
{
	Aws::Auth::CognitoCachingAnonymousCredentialsProvider *anonCredProvider = new Aws::Auth::CognitoCachingAnonymousCredentialsProvider(
		"942521585968", "us-east-1:e8840b78-d9e3-4c03-8d6b-a9bdd5833fbd");

	Aws::Auth::AWSCredentials anonCreds = anonCredProvider->GetAWSCredentials();

	s3Interface.InitWithCredentials(anonCreds);
	cognitoInterface.InitWithCredentials(anonCreds);

	serverConn.ConnectToServer(true);
}

void CustomMapClient::Cleanup()
{
	serverConn.CleanupServerConnection();
	Aws::ShutdownAPI(AWSSDKOptions);
}

bool CustomMapClient::AttemptDeleteMapFromServer(CustomMapEntry &entry)
{
	if (IsLoggedIn())
	{
		if (serverConn.RequestMapDeletion(entry.id, cognitoInterface.GetAccessToken()))
		{
			cout << "map " << entry.name << " by user: " << entry.creatorName << " has been removed" << endl;
			return true;
		}
		else
		{
			cout << "failed to remove map: " << entry.name << " by user: " << entry.creatorName << endl;
		}
	}

	return false;
}

bool CustomMapClient::AttemptUploadMapToServer(const std::string &path, const std::string &mapName,
	bool overwriteIfExists )
{
	if (IsLoggedIn())
	{
		if (serverConn.RequestMapUpload(mapName, cognitoInterface.GetAccessToken(), overwriteIfExists ))
		{
			CustomMapEntry entry;
			entry.name = mapName;
			string file = entry.GetMapFileName();
			s3Interface.UploadObject(path.c_str(), file.c_str(), cognitoInterface.username.c_str()); //assumed to work for now..
			return true;
		}
		else
		{
			cout << "failed upload..." << endl;
		}
	}
	else
	{
		cout << "attempt to upload while not logged in" << endl;
	}

	return false;
}

bool CustomMapClient::AttemptDownloadMapFromServer(const std::string &downloadPath, CustomMapEntry &entry)
{
	if (serverConn.RequestMapDownload(entry.id))
	{
		string key = entry.CreateKey();
		string file = entry.GetMapFileName();
		s3Interface.DownloadObject(downloadPath.c_str(), key.c_str(), file.c_str());
		return true;
	}

	return false;
}

bool CustomMapClient::AttempGetMapListFromServer()
{
	return serverConn.RequestGetMapList(mapEntries);
}

void CustomMapClient::PrintMapEntries()
{
	cout << "Listing all maps: " << endl;
	int numEntries = mapEntries.size();
	for (int i = 0; i < numEntries; ++i)
	{
		cout << i << ", map: " << mapEntries[i].name << "   by user: " << mapEntries[i].creatorName << endl;
	}
}

bool CustomMapClient::AttemptUserLogin(const std::string &user, const std::string &pass)
{
	if (!IsLoggedIn())
	{
		if (cognitoInterface.TryLogIn("test", "Shephard123~"))
		{
			s3Interface.InitWithCredentials(cognitoInterface.currCreds);
			return true;
		}
	}
	return false;
}

bool CustomMapClient::IsLoggedIn()
{
	return cognitoInterface.isLoggedIn;
}
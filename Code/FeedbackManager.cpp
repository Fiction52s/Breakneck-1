#include "FeedbackManager.h"
#include <assert.h>
#include <iostream>

#define FROM_ADDR "<KineticFeedback@kbal.com>"
#define TO_ADDR "<fiction52s@gmail.com>"
#define SERVER "smtp://smtp-relay.sendinblue.com"
#define PORT 587L
#define LOGIN "fiction52s@gmail.com"
#define PASSWORD "bATsk0YDgNxZLVfS"

using namespace std;

CURL *FeedbackManager::curl = NULL;
char FeedbackManager::payload[MAX_PAYLOAD_SIZE];

struct upload_status {
	size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;
	const char *data;
	size_t room = size * nmemb;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	data = &FeedbackManager::payload[upload_ctx->bytes_read];

	if (data) {
		size_t len = strlen(data);
		if (room < len)
			len = room;
		memcpy(ptr, data, len);
		upload_ctx->bytes_read += len;

		return len;
	}

	return 0;
}

bool FeedbackManager::Init()
{
	assert(curl == NULL);
	curl = curl_easy_init();
	return curl != NULL;
}

void FeedbackManager::Cleanup()
{
	curl_easy_cleanup(curl);
	curl = NULL;
}

void FeedbackManager::Test()
{
	Init();

	SetPayload("new test subject 2 2 2", "test body!");

	SubmitFeedback();

	Cleanup();
}

void FeedbackManager::SetPayload(const std::string &subject, const std::string &body)
{
	memset(payload, 0, 2000 * sizeof(char));
	string test =
		"To: " TO_ADDR "\r\n"
		"From: " FROM_ADDR "\r\n"
		"Subject: " + subject + "\r\n"
		"\r\n" + body + "\r\n";

	memcpy(payload, test.c_str(), test.size() + 1);
}

bool FeedbackManager::SubmitFeedback()
{
	CURLcode res = CURLE_OK;

	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx = { 0 };

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, LOGIN);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

		curl_easy_setopt(curl, CURLOPT_URL, SERVER);
		curl_easy_setopt(curl, CURLOPT_PORT, PORT);

		//curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		//curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);

		recipients = curl_slist_append(recipients, TO_ADDR);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			cout << "failureeeee " << res << "\n";
		}
		else
		{
			cout << "sent pog" << "\n";
		}

		curl_slist_free_all(recipients);

		return true;
	}
	else
	{
		cout << "failed" << "\n";

		return false;
	}
}
#include "FeedbackManager.h"
#include <assert.h>
#include <iostream>
#include <vector>

#define FROM_ADDR "<KineticFeedback@kbal.com>"
#define TO_ADDR "<fiction52s@gmail.com>"
//#define SERVER "smtp://smtp-relay.sendinblue.com"
#define SERVER "smtp://smtp-relay.brevo.com"
#define PORT 587L
#define LOGIN "fiction52s@gmail.com"
#define PASSWORD "bATsk0YDgNxZLVfS"

using namespace std;

void FeedbackManager::Test()
{
	//SubmitFeedback("Test Subject", "Test Body", "aaa.png");
}

bool FeedbackManager::SubmitFeedback(const std::string &subject, const std::string &body)
{
	return SubmitFeedback(subject, body, "");
}

bool FeedbackManager::SubmitFeedback(const std::string &subject, const std::string &body, const std::string &previewPath)
{
	CURL *curl = curl_easy_init();

	CURLcode res = CURLE_OK;

	struct curl_slist *headers = NULL;
	struct curl_slist *recipients = NULL;
	struct curl_slist *slist = NULL;
	curl_mime *mime = NULL;
	curl_mimepart *part = NULL;

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, LOGIN);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

		curl_easy_setopt(curl, CURLOPT_URL, SERVER);
		curl_easy_setopt(curl, CURLOPT_PORT, PORT);

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);

		recipients = curl_slist_append(recipients, TO_ADDR);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		std::vector<string> headerTest;
		headerTest.reserve(8);
		headerTest.push_back("To: " TO_ADDR);
		headerTest.push_back("From: " FROM_ADDR " Kinetic Feedback");
		headerTest.push_back("MIME-Version: 1.0"); //not sure if needed
		headerTest.push_back("Subject: " + subject);

		for (auto it = headerTest.begin(); it != headerTest.end(); ++it)
		{
			headers = curl_slist_append(headers, (*it).c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(mime);
		curl_mime_data(part, body.c_str(), body.size());
		//curl_mime_type(part, "text/html");
		curl_mime_type(part, "text/plain");

		//attach the image

		if (previewPath != "")
		{
			part = curl_mime_addpart(mime);
			curl_mime_name(part, "preview_image");
			curl_mime_filedata(part, previewPath.c_str());//"image.png");
			curl_mime_type(part, "image/png");
			curl_mime_encoder(part, "base64");
		}

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		else
		{
			cout << "email has successfully sent!" << endl;
		}

		//possibly messes up memory somehow
		curl_slist_free_all(recipients);
		curl_slist_free_all(headers);
		curl_mime_free(mime);

		curl_easy_cleanup(curl);
		return true;
	}
	else
	{
		cout << "curl failed" << "\n";

		return false;
	}
}
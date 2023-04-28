#ifndef __FEEDBACKMANAGER_H__
#define __FEEDBACKMANAGER_H__

#include "curl\curl.h"
#include <string>

struct FeedbackManager
{
	const static int MAX_PAYLOAD_SIZE = 2048;
	static CURL *curl;
	static char payload[MAX_PAYLOAD_SIZE];

	static void SetPayload(const std::string &subject, const std::string &body);
	static bool Init();
	static void Cleanup();
	static void Test();

	static bool SubmitFeedback();
};

#endif

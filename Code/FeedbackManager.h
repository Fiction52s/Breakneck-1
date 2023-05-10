#ifndef __FEEDBACKMANAGER_H__
#define __FEEDBACKMANAGER_H__

#include "curl\curl.h"
#include <string>

struct FeedbackManager
{
	static void Test();
	static bool SubmitFeedback(const std::string &subject, const std::string &body,
		const std::string &previewPath);
	static bool SubmitFeedback(const std::string &subject, const std::string &body);
};

#endif

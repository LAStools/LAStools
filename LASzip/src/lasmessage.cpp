#include "lasmessage.hpp"
#include <stdio.h>
#include <stdarg.h> 
#include <string>
#include <assert.h>

void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

static LASMessageHandler message_handler = &las_default_message_handler;
static void * message_user_data = 0;
static LAS_MESSAGE_TYPE  las_message_level = LAS_INFO;

void LASMessage(LAS_MESSAGE_TYPE type, LAS_FORMAT_STRING(const char*) fmt, ...)
{
	assert(type <= LAS_FATAL_ERROR);		//message type must be equal lower than LAS_FATAL_ERROR (LAS_QUIET must to be used in LASMessage calls)

	if (type < las_message_level)
		return;

	char buffer[LAS_MAX_MESSAGE_LENGTH];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(buffer, LAS_MAX_MESSAGE_LENGTH, fmt, args);
	va_end(args);

	//remove trailing line feed
	while (len > 0 && buffer[len-1] == '\n')
	{
		buffer[len-1] = '\0';
		--len;
	}

	(*message_handler)(type, buffer, message_user_data);
}

void LASLIB_DLL set_message_log_level(LAS_MESSAGE_TYPE loglevel)
{
	las_message_level = loglevel;
}

LAS_MESSAGE_TYPE LASLIB_DLL get_message_log_level() {
	return las_message_level;
}


void LASLIB_DLL set_las_message_handler(LASMessageHandler callback, void* user_data /*= 0*/)
{
	message_handler = callback;
	message_user_data = user_data;
}

void LASLIB_DLL unset_las_message_handler()
{
	message_handler = las_default_message_handler;
	message_user_data = 0;
}

void format_message(std::string& messsage, unsigned multiline_ident, bool append_trailing_lf = true)
{
	size_t lines = messsage.find('\n');
	std::string result;
	result.reserve(messsage.size() + (lines-1)*multiline_ident + 1);

	const std::string find_str = "\n\t";
	std::string replace_str((size_t)multiline_ident + 1, ' ');	
	replace_str[0] = '\n';
	size_t start_pos = 0, pos = messsage.find(find_str, start_pos);
	while (pos != std::string::npos)
	{
		result += messsage.substr(start_pos, pos - start_pos);
		result += replace_str;
		start_pos = pos + find_str.size();
		pos = messsage.find(find_str, start_pos);
	}
	result += messsage.substr(start_pos, pos - start_pos);

	if (append_trailing_lf)
		result += "\n";

	messsage = result;
}


void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data)
{
	std::string prefix;
	std::string message(msg);

	switch (type)
	{
	case LAS_DEBUG:
		//prefix = "";	//add possible prefix
		break;
	case LAS_VERY_VERBOSE:
		//prefix = "";	//add possible prefix
		break;
	case LAS_VERBOSE:
		//prefix = "";	//add possible prefix
		break;
	case LAS_INFO:
		//prefix = "";	//add possible prefix
		break;
	case LAS_WARNING:
		prefix = "WARNING: ";
		break;
	case LAS_SERIOUS_WARNING:
		prefix = "SERIOUS WARNING: ";
		break;
	case LAS_ERROR:
		prefix = "ERROR: ";
		break;
	case LAS_FATAL_ERROR:
		prefix = "FATAL ERROR: ";
		break;
	}

	if (!prefix.empty())
	{
		format_message(message, (unsigned)prefix.size());
		fprintf(stderr, prefix.c_str());
		fprintf(stderr, message.c_str());
	}
	else
	{
		fprintf(stderr, "%s\n", message.c_str());
	}
}
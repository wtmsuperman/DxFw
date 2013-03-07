#include <dx/dx_tiny_gui.h>
#include <map>
#include <string>
#include <sstream>

static GUILabel* g_label = 0;
static std::map<std::string,std::string> g_logging;
static bool g_logEnable = true;

typedef std::map<std::string,std::string>::iterator LogginIter;

void loggingInit(GUILabel* label)
{
	g_label = label;
}

void disableLog()
{
	g_logEnable = false;
}

void enableLog()
{
	g_logEnable = true;
}

void logToScreen(const char* tag,const char* msg,...)
{
	if (g_logEnable && g_label)
	{
		std::string& org = g_logging[tag];

		char buffer[512];
		va_list var;
		va_start(var,msg);
		vsprintf(buffer,msg,var);
		va_end(var);

		org = buffer;
		LogginIter end = g_logging.end();
		std::stringstream ss;
		for (LogginIter iter=g_logging.begin(); iter!=end; ++iter)
		{
			ss << iter->first;
			ss << " : ";
			ss << iter->second;
			ss << "\n";
		}

		g_label->setText(ss.str().c_str());
	}
}
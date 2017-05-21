#include <cstring>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <list>

const char ** UseListOfStringToArrayofCharArray(const std::list<std::string>& stringList) {
  unsigned list_size = stringList.size();
  char** StringArray = new char*[list_size];

  unsigned index = 0;
  for(std::string stringItem : stringList) {
	  StringArray[index] = new char[stringItem.size() + 1];
	  memcpy(StringArray[index], stringItem.c_str(), stringItem.size() + 1);
	  index++;
  }

  return (const char **) StringArray;
}


#ifdef __MINGW32__
	#if !_WIN64
			#define RVExtension _RVExtension
			#define RVExtensionArgs _RVExtensionArgs
	#endif

	extern "C" {
		void __stdcall RVExtension(char *output, int outputSize, const char *function);
		void __stdcall RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt);
	}
#else
	extern "C" {
		void RVExtension(char *output, int outputSize, const char *function);
		void RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt);
	}
#endif
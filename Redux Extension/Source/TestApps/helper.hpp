#include <cstring>
#include <string>
#include <string.h>
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

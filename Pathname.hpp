#ifndef _PATHNAME_HPP_
#define _PATHNAME_HPP_

#include <string>

std::string getExecutablePathname();

std::string stripFilename(std::string path);

std::string stripDirname(std::string path);

#endif

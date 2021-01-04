#ifndef __PATHUTILITY_H__
#define __PATHUTILITY_H__

#include <string>

std::string _GetFileName( const char* szPath );
std::string _GetDir(const std::string& fname);
std::string _GetDir(const char* fnamae);

#endif
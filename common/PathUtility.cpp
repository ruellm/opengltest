#include "PathUtility.h"

std::string _GetFileName( const char* szPath )
{
	//ZBU_UNSURE : Path directory "\\" to "/"
	std::string strPath( szPath );
	size_t pos = strPath.find_last_of("\\");
	std::string filename = strPath;
	int count = 0;
	do{
		// ZBU_TODO : make a common function for getting the filename out of a path
		//Extract file name. some model files contains non standard directories.
		if(pos != std::string::npos)
			filename.assign(strPath.begin() + pos + 1, strPath.end());
		//else
			//filename = strPath;

		 pos = strPath.find_last_of("/");

	}while( count++ < 2 );

	return (filename);
}

std::string _GetDir(const std::string& fname)
{
     size_t pos = fname.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : fname.substr(0, pos);
}

std::string _GetDir(const char* fnamae)
{
	return _GetDir(std::string(fnamae));
}

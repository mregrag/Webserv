#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <stdlib.h>
#include <sstream>


namespace Utils 
{

	template <typename T>
	std::string toString(const T& value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}	

	std::string getMimeType(const std::string& path);

	std::string getCurrentDate();

	std::string readFileContent(const std::string& filePath);
	bool isDirectory(const std::string& path);
	bool isFileExists(const std::string& path);
	bool isFileWritable(const std::string& path);
	bool isFileReadble(const std::string& path);
	bool isExecutable(const std::string& path);
	std::string getFileExtension(const std::string& path);
	std::string getExtension(const std::string& path);
	std::vector<std::string> listDirectory(const std::string& path);

	std::string trimWhitespace(const std::string& str);
	std::string trim(const std::string& str);
	std::vector<std::string> split(const std::string& str, char delimiter);

	int urlDecode(std::string& uri);
	ssize_t parseHexChunk(const std::string& hexstr);

	size_t stringToSizeT(const std::string& str);
	bool strToSizeT(const std::string& str, size_t& result, bool allowZero = false);

	std::string getMessage(int code);

	bool isValidMethodToken(const std::string& method);
	bool isValidUri(const std::string& uri);
	bool isValidHeaderKey(const std::string& key);
	bool isValidHeaderValue(const std::string& value);

	std::string createUploadFile(const std::string& prefix, const std::string& dir);
	std::string createTempFile(const std::string& prefix, const std::string& dir);
}

#endif



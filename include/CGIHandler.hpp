#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include "HTTPRequest.hpp"
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>


class CGIHandler
{
public:
	CGIHandler();
	~CGIHandler();

	void init(HTTPRequest* request);
	void start();

	bool isRunning(int& status);
	void killProcess();
	bool hasTimedOut();

	time_t getStartTime() const;
	std::string getOutputFile() const;
	pid_t getPid() const;

	void validatePaths() const;
	void buildEnv();
	void buildArgv();
	void cleanEnv();
	void cleanArgv();
	void cleanup();

private:
	pid_t           _pid;
	int             _ouFd;
	int             _inFd;

	std::string     _execPath;
	std::string     _scriptPath;
	std::string     _outputFile;
	std::string     _extension;

	char**          _envp;
	char**          _argv;

	std::map<std::string, std::string> _env;

	time_t          _startTime;

};

#endif


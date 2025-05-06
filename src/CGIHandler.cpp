#include "../include/CGIHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>

CGIHandler::CGIHandler() 
{
}

CGIHandler::~CGIHandler() 
{
}

void CGIHandler::setCgiPath(const std::string &path) 
{
	_cgiPath = path;
}

void CGIHandler::setScriptPath(const std::string &path)
{
	_scriptPath = path;
}

void CGIHandler::setBody(const std::string &body) 
{
	_body = body;
}

void CGIHandler::addEnv(const std::string &key, const std::string &value) 
{
	_env[key] = value;
}

bool CGIHandler::setEnvironment(char **&envp) 
{
	envp = new char*[_env.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it, ++i) 
	{
		std::string entry = it->first + "=" + it->second;
		envp[i] = strdup(entry.c_str());
	}
	envp[i] = NULL;
	return true;
}

bool CGIHandler::executeScript(char **envp)
{
	int stdin_pipe[2], stdout_pipe[2];

	if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1)
		return false;

	pid_t pid = fork();
	if (pid < 0)
		return false;

	if (pid == 0) 
	{
		dup2(stdin_pipe[0], 0);
		dup2(stdout_pipe[1], 1);

		close(stdin_pipe[1]);
		close(stdout_pipe[0]);

		char *args[] = { const_cast<char*>(_cgiPath.c_str()), const_cast<char*>(_scriptPath.c_str()), NULL };
		execve(_cgiPath.c_str(), args, envp);
		exit(1); // exec failed
	}

	close(stdin_pipe[0]);
	write(stdin_pipe[1], _body.c_str(), _body.size());
	close(stdin_pipe[1]);

	close(stdout_pipe[1]);
	char buffer[1024];
	ssize_t bytes;
	std::ostringstream oss;

	while ((bytes = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) 
		oss.write(buffer, bytes);

	close(stdout_pipe[0]);
	waitpid(pid, NULL, 0);

	_response = oss.str();
	return true;
}

bool CGIHandler::run() 
{
	char **envp;
	if (!setEnvironment(envp))
		return false;
	bool success = executeScript(envp);

	for (size_t i = 0; envp[i]; ++i)
		free(envp[i]);
	delete[] envp;

	return success;
}

std::string CGIHandler::getResponse() const {
	return _response;
}


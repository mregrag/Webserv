#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <vector>
#include <map>

class CGIHandler 
{
	private:
		std::string _cgiPath;
		std::string _scriptPath;
		std::string _body;
		std::map<std::string, std::string> _env;
		std::string _response;

		bool setEnvironment(char **&envp);
		bool executeScript(char **envp);

	public:
		CGIHandler();
		~CGIHandler();

		void setCgiPath(const std::string &path);
		void setScriptPath(const std::string &path);
		void setBody(const std::string &body);
		void addEnv(const std::string &key, const std::string &value);

		bool run();
		std::string getResponse() const;
};

#endif


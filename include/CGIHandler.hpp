#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "../include/Utils.hpp"
#include "../include/Logger.hpp"
#include "../include/HTTPRequest.hpp"

class    CGI
{
    int    _pid;
    int    _requestBodyFileFd;
    int    _tmpFileFd;

    HTTPRequest    &_request;

    // execve args
    char    **_args;
    char    **_env;

    void    _initTmpFile(void);
    void    _initArgs(void);
    void    _initEnv(void);

public:
    CGI(HTTPRequest &reuqest);
    ~CGI(void);

    void    init(void);
    void    execute(void);

    friend std::ostream &operator<<(std::ostream &os, const CGI &cgi);
};

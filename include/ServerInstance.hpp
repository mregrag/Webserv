#ifndef SERVERINSTANCE_HPP
#define SERVERINSTANCE_HPP

class ServerInstance
{
public:
    ServerInstance(const ServerConfig& config);
    ~ServerInstance();
    ServerInstance();
    void    setConfig(const ServerConfig& config);

    int getListenFd() const;
    const ServerConfig& getConfig() const;

private:
    void setupSocket();  // Binds and listens on the configured port
    ServerConfig _config;
    int _listenFd; // Socket for incoming connections
};


#endif
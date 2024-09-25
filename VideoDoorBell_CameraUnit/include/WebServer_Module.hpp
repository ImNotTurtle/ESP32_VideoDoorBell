#ifndef WEBSERVER_MODULE_HPP
#define WEBSERVER_MODULE_HPP

#include <ESPAsyncWebServer.h>

#define WEB_SERVER_DEFAULT_PORT 80

class WebServer_Module {
public:
    static void Init();
    static void Begin();
    static void Handle();


private:
    
};

#endif

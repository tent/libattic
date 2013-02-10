#ifndef APPUTILS
#define APPUTILS
#pragma once

#include <vector>
#include <string>

#include "tentapp.h"
#include "errorcodes.h"

namespace app
{

    static int StartupAppInstance( TentApp& pApp,
                                   const std::string& appName,
                                   const std::string& appDescription,
                                   const std::string& url,
                                   const std::string& icon,
                                   std::vector<std::string>& uris,
                                   std::vector<std::string>& scopes)
    {
        int status = ret::A_OK;

        pApp.SetAppName(appName);
        pApp.SetAppDescription(appDescription);
        pApp.SetAppURL(url);
        pApp.SetAppIcon(icon);

        pApp.SetRedirectUris(uris);
        pApp.SetScopes(scopes);

        return status;
    }


}

#endif


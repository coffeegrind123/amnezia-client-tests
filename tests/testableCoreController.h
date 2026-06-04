#ifndef TESTABLECORECONTROLLER_H
#define TESTABLECORECONTROLLER_H

#include "core/controllers/coreController.h"

// Test-only subclass that re-exports CoreController's protected getters as
// public accessors. Upstream PR #2550 ("move tests to separate repo") replaced
// the per-test `friend class` declarations with `protected:` getters precisely
// so out-of-tree tests can reach the controllers/models/repositories by
// subclassing instead of being named as friends. This keeps the client repo
// free of any test-access plumbing.
//
// No Q_OBJECT here on purpose: this type adds no signals/slots of its own and
// reuses CoreController's meta-object. Constructors are inherited verbatim.
class TestableCoreController : public CoreController
{
public:
    using CoreController::CoreController;

    SecureServersRepository *serversRepository() const { return serversRepositoryProtected(); }
    SecureAppSettingsRepository *appSettingsRepository() const { return appSettingsRepositoryProtected(); }

    ServersModel *serversModel() const { return serversModelProtected(); }
    ContainersModel *containersModel() const { return containersModelProtected(); }
    ApiServicesModel *apiServicesModel() const { return apiServicesModelProtected(); }
    NewsModel *newsModel() const { return newsModelProtected(); }
    AllowedDnsModel *allowedDnsModel() const { return allowedDnsModelProtected(); }
    AppSplitTunnelingModel *appSplitTunnelingModel() const { return appSplitTunnelingModelProtected(); }
    IpSplitTunnelingModel *ipSplitTunnelingModel() const { return ipSplitTunnelingModelProtected(); }
    LanguageModel *languageModel() const { return languageModelProtected(); }

    InstallUiController *installUiController() const { return installUiControllerProtected(); }
    ImportController *importCoreController() const { return importCoreControllerProtected(); }
    ExportController *exportController() const { return exportControllerProtected(); }
    InstallController *installController() const { return installControllerProtected(); }
    ServersController *serversController() const { return serversControllerProtected(); }
    SettingsUiController *settingsUiController() const { return settingsUiControllerProtected(); }
    SettingsController *settingsController() const { return settingsControllerProtected(); }
    AllowedDnsUiController *allowedDnsUiController() const { return allowedDnsUiControllerProtected(); }
    AllowedDnsController *allowedDnsController() const { return allowedDnsControllerProtected(); }
    LanguageUiController *languageUiController() const { return languageUiControllerProtected(); }
    IpSplitTunnelingController *ipSplitTunnelingController() const { return ipSplitTunnelingControllerProtected(); }
    IpSplitTunnelingUiController *ipSplitTunnelingUiController() const { return ipSplitTunnelingUiControllerProtected(); }
    AppSplitTunnelingController *appSplitTunnelingController() const { return appSplitTunnelingControllerProtected(); }
    AppSplitTunnelingUiController *appSplitTunnelingUiController() const { return appSplitTunnelingUiControllerProtected(); }
    ServersUiController *serversUiController() const { return serversUiControllerProtected(); }
    ServicesCatalogUiController *servicesCatalogUiController() const { return servicesCatalogUiControllerProtected(); }
    ApiNewsUiController *apiNewsUiController() const { return apiNewsUiControllerProtected(); }
};

#endif // TESTABLECORECONTROLLER_H

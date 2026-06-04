#include <QTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QSignalSpy>
#include <QLocale>

#include "core/controllers/coreController.h"
#include "tests/testableCoreController.h"
#include "ui/controllers/settingsUiController.h"
#include "ui/controllers/languageUiController.h"
#include "ui/models/allowedDnsModel.h"
#include "ui/models/ipSplitTunnelingModel.h"
#include "ui/models/appSplitTunnelingModel.h"
#include "ui/models/languageModel.h"
#include "vpnConnection.h"
#include "secureQSettings.h"

class TestSettingsSignals : public QObject
{
    Q_OBJECT

private:
    TestableCoreController* m_coreController;
    SecureQSettings* m_settings;

private slots:
    void initTestCase() {
        QString testOrg = "AmneziaVPN-Test-" + QUuid::createUuid().toString();
        m_settings = new SecureQSettings(testOrg, "amnezia-client", nullptr, false);
        
        auto vpnConnection = QSharedPointer<VpnConnection>::create(nullptr, nullptr);
        m_coreController = new TestableCoreController(vpnConnection, m_settings, nullptr, this);
    }

    void cleanupTestCase() {
        m_settings->clearSettings();
        delete m_coreController;
        delete m_settings;
    }

    void init() {
        m_settings->clearSettings();
    }

    void testDnsSettingsSignals() {
        QSignalSpy primaryDnsChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::primaryDnsChanged);
        QSignalSpy secondaryDnsChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::secondaryDnsChanged);
        QSignalSpy allowedDnsServersChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::allowedDnsServersChanged);

        QString primaryDns = "8.8.8.8";
        QString secondaryDns = "8.8.4.4";

        m_coreController->settingsUiController()->setPrimaryDns(primaryDns);
        QVERIFY2(primaryDnsChangedSpy.count() == 1, "primaryDnsChanged signal should be emitted");
        QVERIFY2(m_coreController->settingsController()->getPrimaryDns() == primaryDns, "Primary DNS should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->getPrimaryDns() == primaryDns, "Primary DNS should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->primaryDns() == primaryDns, "Primary DNS should be available in SecureAppSettingsRepository");

        m_coreController->settingsUiController()->setSecondaryDns(secondaryDns);
        QVERIFY2(secondaryDnsChangedSpy.count() == 1, "secondaryDnsChanged signal should be emitted");
        QVERIFY2(m_coreController->settingsController()->getSecondaryDns() == secondaryDns, "Secondary DNS should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->getSecondaryDns() == secondaryDns, "Secondary DNS should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->secondaryDns() == secondaryDns, "Secondary DNS should be available in SecureAppSettingsRepository");

        QStringList dnsList = {"1.1.1.1", "1.0.0.1"};
        m_coreController->allowedDnsController()->addDnsList(dnsList, true);
        QVERIFY2(allowedDnsServersChangedSpy.count() == 1, "allowedDnsServersChanged signal should be emitted");
        QVERIFY2(m_coreController->appSettingsRepository()->getAllowedDnsServers() == dnsList, "Allowed DNS servers should be updated in SecureAppSettingsRepository");
        QVERIFY2(m_coreController->allowedDnsController()->getCurrentDnsServers() == dnsList, "Allowed DNS servers should be available in AllowedDnsController");
        
        QVERIFY2(m_coreController->allowedDnsUiController() != nullptr, "AllowedDnsUiController should exist");
        QVERIFY2(m_coreController->allowedDnsModel() != nullptr, "AllowedDnsModel should exist");
        
        QStringList modelDnsList;
        for (int i = 0; i < m_coreController->allowedDnsModel()->rowCount(); ++i) {
            modelDnsList.append(m_coreController->allowedDnsModel()->data(m_coreController->allowedDnsModel()->index(i, 0), AllowedDnsModel::IpRole).toString());
        }
        QVERIFY2(modelDnsList == dnsList, "Allowed DNS servers should be available in AllowedDnsModel");
    }

    void testAmneziaDnsToggleSignal() {
        QSignalSpy amneziaDnsToggledSpy(m_coreController->settingsUiController(), &SettingsUiController::amneziaDnsToggled);
        QSignalSpy useAmneziaDnsChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::useAmneziaDnsChanged);

        bool initialValue = m_coreController->settingsController()->isAmneziaDnsEnabled();
        
        m_coreController->settingsUiController()->toggleAmneziaDns(!initialValue);
        QVERIFY2(amneziaDnsToggledSpy.count() == 1, "amneziaDnsToggled signal should be emitted");
        QVERIFY2(amneziaDnsToggledSpy.at(0).at(0).toBool() == !initialValue, "amneziaDnsToggled should emit correct value");
        QVERIFY2(useAmneziaDnsChangedSpy.count() == 1, "useAmneziaDnsChanged signal should be emitted");
        QVERIFY2(m_coreController->settingsController()->isAmneziaDnsEnabled() == !initialValue, "Amnezia DNS state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isAmneziaDnsEnabled() == !initialValue, "Amnezia DNS state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->useAmneziaDns() == !initialValue, "Amnezia DNS state should be available in SecureAppSettingsRepository");

        m_coreController->settingsUiController()->toggleAmneziaDns(initialValue);
        QVERIFY2(amneziaDnsToggledSpy.count() == 2, "amneziaDnsToggled signal should be emitted again");
        QVERIFY2(useAmneziaDnsChangedSpy.count() == 2, "useAmneziaDnsChanged signal should be emitted again");
        QVERIFY2(m_coreController->settingsUiController()->isAmneziaDnsEnabled() == initialValue, "Amnezia DNS state should be restored in SettingsUiController");
    }

    void testLoggingSignals() {
        QSignalSpy loggingStateChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::loggingStateChanged);
        QSignalSpy saveLogsChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::saveLogsChanged);

        bool initialLogging = m_coreController->settingsController()->isLoggingEnabled();

        m_coreController->settingsUiController()->toggleLogging(!initialLogging);
        QVERIFY2(loggingStateChangedSpy.count() == 1, "loggingStateChanged signal should be emitted");
        QVERIFY2(saveLogsChangedSpy.count() == 1, "saveLogsChanged signal should be emitted");
        QVERIFY2(m_coreController->settingsController()->isLoggingEnabled() == !initialLogging, "Logging state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isLoggingEnabled() == !initialLogging, "Logging state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->isSaveLogs() == !initialLogging, "Logging state should be available in SecureAppSettingsRepository");

        m_coreController->settingsUiController()->toggleLogging(initialLogging);
        QVERIFY2(loggingStateChangedSpy.count() == 2, "loggingStateChanged signal should be emitted again");
        QVERIFY2(saveLogsChangedSpy.count() == 2, "saveLogsChanged signal should be emitted again");
        QVERIFY2(m_coreController->settingsUiController()->isLoggingEnabled() == initialLogging, "Logging state should be restored in SettingsUiController");
    }

    void testScreenshotsSignals() {
        QSignalSpy screenshotsEnabledChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::screenshotsEnabledChanged);

        bool initialScreenshots = m_coreController->settingsController()->isScreenshotsEnabled();

        m_coreController->settingsUiController()->toggleScreenshotsEnabled(!initialScreenshots);
        QVERIFY2(screenshotsEnabledChangedSpy.count() == 1, "screenshotsEnabledChanged signal should be emitted");
        QVERIFY2(screenshotsEnabledChangedSpy.at(0).at(0).toBool() == !initialScreenshots, "screenshotsEnabledChanged should emit correct value");
        QVERIFY2(m_coreController->settingsController()->isScreenshotsEnabled() == !initialScreenshots, "Screenshots state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isScreenshotsEnabled() == !initialScreenshots, "Screenshots state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->isScreenshotsEnabled() == !initialScreenshots, "Screenshots state should be available in SecureAppSettingsRepository");
    }

    void testStartMinimizedSignals() {
        QSignalSpy startMinimizedChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::startMinimizedChanged);

        m_coreController->settingsUiController()->toggleAutoStart(true);

        bool initialStartMinimized = m_coreController->settingsController()->isStartMinimizedEnabled();

        m_coreController->settingsUiController()->toggleStartMinimized(!initialStartMinimized);
        QVERIFY2(startMinimizedChangedSpy.count() == 1, "startMinimizedChanged signal should be emitted");
        QVERIFY2(m_coreController->settingsController()->isStartMinimizedEnabled() == !initialStartMinimized, "Start minimized state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isStartMinimizedEnabled() == !initialStartMinimized, "Start minimized state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->isStartMinimized() == !initialStartMinimized, "Start minimized state should be available in SecureAppSettingsRepository");

        m_coreController->settingsUiController()->toggleAutoStart(false);
    }

    void testAutoStartDisablesStartMinimizedUi() {
        QSignalSpy startMinimizedChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::startMinimizedChanged);

        m_coreController->settingsUiController()->toggleAutoStart(true);
        m_coreController->settingsUiController()->toggleStartMinimized(true);
        QVERIFY2(m_coreController->settingsUiController()->isStartMinimizedEnabled(), "Start minimized should be enabled with autostart");

        m_coreController->settingsUiController()->toggleAutoStart(false);
        QVERIFY2(startMinimizedChangedSpy.count() >= 1, "startMinimizedChanged signal should be emitted when autostart is disabled");
        QVERIFY2(!m_coreController->settingsUiController()->isStartMinimizedEnabled(), "Start minimized should be disabled when autostart is disabled");
        QVERIFY2(!m_coreController->appSettingsRepository()->isStartMinimized(), "Start minimized setting should be cleared when autostart is disabled");
    }

    void testAutoConnectSignals() {
        bool initialAutoConnect = m_coreController->settingsController()->isAutoConnectEnabled();

        m_coreController->settingsUiController()->toggleAutoConnect(!initialAutoConnect);
        QVERIFY2(m_coreController->settingsController()->isAutoConnectEnabled() == !initialAutoConnect, "Auto connect state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isAutoConnectEnabled() == !initialAutoConnect, "Auto connect state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->isAutoConnect() == !initialAutoConnect, "Auto connect state should be available in SecureAppSettingsRepository");

        m_coreController->settingsUiController()->toggleAutoConnect(initialAutoConnect);
        QVERIFY2(m_coreController->settingsController()->isAutoConnectEnabled() == initialAutoConnect, "Auto connect state should be restored in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isAutoConnectEnabled() == initialAutoConnect, "Auto connect state should be restored in SettingsUiController");
    }

    void testLanguageChangeSignals() {
        QSignalSpy appLanguageChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::appLanguageChanged);
        QSignalSpy translationsUpdatedSpy(m_coreController->languageUiController(), &LanguageUiController::translationsUpdated);

        QLocale initialLocale = m_coreController->settingsController()->getAppLanguage();
        QLocale newLocale = (initialLocale.language() == QLocale::English) ? QLocale::Russian : QLocale::English;

        m_coreController->settingsController()->setAppLanguage(newLocale);
        QVERIFY2(appLanguageChangedSpy.count() == 1, "appLanguageChanged signal should be emitted");
        QVERIFY2(appLanguageChangedSpy.at(0).at(0).value<QLocale>() == newLocale, "appLanguageChanged should emit correct locale");
        QVERIFY2(m_coreController->settingsController()->getAppLanguage() == newLocale, "App language should be updated in SettingsController");
        QVERIFY2(m_coreController->appSettingsRepository()->getAppLanguage() == newLocale, "App language should be available in SecureAppSettingsRepository");
        
        if (m_coreController->languageModel()) {
            QString newLanguageName = m_coreController->languageUiController()->getCurrentLanguageName();
            QVERIFY2(!newLanguageName.isEmpty(), "Language name should be available in LanguageUiController");
        }
    }

    void testGatewayEndpointSignals() {
        QSignalSpy gatewayEndpointChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::gatewayEndpointChanged);
        QSignalSpy devGatewayEnvChangedSpy(m_coreController->settingsUiController(), &SettingsUiController::devGatewayEnvChanged);

        QString initialEndpoint = m_coreController->settingsController()->getGatewayEndpoint();
        QString newEndpoint = "https://test-gateway.example.com";

        m_coreController->settingsUiController()->setGatewayEndpoint(newEndpoint);
        QVERIFY2(gatewayEndpointChangedSpy.count() == 1, "gatewayEndpointChanged signal should be emitted");
        QVERIFY2(gatewayEndpointChangedSpy.at(0).at(0).toString() == newEndpoint, "gatewayEndpointChanged should emit correct endpoint");
        QVERIFY2(m_coreController->settingsController()->getGatewayEndpoint() == newEndpoint, "Gateway endpoint should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->getGatewayEndpoint() == newEndpoint, "Gateway endpoint should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->getGatewayEndpoint() == newEndpoint, "Gateway endpoint should be available in SecureAppSettingsRepository");

        bool initialDevEnv = m_coreController->settingsController()->isDevGatewayEnv();
        m_coreController->settingsUiController()->toggleDevGatewayEnv(!initialDevEnv);
        QVERIFY2(devGatewayEnvChangedSpy.count() == 1, "devGatewayEnvChanged signal should be emitted");
        QVERIFY2(devGatewayEnvChangedSpy.at(0).at(0).toBool() == !initialDevEnv, "devGatewayEnvChanged should emit correct value");
        QVERIFY2(m_coreController->settingsController()->isDevGatewayEnv() == !initialDevEnv, "Dev gateway env state should be updated in SettingsController");
        QVERIFY2(m_coreController->settingsUiController()->isDevGatewayEnv() == !initialDevEnv, "Dev gateway env state should be available in SettingsUiController");
        QVERIFY2(m_coreController->appSettingsRepository()->isDevGatewayEnv() == !initialDevEnv, "Dev gateway env state should be available in SecureAppSettingsRepository");
    }

    void testSettingsClearedSignal() {
        QSignalSpy settingsClearedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::settingsCleared);

        m_coreController->settingsController()->clearSettings();
        QVERIFY2(settingsClearedSpy.count() == 1, "settingsCleared signal should be emitted");
    }

    void testSplitTunnelingSignals() {
        QSignalSpy siteSplitTunnelingToggledSpy(m_coreController->settingsController(), &SettingsController::siteSplitTunnelingToggled);
        QSignalSpy appSplitTunnelingToggledSpy(m_coreController->settingsController(), &SettingsController::appSplitTunnelingToggled);
        QSignalSpy sitesSplitTunnelingEnabledChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::sitesSplitTunnelingEnabledChanged);
        QSignalSpy appsSplitTunnelingEnabledChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::appsSplitTunnelingEnabledChanged);
        QSignalSpy routeModeChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::routeModeChanged);
        QSignalSpy appsRouteModeChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::appsRouteModeChanged);
        QSignalSpy sitesChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::sitesChanged);
        QSignalSpy appsChangedSpy(m_coreController->appSettingsRepository(), &SecureAppSettingsRepository::appsChanged);

        bool initialSitesSplitTunneling = m_coreController->ipSplitTunnelingController()->isSplitTunnelingEnabled();
        m_coreController->ipSplitTunnelingController()->toggleSplitTunneling(!initialSitesSplitTunneling);
        QVERIFY2(sitesSplitTunnelingEnabledChangedSpy.count() == 1, "sitesSplitTunnelingEnabledChanged signal should be emitted");
        QVERIFY2(m_coreController->ipSplitTunnelingController()->isSplitTunnelingEnabled() == !initialSitesSplitTunneling, "Sites split tunneling should be updated in IpSplitTunnelingController");
        QVERIFY2(m_coreController->appSettingsRepository()->isSitesSplitTunnelingEnabled() == !initialSitesSplitTunneling, "Sites split tunneling should be available in SecureAppSettingsRepository");

        bool initialAppsSplitTunneling = m_coreController->appSplitTunnelingController()->isSplitTunnelingEnabled();
        m_coreController->appSplitTunnelingController()->toggleSplitTunneling(!initialAppsSplitTunneling);
        QVERIFY2(appsSplitTunnelingEnabledChangedSpy.count() == 1, "appsSplitTunnelingEnabledChanged signal should be emitted");
        QVERIFY2(m_coreController->appSplitTunnelingController()->isSplitTunnelingEnabled() == !initialAppsSplitTunneling, "Apps split tunneling should be updated in AppSplitTunnelingController");
        QVERIFY2(m_coreController->appSettingsRepository()->isAppsSplitTunnelingEnabled() == !initialAppsSplitTunneling, "Apps split tunneling should be available in SecureAppSettingsRepository");

        RouteMode initialRouteMode = m_coreController->ipSplitTunnelingController()->getRouteMode();
        RouteMode newRouteMode = (initialRouteMode == RouteMode::VpnOnlyForwardSites) 
                                 ? RouteMode::VpnAllExceptSites 
                                 : RouteMode::VpnOnlyForwardSites;
        m_coreController->ipSplitTunnelingController()->setRouteMode(newRouteMode);
        QVERIFY2(routeModeChangedSpy.count() == 1, "routeModeChanged signal should be emitted");
        QVERIFY2(m_coreController->ipSplitTunnelingController()->getRouteMode() == newRouteMode, "Route mode should be updated in IpSplitTunnelingController");
        QVERIFY2(m_coreController->appSettingsRepository()->routeMode() == newRouteMode, "Route mode should be available in SecureAppSettingsRepository");

        AppsRouteMode initialAppsRouteMode = m_coreController->appSplitTunnelingController()->getRouteMode();
        AppsRouteMode newAppsRouteMode = (initialAppsRouteMode == AppsRouteMode::VpnAllExceptApps)
                                         ? AppsRouteMode::VpnAllApps
                                         : AppsRouteMode::VpnAllExceptApps;
        m_coreController->appSplitTunnelingController()->setRouteMode(newAppsRouteMode);
        QVERIFY2(appsRouteModeChangedSpy.count() == 1, "appsRouteModeChanged signal should be emitted");
        QVERIFY2(m_coreController->appSplitTunnelingController()->getRouteMode() == newAppsRouteMode, "Apps route mode should be updated in AppSplitTunnelingController");
        QVERIFY2(m_coreController->appSettingsRepository()->appsRouteMode() == newAppsRouteMode, "Apps route mode should be available in SecureAppSettingsRepository");

        QMap<QString, QString> sitesMap{{"example.com", "1.2.3.4"}};
        m_coreController->ipSplitTunnelingController()->addSites(sitesMap, true);
        QVERIFY2(sitesChangedSpy.count() >= 1, "sitesChanged signal should be emitted");
        QVector<QPair<QString, QString>> currentSites = m_coreController->ipSplitTunnelingController()->getCurrentSites();
        QVERIFY2(currentSites.size() >= 1, "Sites should be available in IpSplitTunnelingController");
        
        QVERIFY2(m_coreController->ipSplitTunnelingUiController() != nullptr, "IpSplitTunnelingUiController should exist");
        QVERIFY2(m_coreController->ipSplitTunnelingModel() != nullptr, "IpSplitTunnelingModel should exist");
        
        m_coreController->ipSplitTunnelingUiController()->updateModel();
        QVERIFY2(m_coreController->ipSplitTunnelingModel()->rowCount() >= 1, "Sites should be available in IpSplitTunnelingModel");
        QString modelUrl = m_coreController->ipSplitTunnelingModel()->data(m_coreController->ipSplitTunnelingModel()->index(0, 0), IpSplitTunnelingModel::UrlRole).toString();
        QVERIFY2(modelUrl == "example.com", "Site URL should be available in IpSplitTunnelingModel");
    }
};

QTEST_MAIN(TestSettingsSignals)
#include "testSettingsSignals.moc"


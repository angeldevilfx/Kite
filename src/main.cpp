#include "browser/app_settings.h"
#include "browser/browser_window.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    qunsetenv("QT_WEBENGINE_DISABLE_VULKAN");

    QApplication app(argc, argv);
    app.setApplicationName("Kite Browser");
    app.setOrganizationName("Kite");
    applyBrowserTheme(app, loadBrowserSettings().themeMode);

    BrowserWindow window;
    window.show();

    return app.exec();
}

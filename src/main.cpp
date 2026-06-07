#include "browser/browser_window.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    qunsetenv("QT_WEBENGINE_DISABLE_VULKAN");

    QApplication app(argc, argv);
    app.setApplicationName("Qt URL Browser");
    app.setOrganizationName("Codex");

    BrowserWindow window;
    window.show();

    return app.exec();
}

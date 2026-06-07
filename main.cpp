#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLineEdit>
#include <QMainWindow>
#include <QStyle>
#include <QShortcut>
#include <QToolBar>
#include <QUrl>
#include <QWebEngineHistory>
#include <QWebEngineView>

namespace {
QUrl normalizeUrl(const QString &text) {
    QUrl url = QUrl::fromUserInput(text.trimmed());
    if (!url.isValid() || url.isEmpty()) {
        return QUrl("https://example.com");
    }
    return url;
}
} // namespace

int main(int argc, char *argv[]) {
    qunsetenv("QT_WEBENGINE_DISABLE_VULKAN");

    QApplication app(argc, argv);
    app.setApplicationName("Qt URL Browser");
    app.setOrganizationName("Codex");

    QMainWindow window;
    window.resize(1280, 800);

    auto *webView = new QWebEngineView(&window);
    auto *toolbar = new QToolBar("Navigation", &window);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    window.addToolBar(toolbar);

    auto iconFor = [&](QStyle::StandardPixmap pixmap) {
        return window.style()->standardIcon(pixmap);
    };

    auto *backAction = toolbar->addAction(iconFor(QStyle::SP_ArrowBack), "Back");
    backAction->setToolTip("Back");
    backAction->setStatusTip("Back");

    auto *forwardAction = toolbar->addAction(iconFor(QStyle::SP_ArrowForward), "Forward");
    forwardAction->setToolTip("Forward");
    forwardAction->setStatusTip("Forward");

    auto *reloadAction = toolbar->addAction(iconFor(QStyle::SP_BrowserReload), "Reload");
    reloadAction->setToolTip("Reload");
    reloadAction->setStatusTip("Reload");
    toolbar->addSeparator();

    auto *urlBar = new QLineEdit(&window);
    urlBar->setPlaceholderText("Enter a URL and press Enter");
    urlBar->setClearButtonEnabled(true);
    urlBar->setMinimumWidth(400);
    toolbar->addWidget(urlBar);

    auto *goAction = toolbar->addAction(iconFor(QStyle::SP_ArrowRight), "Go");
    goAction->setToolTip("Go");
    goAction->setStatusTip("Go");

    window.setCentralWidget(webView);

    auto loadFromUrlBar = [&]() {
        const QUrl url = normalizeUrl(urlBar->text());
        webView->load(url);
    };

    QObject::connect(urlBar, &QLineEdit::returnPressed, &window, loadFromUrlBar);
    QObject::connect(goAction, &QAction::triggered, &window, loadFromUrlBar);
    QObject::connect(backAction, &QAction::triggered, webView, &QWebEngineView::back);
    QObject::connect(forwardAction, &QAction::triggered, webView, &QWebEngineView::forward);
    QObject::connect(reloadAction, &QAction::triggered, webView, &QWebEngineView::reload);
    auto updateNavigationState = [&]() {
        backAction->setEnabled(webView->history()->canGoBack());
        forwardAction->setEnabled(webView->history()->canGoForward());
    };
    QObject::connect(webView, &QWebEngineView::urlChanged, &window, [&](const QUrl &url) {
        urlBar->setText(url.toString());
        updateNavigationState();
    });
    QObject::connect(webView, &QWebEngineView::titleChanged, &window, [&](const QString &title) {
        window.setWindowTitle(title.isEmpty() ? "Qt URL Browser" : title + " - Qt URL Browser");
    });

    QObject::connect(webView, &QWebEngineView::loadStarted, &window, [&]() {
        reloadAction->setToolTip("Loading...");
        reloadAction->setStatusTip("Loading...");
    });
    QObject::connect(webView, &QWebEngineView::loadFinished, &window, [&](bool ok) {
        reloadAction->setToolTip(ok ? "Reload" : "Retry");
        reloadAction->setStatusTip(ok ? "Reload" : "Retry");
        updateNavigationState();
    });

    backAction->setEnabled(false);
    forwardAction->setEnabled(false);
    updateNavigationState();

    auto *focusUrlShortcut = new QShortcut(QKeySequence::Open, &window);
    QObject::connect(focusUrlShortcut, &QShortcut::activated, &window, [&]() {
        urlBar->setFocus();
        urlBar->selectAll();
    });

    webView->load(QUrl("https://example.com"));
    window.show();

    return app.exec();
}

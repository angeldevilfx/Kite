#include "browser_window.h"

#include "browser_tab.h"

#include <QAction>
#include <QLineEdit>
#include <QShortcut>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QUrl>
#include <QWebEngineHistory>
#include <QWebEngineView>

namespace {
constexpr const char *kDefaultHome = "https://example.com";
}

QUrl BrowserWindow::normalizeUrl(const QString &text) {
    QUrl url = QUrl::fromUserInput(text.trimmed());
    if (!url.isValid() || url.isEmpty()) {
        return QUrl(kDefaultHome);
    }
    return url;
}

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent),
      m_backAction(nullptr),
      m_forwardAction(nullptr),
      m_reloadAction(nullptr),
      m_newTabAction(nullptr),
      m_closeTabAction(nullptr),
      m_goAction(nullptr),
      m_urlBar(nullptr),
      m_tabs(nullptr),
      m_toolbar(nullptr),
      m_homeUrl(kDefaultHome) {
    resize(1280, 800);

    auto iconFor = [&](QStyle::StandardPixmap pixmap) {
        return style()->standardIcon(pixmap);
    };

    m_toolbar = new QToolBar("Navigation", this);
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    addToolBar(m_toolbar);

    m_backAction = m_toolbar->addAction(iconFor(QStyle::SP_ArrowBack), "Back");
    m_backAction->setToolTip("Back");
    m_backAction->setStatusTip("Back");

    m_forwardAction = m_toolbar->addAction(iconFor(QStyle::SP_ArrowForward), "Forward");
    m_forwardAction->setToolTip("Forward");
    m_forwardAction->setStatusTip("Forward");

    m_reloadAction = m_toolbar->addAction(iconFor(QStyle::SP_BrowserReload), "Reload");
    m_reloadAction->setToolTip("Reload");
    m_reloadAction->setStatusTip("Reload");

    m_newTabAction = m_toolbar->addAction(iconFor(QStyle::SP_FileDialogNewFolder), "New Tab");
    m_newTabAction->setToolTip("New Tab");
    m_newTabAction->setStatusTip("New Tab");

    m_closeTabAction = m_toolbar->addAction(iconFor(QStyle::SP_DialogCloseButton), "Close Tab");
    m_closeTabAction->setToolTip("Close Tab");
    m_closeTabAction->setStatusTip("Close Tab");

    m_toolbar->addSeparator();

    m_urlBar = new QLineEdit(this);
    m_urlBar->setPlaceholderText("Enter a URL and press Enter");
    m_urlBar->setClearButtonEnabled(true);
    m_urlBar->setMinimumWidth(400);
    m_toolbar->addWidget(m_urlBar);

    m_goAction = m_toolbar->addAction(iconFor(QStyle::SP_ArrowRight), "Go");
    m_goAction->setToolTip("Go");
    m_goAction->setStatusTip("Go");

    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    setCentralWidget(m_tabs);

    connect(m_urlBar, &QLineEdit::returnPressed, this, [this]() {
        const QUrl url = normalizeUrl(m_urlBar->text());
        if (BrowserTab *tab = currentTab()) {
            tab->load(url);
        }
    });

    connect(m_goAction, &QAction::triggered, this, [this]() {
        const QUrl url = normalizeUrl(m_urlBar->text());
        if (BrowserTab *tab = currentTab()) {
            tab->load(url);
        }
    });

    connect(m_backAction, &QAction::triggered, this, [this]() {
        if (BrowserTab *tab = currentTab()) {
            tab->webView()->back();
        }
    });

    connect(m_forwardAction, &QAction::triggered, this, [this]() {
        if (BrowserTab *tab = currentTab()) {
            tab->webView()->forward();
        }
    });

    connect(m_reloadAction, &QAction::triggered, this, [this]() {
        if (BrowserTab *tab = currentTab()) {
            tab->webView()->reload();
        }
    });

    connect(m_newTabAction, &QAction::triggered, this, [this]() {
        createTab(m_homeUrl);
    });

    connect(m_closeTabAction, &QAction::triggered, this, [this]() {
        const int index = m_tabs->currentIndex();
        if (index < 0) {
            return;
        }

        if (m_tabs->count() == 1) {
            if (BrowserTab *tab = currentTab()) {
                tab->load(m_homeUrl);
            }
            return;
        }

        QWidget *widget = m_tabs->widget(index);
        m_tabs->removeTab(index);
        widget->deleteLater();
        updateNavigationState();
        updateLocationBar();
        updateWindowTitle();
    });

    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int) {
        updateNavigationState();
        updateLocationBar();
        updateWindowTitle();
    });

    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        if (index < 0) {
            return;
        }

        if (m_tabs->count() == 1) {
            if (BrowserTab *tab = currentTab()) {
                tab->load(m_homeUrl);
            }
            return;
        }

        QWidget *widget = m_tabs->widget(index);
        m_tabs->removeTab(index);
        widget->deleteLater();
        updateNavigationState();
        updateLocationBar();
        updateWindowTitle();
    });

    auto *focusUrlShortcut = new QShortcut(QKeySequence::Open, this);
    connect(focusUrlShortcut, &QShortcut::activated, this, [this]() {
        m_urlBar->setFocus();
        m_urlBar->selectAll();
    });

    auto *newTabShortcut = new QShortcut(QKeySequence::AddTab, this);
    connect(newTabShortcut, &QShortcut::activated, this, [this]() {
        createTab(m_homeUrl);
    });

    auto *closeTabShortcut = new QShortcut(QKeySequence::Close, this);
    connect(closeTabShortcut, &QShortcut::activated, this, [this]() {
        m_closeTabAction->trigger();
    });

    createTab(m_homeUrl);
}

BrowserTab *BrowserWindow::currentTab() const {
    return qobject_cast<BrowserTab *>(m_tabs->currentWidget());
}

BrowserTab *BrowserWindow::createTab(const QUrl &url) {
    auto *tab = new BrowserTab;

    connect(tab, &BrowserTab::titleChanged, this, [this, tab](const QString &title) {
        const int index = m_tabs->indexOf(tab);
        if (index >= 0) {
            m_tabs->setTabText(index, title.isEmpty() ? "New Tab" : title);
        }
        if (tab == currentTab()) {
            updateWindowTitle();
        }
    });

    connect(tab, &BrowserTab::urlChanged, this, [this, tab](const QUrl &newUrl) {
        if (tab == currentTab()) {
            m_urlBar->setText(newUrl.toString());
        }
        updateNavigationState();
    });

    connect(tab, &BrowserTab::loadStarted, this, [this, tab]() {
        if (tab == currentTab()) {
            m_reloadAction->setToolTip("Loading...");
            m_reloadAction->setStatusTip("Loading...");
        }
    });

    connect(tab, &BrowserTab::loadFinished, this, [this, tab](bool ok) {
        if (tab == currentTab()) {
            m_reloadAction->setToolTip(ok ? "Reload" : "Retry");
            m_reloadAction->setStatusTip(ok ? "Reload" : "Retry");
        }
        updateNavigationState();
    });

    const int index = m_tabs->addTab(tab, "New Tab");
    m_tabs->setCurrentIndex(index);
    tab->load(url);
    return tab;
}

void BrowserWindow::updateNavigationState() {
    BrowserTab *tab = currentTab();
    if (!tab) {
        m_backAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_reloadAction->setEnabled(false);
        m_closeTabAction->setEnabled(false);
        return;
    }

    QWebEngineHistory *history = tab->webView()->history();
    m_backAction->setEnabled(history->canGoBack());
    m_forwardAction->setEnabled(history->canGoForward());
    m_reloadAction->setEnabled(true);
    m_closeTabAction->setEnabled(true);
}

void BrowserWindow::updateLocationBar() {
    BrowserTab *tab = currentTab();
    if (!tab) {
        m_urlBar->clear();
        return;
    }

    m_urlBar->setText(tab->webView()->url().toString());
}

void BrowserWindow::updateWindowTitle() {
    BrowserTab *tab = currentTab();
    if (!tab) {
        setWindowTitle("Qt URL Browser");
        return;
    }

    const QString title = tab->webView()->title();
    setWindowTitle(title.isEmpty() ? "Qt URL Browser" : title + " - Qt URL Browser");
}

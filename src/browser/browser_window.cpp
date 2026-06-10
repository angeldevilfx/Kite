#include "browser_window.h"

#include "browser_tab.h"
#include "settings_dialog.h"

#include <QApplication>
#include <QAction>
#include <QByteArray>
#include <QCompleter>
#include <QDialog>
#include <QAbstractItemView>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QShortcut>
#include <QStandardPaths>
#include <QStringListModel>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QWebEngineHistory>
#include <QWebEngineView>

namespace {
QString buildHomePageHtml() {
    return QStringLiteral(R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="color-scheme" content="light dark">
  <title>Kite Browser</title>
  <style>
    :root {
      --bg: #eff0f1;
    }

    @media (prefers-color-scheme: dark) {
      :root {
        --bg: #232629;
      }
    }

    html, body { min-height: 100%; margin: 0; }
    body {
      background: var(--bg);
    }
  </style>
</head>
<body>
</body>
</html>
)HTML");
}

QUrl buildHomePageUrl() {
    const QByteArray encoded = QUrl::toPercentEncoding(buildHomePageHtml());
    return QUrl(QStringLiteral("data:text/html;charset=utf-8,") + QString::fromLatin1(encoded));
}

QUrl buildSearchUrl(const QString &query) {
    const QByteArray encoded = QUrl::toPercentEncoding(query);
    return QUrl(QStringLiteral("https://duckduckgo.com/?q=") + QString::fromLatin1(encoded));
}

bool isLikelyUrl(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }
    if (trimmed.contains(QChar::Space)) {
        return false;
    }
    if (trimmed.startsWith(QStringLiteral("http://")) || trimmed.startsWith(QStringLiteral("https://")) ||
        trimmed.startsWith(QStringLiteral("ftp://")) || trimmed.contains(QStringLiteral("://"))) {
        return true;
    }
    if (trimmed.startsWith(QStringLiteral("localhost")) || trimmed.startsWith(QStringLiteral("127.0.0.1"))) {
        return true;
    }
    return trimmed.contains(u'.') || trimmed.contains(u':');
}
}  // namespace

QUrl BrowserWindow::normalizeUrl(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return buildHomePageUrl();
    }

    if (isLikelyUrl(trimmed)) {
        const QUrl url = QUrl::fromUserInput(trimmed);
        if (url.isValid() && !url.isEmpty()) {
            return url;
        }
    }

    return buildSearchUrl(trimmed);
}

bool BrowserWindow::looksLikeUrl(const QString &text) {
    return isLikelyUrl(text);
}

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent),
      m_backAction(nullptr),
      m_forwardAction(nullptr),
      m_reloadAction(nullptr),
      m_newTabAction(nullptr),
      m_closeTabAction(nullptr),
      m_goAction(nullptr),
      m_settingsAction(nullptr),
      m_urlBar(nullptr),
      m_tabs(nullptr),
      m_toolbar(nullptr),
      m_newTabButton(nullptr),
      m_lastTabCloseTimer(nullptr),
      m_suggestionTimer(nullptr),
      m_completer(nullptr),
      m_suggestionModel(nullptr),
      m_networkManager(nullptr),
      m_updatingFromSuggestion(false),
      m_homeUrl(buildHomePageUrl()) {
    resize(1280, 800);

    m_settings = loadBrowserSettings();
    applyBrowserTheme(*qApp, m_settings.themeMode);

    m_lastTabCloseArmed = false;
    m_lastTabCloseTimer = new QTimer(this);
    m_lastTabCloseTimer->setSingleShot(true);
    m_lastTabCloseTimer->setInterval(1500);
    connect(m_lastTabCloseTimer, &QTimer::timeout, this, [this]() {
        disarmLastTabClose();
    });

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

    m_closeTabAction = m_toolbar->addAction(iconFor(QStyle::SP_DialogCloseButton), "Close Tab");
    m_closeTabAction->setToolTip("Close Tab");
    m_closeTabAction->setStatusTip("Close Tab");

    m_settingsAction = m_toolbar->addAction(iconFor(QStyle::SP_FileDialogDetailedView), "Settings");
    m_settingsAction->setToolTip("Settings");
    m_settingsAction->setStatusTip("Settings");

    m_toolbar->addSeparator();

    m_urlBar = new QLineEdit(this);
    m_urlBar->setPlaceholderText("Search or type an address");
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

    m_newTabAction = new QAction(QIcon::fromTheme("tab-new", QIcon::fromTheme("list-add", iconFor(QStyle::SP_DialogOpenButton))),
                                 "New Tab",
                                 this);
    m_newTabAction->setToolTip("New Tab");
    m_newTabAction->setStatusTip("New Tab");

    m_newTabButton = new QToolButton(this);
    m_newTabButton->setAutoRaise(true);
    m_newTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_newTabButton->setDefaultAction(m_newTabAction);
    m_tabs->setCornerWidget(m_newTabButton, Qt::TopRightCorner);

    m_networkManager = new QNetworkAccessManager(this);
    m_suggestionModel = new QStringListModel(this);
    m_completer = new QCompleter(m_suggestionModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_urlBar->setCompleter(m_completer);

    m_suggestionTimer = new QTimer(this);
    m_suggestionTimer->setSingleShot(true);
    m_suggestionTimer->setInterval(220);
    connect(m_suggestionTimer, &QTimer::timeout, this, [this]() {
        fetchSuggestions(m_pendingSuggestionText);
    });

    connect(m_urlBar, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (!m_updatingFromSuggestion) {
            queueSuggestions(text);
        }
    });

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
        closeCurrentTabOrWindow();
    });

    connect(m_settingsAction, &QAction::triggered, this, [this]() {
        openSettings();
    });

    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int) {
        updateNavigationState();
        updateLocationBar();
        updateWindowTitle();
    });

    connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        closeTabAtIndex(index);
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

    auto *closeBrowserShortcut = new QShortcut(QKeySequence::Quit, this);
    connect(closeBrowserShortcut, &QShortcut::activated, this, [this]() {
        closeBrowserImmediately();
    });

    auto *closeBrowserAltShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W), this);
    connect(closeBrowserAltShortcut, &QShortcut::activated, this, [this]() {
        closeBrowserImmediately();
    });

    auto *settingsShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma), this);
    connect(settingsShortcut, &QShortcut::activated, this, [this]() {
        openSettings();
    });

    updateCompletionBehavior();
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

    connect(tab, &BrowserTab::urlChanged, this, [this, tab](const QUrl &) {
        if (tab == currentTab()) {
            updateLocationBar();
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

    const QUrl currentUrl = tab->webView()->url();
    if (currentUrl == m_homeUrl) {
        m_urlBar->clear();
        return;
    }

    m_urlBar->setText(currentUrl.toString());
}

void BrowserWindow::updateWindowTitle() {
    BrowserTab *tab = currentTab();
    if (!tab) {
        setWindowTitle("Kite Browser");
        return;
    }

    const QString title = tab->webView()->title();
    setWindowTitle(title.isEmpty() ? "Kite Browser" : title + " - Kite Browser");
}

void BrowserWindow::openSettings() {
    SettingsDialog dialog(m_settings, this);
    if (dialog.exec() == QDialog::Accepted) {
        applySettings(dialog.settings());
    }
}

void BrowserWindow::applySettings(const KiteBrowserSettings &settings) {
    m_settings = settings;
    saveBrowserSettings(m_settings);
    applyBrowserTheme(*qApp, m_settings.themeMode);
    updateCompletionBehavior();
    queueSuggestions(m_urlBar->text());
}

void BrowserWindow::queueSuggestions(const QString &text) {
    m_pendingSuggestionText = text.trimmed();
    if (m_pendingSuggestionText.isEmpty() || (!m_settings.searchAutocomplete && !m_settings.searchSuggestions)) {
        m_suggestionTimer->stop();
        m_suggestionModel->setStringList({});
        m_completer->popup()->hide();
        return;
    }

    m_suggestionTimer->start();
}

void BrowserWindow::fetchSuggestions(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || (!m_settings.searchAutocomplete && !m_settings.searchSuggestions)) {
        return;
    }

    QUrl url(QStringLiteral("https://duckduckgo.com/ac/"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), trimmed);
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("list"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Kite Browser"));
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmed]() {
        const QByteArray payload = reply->readAll();
        reply->deleteLater();

        if (trimmed != m_pendingSuggestionText) {
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        QStringList suggestions;
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (doc.isArray()) {
            for (const QJsonValue &value : doc.array()) {
                if (value.isObject()) {
                    const QString phrase = value.toObject().value(QStringLiteral("phrase")).toString();
                    if (!phrase.isEmpty()) {
                        suggestions.push_back(phrase);
                    }
                } else if (value.isString()) {
                    const QString phrase = value.toString();
                    if (!phrase.isEmpty()) {
                        suggestions.push_back(phrase);
                    }
                }
            }
        }

        m_suggestionModel->setStringList(suggestions);

        if (m_settings.searchSuggestions && !suggestions.isEmpty()) {
            m_completer->setCompletionPrefix(trimmed);
            m_completer->complete();
        } else {
            m_completer->popup()->hide();
        }

        if (m_settings.searchAutocomplete && !suggestions.isEmpty()) {
            const QString first = suggestions.first();
            if (first.startsWith(trimmed, Qt::CaseInsensitive)) {
                m_updatingFromSuggestion = true;
                m_urlBar->setText(first);
                m_urlBar->setSelection(trimmed.length(), first.length() - trimmed.length());
                m_updatingFromSuggestion = false;
            }
        }
    });
}

void BrowserWindow::updateCompletionBehavior() {
    m_completer->setCompletionMode(m_settings.searchSuggestions ? QCompleter::PopupCompletion : QCompleter::InlineCompletion);
}

void BrowserWindow::closeTabAtIndex(int index) {
    if (index < 0) {
        return;
    }

    if (m_tabs->count() == 1) {
        if (m_lastTabCloseArmed) {
            closeBrowserImmediately();
            return;
        }

        if (BrowserTab *tab = currentTab()) {
            tab->load(m_homeUrl);
        }
        armLastTabClose();
        return;
    }

    disarmLastTabClose();

    QWidget *widget = m_tabs->widget(index);
    m_tabs->removeTab(index);
    widget->deleteLater();
    updateNavigationState();
    updateLocationBar();
    updateWindowTitle();
}

void BrowserWindow::closeCurrentTabOrWindow() {
    closeTabAtIndex(m_tabs->currentIndex());
}

void BrowserWindow::closeBrowserImmediately() {
    disarmLastTabClose();
    close();
}

void BrowserWindow::armLastTabClose() {
    m_lastTabCloseArmed = true;
    m_lastTabCloseTimer->start();
}

void BrowserWindow::disarmLastTabClose() {
    m_lastTabCloseArmed = false;
    m_lastTabCloseTimer->stop();
}

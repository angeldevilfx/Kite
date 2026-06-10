#pragma once

#include "app_settings.h"

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QUrl>

class QAction;
class QCompleter;
class QLineEdit;
class QTabWidget;
class QToolBar;
class QTimer;
class QToolButton;
class QStringListModel;
class QNetworkAccessManager;
class QNetworkReply;

class BrowserTab;

class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);

private:
    static QUrl normalizeUrl(const QString &text);
    static bool looksLikeUrl(const QString &text);

    BrowserTab *currentTab() const;
    BrowserTab *createTab(const QUrl &url);
    void updateNavigationState();
    void updateLocationBar();
    void updateWindowTitle();
    void openSettings();
    void applySettings(const KiteBrowserSettings &settings);
    void queueSuggestions(const QString &text);
    void fetchSuggestions(const QString &text);
    void updateCompletionBehavior();
    void closeTabAtIndex(int index);
    void closeCurrentTabOrWindow();
    void closeBrowserImmediately();
    void armLastTabClose();
    void disarmLastTabClose();

    QAction *m_backAction;
    QAction *m_forwardAction;
    QAction *m_reloadAction;
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_goAction;
    QAction *m_settingsAction;
    QLineEdit *m_urlBar;
    QTabWidget *m_tabs;
    QToolBar *m_toolbar;
    QToolButton *m_newTabButton;
    QTimer *m_lastTabCloseTimer;
    QTimer *m_suggestionTimer;
    QCompleter *m_completer;
    QStringListModel *m_suggestionModel;
    QNetworkAccessManager *m_networkManager;
    QUrl m_homeUrl;
    KiteBrowserSettings m_settings;
    QString m_pendingSuggestionText;
    bool m_updatingFromSuggestion;
    bool m_lastTabCloseArmed;
};

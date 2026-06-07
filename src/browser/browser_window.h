#pragma once

#include <QMainWindow>
#include <QUrl>

class QAction;
class QLineEdit;
class QTabWidget;
class QToolBar;

class BrowserTab;

class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);

private:
    static QUrl normalizeUrl(const QString &text);

    BrowserTab *currentTab() const;
    BrowserTab *createTab(const QUrl &url);
    void updateNavigationState();
    void updateLocationBar();
    void updateWindowTitle();

    QAction *m_backAction;
    QAction *m_forwardAction;
    QAction *m_reloadAction;
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_goAction;
    QLineEdit *m_urlBar;
    QTabWidget *m_tabs;
    QToolBar *m_toolbar;
    QUrl m_homeUrl;
};

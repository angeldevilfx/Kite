#pragma once

#include "app_settings.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QPushButton;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const KiteBrowserSettings &settings, QWidget *parent = nullptr);

    KiteBrowserSettings settings() const;

private:
    void setAsDefaultBrowser();

    KiteBrowserSettings m_settings;
    QComboBox *m_themeCombo;
    QCheckBox *m_autocompleteCheck;
    QCheckBox *m_suggestionsCheck;
    QPushButton *m_defaultBrowserButton;
};

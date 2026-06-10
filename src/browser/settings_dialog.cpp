#include "settings_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const KiteBrowserSettings &settings, QWidget *parent)
    : QDialog(parent),
      m_settings(settings),
      m_themeCombo(new QComboBox(this)),
      m_autocompleteCheck(new QCheckBox(tr("Enable search autocomplete"), this)),
      m_suggestionsCheck(new QCheckBox(tr("Enable search suggestions"), this)),
      m_defaultBrowserButton(new QPushButton(tr("Set Kite as default browser"), this)) {
    setWindowTitle(tr("Settings"));
    setModal(true);
    resize(520, 360);

    auto *tabs = new QTabWidget(this);

    auto *generalTab = new QWidget(this);
    auto *generalLayout = new QFormLayout(generalTab);
    generalLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_themeCombo->addItem(tr("System"), static_cast<int>(KiteThemeMode::System));
    m_themeCombo->addItem(tr("Light"), static_cast<int>(KiteThemeMode::Light));
    m_themeCombo->addItem(tr("Dark"), static_cast<int>(KiteThemeMode::Dark));
    m_themeCombo->setCurrentIndex(m_themeCombo->findData(static_cast<int>(m_settings.themeMode)));

    generalLayout->addRow(tr("App theme"), m_themeCombo);
    generalLayout->addRow(QString(), new QLabel(tr("Theme controls the app chrome. The home page follows the KDE light/dark background only."), this));
    generalLayout->addRow(QString(), m_defaultBrowserButton);

    auto *searchTab = new QWidget(this);
    auto *searchLayout = new QVBoxLayout(searchTab);
    searchLayout->addWidget(new QLabel(tr("DuckDuckGo powers search, autocomplete, and suggestions."), this));
    m_autocompleteCheck->setChecked(m_settings.searchAutocomplete);
    m_suggestionsCheck->setChecked(m_settings.searchSuggestions);
    searchLayout->addWidget(m_autocompleteCheck);
    searchLayout->addWidget(m_suggestionsCheck);
    searchLayout->addStretch(1);

    tabs->addTab(generalTab, tr("General"));
    tabs->addTab(searchTab, tr("Search"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    connect(m_defaultBrowserButton, &QPushButton::clicked, this, [this]() {
        setAsDefaultBrowser();
    });

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttons);
}

KiteBrowserSettings SettingsDialog::settings() const {
    KiteBrowserSettings result = m_settings;
    result.themeMode = static_cast<KiteThemeMode>(m_themeCombo->currentData().toInt());
    result.searchAutocomplete = m_autocompleteCheck->isChecked();
    result.searchSuggestions = m_suggestionsCheck->isChecked();
    return result;
}

void SettingsDialog::setAsDefaultBrowser() {
    const bool ok1 = QProcess::startDetached(QStringLiteral("xdg-mime"),
                                             {QStringLiteral("default"),
                                              QStringLiteral("kitebrowser.desktop"),
                                              QStringLiteral("x-scheme-handler/http"),
                                              QStringLiteral("x-scheme-handler/https")});
    const bool ok2 = QProcess::startDetached(QStringLiteral("xdg-settings"),
                                             {QStringLiteral("set"),
                                              QStringLiteral("default-web-browser"),
                                              QStringLiteral("kitebrowser.desktop")});

    QMessageBox::information(this,
                             tr("Set as default browser"),
                             ok1 || ok2
                                 ? tr("Kite was sent to the desktop settings tools as the default browser request.")
                                 : tr("Unable to launch the system default-browser tools."));
}

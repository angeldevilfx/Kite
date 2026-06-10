#include "app_settings.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QSettings>
#include <QString>

namespace {
constexpr const char *kOrg = "Kite";
constexpr const char *kApp = "Kite Browser";

QPalette buildDarkPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#232629"));
    palette.setColor(QPalette::WindowText, QColor("#eff0f1"));
    palette.setColor(QPalette::Base, QColor("#31363b"));
    palette.setColor(QPalette::AlternateBase, QColor("#2a2e32"));
    palette.setColor(QPalette::ToolTipBase, QColor("#eff0f1"));
    palette.setColor(QPalette::ToolTipText, QColor("#232629"));
    palette.setColor(QPalette::Text, QColor("#eff0f1"));
    palette.setColor(QPalette::Button, QColor("#31363b"));
    palette.setColor(QPalette::ButtonText, QColor("#eff0f1"));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor("#3daee9"));
    palette.setColor(QPalette::Highlight, QColor("#1d99f3"));
    palette.setColor(QPalette::HighlightedText, QColor("#eff0f1"));
    return palette;
}

QPalette buildLightPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#eff0f1"));
    palette.setColor(QPalette::WindowText, QColor("#31363b"));
    palette.setColor(QPalette::Base, QColor("#fcfcfc"));
    palette.setColor(QPalette::AlternateBase, QColor("#f4f5f6"));
    palette.setColor(QPalette::ToolTipBase, QColor("#fcfcfc"));
    palette.setColor(QPalette::ToolTipText, QColor("#31363b"));
    palette.setColor(QPalette::Text, QColor("#31363b"));
    palette.setColor(QPalette::Button, QColor("#fcfcfc"));
    palette.setColor(QPalette::ButtonText, QColor("#31363b"));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor("#3daee9"));
    palette.setColor(QPalette::Highlight, QColor("#1d99f3"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    return palette;
}
}  // namespace

KiteBrowserSettings loadBrowserSettings() {
    QSettings settings(kOrg, kApp);
    KiteBrowserSettings result;
    result.themeMode = themeModeFromString(settings.value("themeMode", QStringLiteral("system")).toString());
    result.searchAutocomplete = settings.value("searchAutocomplete", true).toBool();
    result.searchSuggestions = settings.value("searchSuggestions", true).toBool();
    return result;
}

void saveBrowserSettings(const KiteBrowserSettings &settings) {
    QSettings qsettings(kOrg, kApp);
    qsettings.setValue("themeMode", themeModeToString(settings.themeMode));
    qsettings.setValue("searchAutocomplete", settings.searchAutocomplete);
    qsettings.setValue("searchSuggestions", settings.searchSuggestions);
}

void applyBrowserTheme(QApplication &app, KiteThemeMode mode) {
    if (mode == KiteThemeMode::System) {
        app.setPalette(app.style()->standardPalette());
        app.setStyleSheet({});
        return;
    }

    app.setPalette(mode == KiteThemeMode::Dark ? buildDarkPalette() : buildLightPalette());
}

QString themeModeToString(KiteThemeMode mode) {
    switch (mode) {
    case KiteThemeMode::Light:
        return QStringLiteral("light");
    case KiteThemeMode::Dark:
        return QStringLiteral("dark");
    case KiteThemeMode::System:
    default:
        return QStringLiteral("system");
    }
}

KiteThemeMode themeModeFromString(const QString &value) {
    if (value == QLatin1String("light")) {
        return KiteThemeMode::Light;
    }
    if (value == QLatin1String("dark")) {
        return KiteThemeMode::Dark;
    }
    return KiteThemeMode::System;
}

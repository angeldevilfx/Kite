#pragma once

#include <QPalette>
#include <QString>

class QApplication;

enum class KiteThemeMode {
    System = 0,
    Light,
    Dark,
};

struct KiteBrowserSettings {
    KiteThemeMode themeMode = KiteThemeMode::System;
    bool searchAutocomplete = true;
    bool searchSuggestions = true;
};

KiteBrowserSettings loadBrowserSettings();
void saveBrowserSettings(const KiteBrowserSettings &settings);
void applyBrowserTheme(QApplication &app, KiteThemeMode mode);
QString themeModeToString(KiteThemeMode mode);
KiteThemeMode themeModeFromString(const QString &value);

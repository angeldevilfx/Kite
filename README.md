# Kite Browser
Kite Browser is a lightweight tabbed browser for KDE Plasma users. It is built with C++, Qt, and Qt Web Engine for displaying web content.

The current build includes:

- Tabbed browsing
- Back, forward, reload, and new tab controls
- URL bar navigation with keyboard shortcuts
- A simple, minimal interface focused on daily use

## Usage

Build the project with CMake:

```bash
cmake -S . -B build
cmake --build build
```

Then run the binary from the `build/` directory:

```bash
./build/kitebrowser
```

## Roadmap

- Better tab management
- Saved sessions and history
- Downloads and bookmarks
- More browser settings and polish

# Astrolog 8.00

This is a copy of the "official" version of Astrolog 8.00 (released May 31, 2026) described at: http://www.astrolog.org/astrolog.htm

The files here were copied from: http://www.astrolog.org/ftp/ast80src.zip

The changes in version 8.00 are described at: http://www.astrolog.org/ftp/updat800.htm

In addition to this README, this copy adds a Qt 6 GUI version for Linux,
described in [Linux Qt version](#linux-qt-version) below. It adds new files
(`wqt.h`, `wqt.cpp`, `rcqt.inc`, `tools/rc2qt.py`, `astrolog.desktop`, and
the `qt` target in the `Makefile`), and makes small `#ifdef` guarded changes
to some of the original source files. The Windows and the original X11
versions build and work the same as before.

## Linux Qt version

On Windows, Astrolog has a menu bar, right-click menus, and settings dialogs.
The original Unix version only has a bare X11 window controlled by single
keys. The Qt version (`make qt`) gives Linux the same interface as the
Windows version:

- The full menu bar and all right-click chart menus, with check marks.
- All settings dialogs (Chart Info, Aspects, Objects, Graphics, Transits, ...).
- All Windows keyboard shortcuts, mouse scribbling, and dragging to rotate globes.
- Text charts in the window, animation, file open/save dialogs, clipboard
  copy/paste, printing, and the TrueType astrology fonts in charts.
- It runs natively on Wayland and on X11, e.g. on KDE Plasma.

The Qt version reuses the Windows menu and dialog code (`wdriver.cpp`,
`wdialog.cpp`) unchanged, on top of a small part of the Win32 API implemented
with Qt (`wqt.h`, `wqt.cpp`). The menus, dialogs, and keyboard shortcuts are
generated from the Windows resource file `astrolog.rc` (see `rcqt.inc`).
Charts are drawn by Astrolog into its own memory bitmap, so no X11 or Windows
drawing calls are involved.

### Requirements

- A C++17 compiler (GCC or Clang), `make`, and `pkg-config`.
- Qt 6.2 or newer, with the Widgets and PrintSupport modules (both part of
  Qt's base package).
- On Wayland: the Qt Wayland platform plugin. Without it, Qt runs through
  XWayland instead.
- Optional: Python 3, only needed to regenerate `rcqt.inc` after changing
  `astrolog.rc`.

### Install the build packages

**Void Linux:**

```sh
sudo xbps-install -S base-devel qt6-base-devel qt6-wayland-client
```

- `base-devel` provides `gcc` (including `g++`), `make`, and `pkg-config`.
- `qt6-base-devel` provides the Qt 6 headers and the pkg-config files for
  `Qt6Widgets` and `Qt6PrintSupport`, and pulls in the Qt runtime libraries.
- `qt6-wayland-client` is the Wayland platform plugin. It is a separate
  package on Void, so install it to run natively on Wayland.
- Optional: `python3` to regenerate `rcqt.inc`, and `libX11-devel` to also
  build the original X11 version with plain `make`.
- Text charts need a fixed width font. If no font is installed, e.g. on a
  minimal system, install one such as `dejavu-fonts-ttf`.

**Debian / Ubuntu / KDE neon:**

```sh
sudo apt install build-essential pkg-config qt6-base-dev qt6-wayland
```

**Fedora:**

```sh
sudo dnf install gcc-c++ make pkgconf-pkg-config qt6-qtbase-devel qt6-qtwayland
```

**Arch Linux:**

```sh
sudo pacman -S --needed base-devel qt6-base qt6-wayland
```

### Build

In the Astrolog source folder:

```sh
make qt
```

This creates the program `astrolog-qt`. Its object files go in the folder
`obj-qt/`, separate from the X11 version's, so both versions can be built in
the same folder. You don't need to edit `astrolog.h` for the Qt version:
`make qt` passes `-DQT`, which also turns off the X11 code.

To build faster on a computer with several cores, use e.g. `make -j8 qt`.

If pkg-config can't find Qt (`Package Qt6Widgets was not found`), check that
the Qt 6 development package is installed. If Qt is installed somewhere
non-standard, point pkg-config at it, e.g.:

```sh
PKG_CONFIG_PATH=/opt/qt6/lib/pkgconfig make qt
```

To remove the build output of both versions:

```sh
make clean
```

### Run

Astrolog looks for its data files (`astrolog.as`, `atlas.as`, `timezone.as`,
the `ephem/` ephemeris files, `font/`, and the `.ico` icons) in the folder
of the program, then the current folder, then the folder named by the
`ASTROLOG` environment variable. So the simplest way is to run it from the
source folder:

```sh
./astrolog-qt
```

To run it from elsewhere, set the environment variable to the source folder:

```sh
ASTROLOG=$HOME/src/Astrolog ./astrolog-qt
```

Command line switches work like in the Windows version, e.g. `./astrolog-qt -X`
starts with a graphics chart.

Qt picks Wayland or X11 automatically. To choose one yourself:

```sh
QT_QPA_PLATFORM=wayland ./astrolog-qt
```

```sh
QT_QPA_PLATFORM=xcb ./astrolog-qt
```

### Add it to the application menu

`astrolog.desktop` is a desktop entry for KDE Plasma and other desktops. To
add it for your user, copy it and change `Exec` to the full path of the
program (and of the source folder):

```sh
mkdir -p ~/.local/share/applications
sed -e "s|^Exec=.*|Exec=env ASTROLOG=$PWD $PWD/astrolog-qt|" \
    -e "s|^Icon=.*|Icon=$PWD/astrlog1.ico|" \
    astrolog.desktop > ~/.local/share/applications/astrolog.desktop
```

On Wayland, the desktop entry is also what gives the window its name and
icon in the Plasma task bar. Without it installed, Qt may print a harmless
warning at startup like `qt.qpa.services: Failed to register with host
portal ... App info not found for 'astrolog'`. Installing the desktop entry
as above makes it go away.

### Regenerate the menus and dialogs

`rcqt.inc` is generated from `astrolog.rc` and `resource.h`, and is included
in the repository, so building doesn't need Python. After changing the menus,
dialogs, or keyboard shortcuts in `astrolog.rc`, regenerate it with:

```sh
make rcqt
```

### Differences from the Windows version

- On Wayland, programs can't position their own windows, so window position
  switches such as `-Ww` only change the size there.
- Drawing on the desktop background (`-XB`), setting the desktop wallpaper,
  and the Windows setup commands (program group, file associations) aren't
  available, and are removed from the menus.
- Antialiasing smooths the chart like for bitmap files. The Windows zoom
  antialiasing level (`-Wx`) is ignored.
- File / Print prints graphics charts as shown in the window, scaled to fit
  the page, and text charts as plain text.
- Keyboard shortcuts assume a US keyboard layout for shifted keys, like
  Windows does.

### Original X11 version

The original Unix version with the bare X11 window still builds as before:

```sh
make
```

It needs the X11 development package, e.g. `libX11-devel` on Void Linux or
`libx11-dev` on Debian and Ubuntu.

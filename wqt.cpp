/*
** Astrolog (Version 8.00) File: wqt.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated elsewhere in this program are Copyright (C) 1991-2026 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** This file contains the Qt 6 version's main program, window, and message
** handling, and implements the subset of the Win32 API declared in wqt.h, so
** that Astrolog's Windows menu and dialog code in wdriver.cpp and wdialog.cpp
** can be reused as is. It is only compiled when QT is defined (make qt).
**
** Charts are drawn by the core program into its 24 bit memory bitmap gi.bmp
** (the same path used for writing bitmap files), which is displayed in the
** window. No Xlib or GDI calls are made, so this runs natively on Wayland as
** well as X11.
*/

// Qt headers must come before astrolog.h, since astrolog.h defines short
// macros (e.g. loop, inv, neg, P, S, min, max) that would clash with them.
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QCursor>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QScrollBar>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>
#include <QWidget>
#include <map>
#include <tuple>
#include <vector>
#include <string>
#include <unistd.h>

#include "astrolog.h"
#undef min
#undef max


#ifdef QT
/*
******************************************************************************
** Internal State.
******************************************************************************
*/

// The objects behind the opaque Win32 style handles declared in wqt.h.

struct QtDlgState;

struct QtWnd {
  QWidget *widget;
  QtDlgState *pds;  // The dialog this window is or is in, if any.
};

// A dialog being run with DialogBox().
class AstroDialog;
struct QtDlgState {
  AstroDialog *dlg;   // The dialog window.
  QtWnd wnd;          // The handle for the dialog itself.
  DLGPROC pfn;        // The dialog procedure in wdialog.cpp.
  std::map<int, QtWnd *> mapCtl;  // Handles for the controls by id.
  flag fEnded;        // Has EndDialog() been called?
  intptr_t nResult;   // The value passed to EndDialog().
};

// A menu or submenu. Each menu resource loaded (the main menu bar, or a
// context menu) is a tree of these, all sharing the same root, which knows
// every action in the tree by its command id.

struct QtMenuAction {
  QAction *action;
  QMenu *parent;
};

struct QtMenu {
  QMenu *qmenu;                 // The Qt menu, or NULL for a root.
  QtMenu *root;                 // Root of this menu tree.
  std::vector<QtMenu *> rgSub;  // Submenu at each position, or NULL.
  std::multimap<UINT, QtMenuAction> mapCmd;  // Root only: Actions by id.
  std::vector<QtMenu *> rgAll;  // Root only: All nodes, for freeing.
};

struct QtDC {
  QImage *img;      // Image being drawn upon, if any.
  QPainter *paint;  // Active painter on that image, if any.
  COLORREF kvText;  // Current text color.
  int nBkMode;      // TRANSPARENT or OPAQUE.
};

// Keyboard accelerator table entry, generated into rcqt.inc.
struct QtAccel {
  int key;    // Virtual key code, or character if not QTACCEL_VIRTKEY.
  int flags;  // QTACCEL_* bits below.
  int cmd;    // Menu command to run.
};
#define QTACCEL_VIRTKEY 1
#define QTACCEL_SHIFT   2
#define QTACCEL_CONTROL 4
#define QTACCEL_ALT     8

// Menu table entry, generated into rcqt.inc.
struct QtMenuEntry {
  int idMenu;  // Menu resource id, e.g. menu or menuV.
  int nKind;   // QTMENU_* kind below.
  int cmd;     // Menu command, for QTMENU_ITEM.
  CONST char *sz;  // Text (UTF-8), for QTMENU_ITEM and QTMENU_POPUP.
};
#define QTMENU_ITEM  0
#define QTMENU_SEP   1
#define QTMENU_POPUP 2
#define QTMENU_END   3

// Dialog table entry, generated into rcqt.inc. Positions are dialog units.
struct QtDlgEntry {
  int idDlg;   // Dialog resource id, e.g. dlgInfo.
  int nKind;   // QTDLG_* kind below.
  int id;      // Control id (-1 for IDC_STATIC).
  int x, y, cx, cy;
  int nFlags;  // QTDLG_F* flags below.
  CONST char *sz;  // Text (UTF-8), the caption, or icon file name.
};
#define QTDLG_DIALOG    0
#define QTDLG_LTEXT     1
#define QTDLG_RTEXT     2
#define QTDLG_CTEXT     3
#define QTDLG_EDIT      4
#define QTDLG_COMBO     5
#define QTDLG_LIST      6
#define QTDLG_BUTTON    7
#define QTDLG_DEFBUTTON 8
#define QTDLG_GROUPBOX  9
#define QTDLG_ICON      10
#define QTDLG_CHECK     11
#define QTDLG_RADIO     12
#define QTDLG_STATIC    13
#define QTDLG_FGROUP     0x001
#define QTDLG_FTABSTOP   0x002
#define QTDLG_FSORT      0x004
#define QTDLG_FDISABLED  0x008
#define QTDLG_FHIDDEN    0x010
#define QTDLG_FDROPLIST  0x020
#define QTDLG_FREADONLY  0x040
#define QTDLG_FMULTILINE 0x080
#define QTDLG_FNUMBER    0x100
#define QTDLG_FPASSWORD  0x200

#include "rcqt.inc"

class AstroChartView;
class AstroMainWindow;

static QApplication *g_app = NULL;
static AstroMainWindow *g_win = NULL;
static AstroChartView *g_view = NULL;
static QScrollBar *g_sbHorz = NULL, *g_sbVert = NULL;
static QTimer *g_timer = NULL;
static QImage g_imgChart;        // What's currently displayed in the window.
static flag g_fImgText = fFalse; // Is the above a text chart?
static flag g_fRedrawPending = fFalse;
static flag g_fClosing = fFalse;
static int g_cWait = 0;          // Nesting count of hourglass cursor.

static QtWnd g_wndMain = {NULL, NULL};
static QtMenu *g_pmenuMain = NULL;  // The main menu bar's tree.
static QtDC g_dcWin = {NULL, NULL, 0xffffff, OPAQUE};

#define HCURSOR_ARROW ((HCURSOR)(intptr_t)1)
#define HCURSOR_WAIT  ((HCURSOR)(intptr_t)2)

static void QtDispatch(UINT wMsg, WPARAM wParam, LPARAM lParam);
static void QtScheduleRedraw(void);


/*
******************************************************************************
** Helper Routines.
******************************************************************************
*/

// Convert an Astrolog color value (Win32 COLORREF layout) to a Qt color.

static QColor QcFromKv(KV kv)
{
  return QColor(RgbR(kv), RgbG(kv), RgbB(kv));
}


// Convert an 8 bit string from Astrolog in its current output character set
// to a Qt Unicode string.

static QString QsFromSz(CONST char *sz, int cch = -1)
{
  QString qs;
  int i;

  if (sz == NULL)
    return qs;
  if (cch < 0)
    cch = CchSz(sz);
  if (us.nCharset >= ccUTF8)
    return QString::fromUtf8(sz, cch);
  for (i = 0; i < cch; i++)
    qs += QChar((ushort)(us.nCharset == ccIBM ? WchFromChIBM((uchar)sz[i]) :
      WchFromChLatin((uchar)sz[i])));
  return qs;
}


// Copy a Qt string into an 8 bit Astrolog string buffer as UTF-8.

static void CopyQsToSz(const QString &qs, char *sz, int cchMax)
{
  QByteArray ba = qs.toUtf8();
  int cch;

  if (cchMax <= 0)
    return;
  cch = (int)ba.size();
  if (cch > cchMax-1)
    cch = cchMax-1;
  memcpy(sz, ba.constData(), cch);
  sz[cch] = chNull;
}


// Convert a Qt key event to a Win32 virtual key code. Like Windows, this
// assumes a US keyboard layout for which shifted keys map to which keys.

static int VkFromQtKey(int key)
{
  if (key >= Qt::Key_A && key <= Qt::Key_Z)
    return 'A' + (key - Qt::Key_A);
  if (key >= Qt::Key_0 && key <= Qt::Key_9)
    return '0' + (key - Qt::Key_0);
  if (key >= Qt::Key_F1 && key <= Qt::Key_F24)
    return VK_F1 + (key - Qt::Key_F1);
  switch (key) {
  case Qt::Key_Backspace:   return 0x08;
  case Qt::Key_Tab:
  case Qt::Key_Backtab:     return 0x09;
  case Qt::Key_Return:
  case Qt::Key_Enter:       return 0x0D;
  case Qt::Key_Pause:       return 0x13;
  case Qt::Key_Escape:      return 0x1B;
  case Qt::Key_Space:       return 0x20;
  case Qt::Key_PageUp:      return 0x21;
  case Qt::Key_PageDown:    return 0x22;
  case Qt::Key_End:         return 0x23;
  case Qt::Key_Home:        return 0x24;
  case Qt::Key_Left:        return 0x25;
  case Qt::Key_Up:          return 0x26;
  case Qt::Key_Right:       return 0x27;
  case Qt::Key_Down:        return 0x28;
  case Qt::Key_Exclam:      return '1';
  case Qt::Key_At:          return '2';
  case Qt::Key_NumberSign:  return '3';
  case Qt::Key_Dollar:      return '4';
  case Qt::Key_Percent:     return '5';
  case Qt::Key_AsciiCircum: return '6';
  case Qt::Key_Ampersand:   return '7';
  case Qt::Key_Asterisk:    return '8';
  case Qt::Key_ParenLeft:   return '9';
  case Qt::Key_ParenRight:  return '0';
  case Qt::Key_Semicolon:
  case Qt::Key_Colon:       return 0xBA;
  case Qt::Key_Plus:
  case Qt::Key_Equal:       return 0xBB;
  case Qt::Key_Comma:
  case Qt::Key_Less:        return 0xBC;
  case Qt::Key_Minus:
  case Qt::Key_Underscore:  return 0xBD;
  case Qt::Key_Period:
  case Qt::Key_Greater:     return 0xBE;
  case Qt::Key_Slash:
  case Qt::Key_Question:    return 0xBF;
  case Qt::Key_QuoteLeft:
  case Qt::Key_AsciiTilde:  return 0xC0;
  case Qt::Key_BracketLeft:
  case Qt::Key_BraceLeft:   return 0xDB;
  case Qt::Key_Backslash:
  case Qt::Key_Bar:         return 0xDC;
  case Qt::Key_BracketRight:
  case Qt::Key_BraceRight:  return 0xDD;
  case Qt::Key_Apostrophe:
  case Qt::Key_QuoteDbl:    return 0xDE;
  }
  return 0;
}


// Look up a key press in the accelerator table generated from astrolog.rc,
// returning the menu command it invokes, or 0 if none. Like Windows, virtual
// key entries (which must match modifiers exactly) take precedence over
// character entries.

static int CmdFromKeyEvent(QKeyEvent *e)
{
  Qt::KeyboardModifiers km = e->modifiers();
  int vk = VkFromQtKey(e->key()), flags = 0, i;
  QString qs;

  if (km & Qt::ShiftModifier)
    flags |= QTACCEL_SHIFT;
  if (km & Qt::ControlModifier)
    flags |= QTACCEL_CONTROL;
  if (km & Qt::AltModifier)
    flags |= QTACCEL_ALT;
  if (vk != 0)
    for (i = 0; rgQtAccel[i].cmd != 0; i++)
      if ((rgQtAccel[i].flags & QTACCEL_VIRTKEY) && rgQtAccel[i].key == vk &&
        (rgQtAccel[i].flags & ~QTACCEL_VIRTKEY) == flags)
        return rgQtAccel[i].cmd;
  if (flags & (QTACCEL_CONTROL | QTACCEL_ALT))
    return 0;
  qs = e->text();
  if (qs.size() != 1 || qs[0].unicode() >= 128)
    return 0;
  for (i = 0; rgQtAccel[i].cmd != 0; i++)
    if (!(rgQtAccel[i].flags & QTACCEL_VIRTKEY) &&
      rgQtAccel[i].key == qs[0].unicode())
      return rgQtAccel[i].cmd;
  return 0;
}


// Return the Win32 style MK_* flags for the current mouse and modifier keys.

static WPARAM MkFromQt(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
  WPARAM w = 0;

  if (mb & Qt::LeftButton)
    w |= MK_LBUTTON;
  if (mb & Qt::RightButton)
    w |= MK_RBUTTON;
  if (km & Qt::ShiftModifier)
    w |= MK_SHIFT;
  if (km & Qt::ControlModifier)
    w |= MK_CONTROL;
  return w;
}


// Return the pixel offset the displayed chart image has within the window,
// based on the scrollbar positions, the same as the Windows version does.

static void GetChartOffset(int *px, int *py)
{
  if (g_fImgText || g_imgChart.isNull()) {
    *px = *py = 0;
    return;
  }
  *px = NMultDiv(wi.xClient - g_imgChart.width(), wi.xScroll, nScrollDiv);
  *py = NMultDiv(wi.yClient - g_imgChart.height(), wi.yScroll, nScrollDiv);
}


/*
******************************************************************************
** Chart View and Main Window.
******************************************************************************
*/

// The client area of the main window, which displays the current chart, and
// forwards mouse and keyboard input to the Windows style message handler.

class AstroChartView : public QWidget
{
public:
  AstroChartView(QWidget *parent) : QWidget(parent)
  {
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(fTrue);
    setMinimumSize(BITMAPX1, BITMAPY1);
  }

protected:
  void paintEvent(QPaintEvent *) override
  {
    QPainter p(this);
    int x, y;

    p.fillRect(rect(), QcFromKv(KvFromKi(g_fImgText ?
      (gs.fInverse ? kWhiteA : kBlackA) : gi.kiOff)));
    if (g_imgChart.isNull())
      return;
    GetChartOffset(&x, &y);
    gi.xOffset = x; gi.yOffset = y;
    p.drawImage(x, y, g_imgChart);
  }

  void resizeEvent(QResizeEvent *) override
  {
    QtDispatch(WM_SIZE, 0, MAKELPARAM(width(), height()));
    QtScheduleRedraw();
  }

  void mousePressEvent(QMouseEvent *e) override
  {
    QPoint pt = e->position().toPoint();

    if (e->button() == Qt::LeftButton)
      QtDispatch(WM_LBUTTONDOWN, MkFromQt(e->buttons(), e->modifiers()),
        MAKELPARAM(pt.x(), pt.y()));
    else if (e->button() == Qt::RightButton)
      QtDispatch(WM_RBUTTONDOWN, MkFromQt(e->buttons(), e->modifiers()),
        MAKELPARAM(pt.x(), pt.y()));
  }

  void mouseReleaseEvent(QMouseEvent *e) override
  {
    QPoint pt = e->position().toPoint();

    if (e->button() == Qt::RightButton)
      QtDispatch(WM_RBUTTONUP, MkFromQt(e->buttons(), e->modifiers()),
        MAKELPARAM(pt.x(), pt.y()));
  }

  void mouseMoveEvent(QMouseEvent *e) override
  {
    QPoint pt = e->position().toPoint();

    if (e->buttons() == Qt::NoButton)
      return;
    QtDispatch(WM_MOUSEMOVE, MkFromQt(e->buttons(), e->modifiers()),
      MAKELPARAM(pt.x(), pt.y()));
  }

  void wheelEvent(QWheelEvent *e) override
  {
    int n = e->angleDelta().y(), m = e->angleDelta().x();

    if (n != 0)
      QtDispatch(WM_VSCROLL, n > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
    if (m != 0)
      QtDispatch(WM_HSCROLL, m > 0 ? SB_LINELEFT : SB_LINERIGHT, 0);
    e->accept();
  }

  void keyPressEvent(QKeyEvent *e) override
  {
    int cmd = CmdFromKeyEvent(e);

    if (cmd == 0) {
      QWidget::keyPressEvent(e);
      return;
    }
    e->accept();
    QtDispatch(WM_COMMAND, (WPARAM)cmd, 0);
  }

  // Don't let Tab move keyboard focus away, since Tab is an accelerator.
  bool focusNextPrevChild(bool) override
  {
    return fFalse;
  }

  // Like Windows, accelerator keys take precedence over menu mnemonics such
  // as Alt+F, so claim any key that has an accelerator before Qt's shortcut
  // handling sees it. It then arrives in keyPressEvent() above.
  bool event(QEvent *e) override
  {
    if (e->type() == QEvent::ShortcutOverride &&
      CmdFromKeyEvent((QKeyEvent *)e) != 0) {
      e->accept();
      return fTrue;
    }
    return QWidget::event(e);
  }
};


// The top level frame window, containing the menu bar, chart view, and the
// scrollbars (which work like the Windows version's, from 0 to nScrollDiv).

class AstroMainWindow : public QMainWindow
{
public:
  AstroMainWindow()
  {
    QWidget *w = new QWidget(this);
    QGridLayout *grid = new QGridLayout(w);

    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);
    g_view = new AstroChartView(w);
    g_sbHorz = new QScrollBar(Qt::Horizontal, w);
    g_sbVert = new QScrollBar(Qt::Vertical, w);
    g_sbHorz->setRange(0, nScrollDiv);
    g_sbVert->setRange(0, nScrollDiv);
    g_sbHorz->setPageStep(nScrollPage);
    g_sbVert->setPageStep(nScrollPage);
    grid->addWidget(g_view, 0, 0);
    grid->addWidget(g_sbVert, 0, 1);
    grid->addWidget(g_sbHorz, 1, 0);
    setCentralWidget(w);
    QObject::connect(g_sbHorz, &QScrollBar::actionTriggered, [](int)
      { QtDispatch(WM_HSCROLL,
        MAKEWPARAM(SB_THUMBPOSITION, g_sbHorz->sliderPosition()), 0); });
    QObject::connect(g_sbVert, &QScrollBar::actionTriggered, [](int)
      { QtDispatch(WM_VSCROLL,
        MAKEWPARAM(SB_THUMBPOSITION, g_sbVert->sliderPosition()), 0); });
    setWindowTitle(szAppNameCore " " szVersionCore);
  }

protected:
  void closeEvent(QCloseEvent *e) override
  {
    if (!g_fClosing) {
      e->ignore();
      QtDispatch(WM_CLOSE, 0, 0);
    } else
      e->accept();
  }
};


/*
******************************************************************************
** Message Handling.
******************************************************************************
*/

// Draw on the chart window with the scribble pen, as done with the mouse.
// This corresponds to the drawing on the window DC in Windows' WndProc().

static void ScribbleDraw(int nShape, int x1, int y1, int x2, int y2)
{
  int xo, yo, n;

  if (g_imgChart.isNull() || g_fImgText)
    return;
  GetChartOffset(&xo, &yo);
  QPainter p(&g_imgChart);
  p.translate(-xo, -yo);
  n = (!gs.fThick ? 0 : 2) + gs.nThickAdjust;
  p.setPen(QPen(QcFromKv(KvFromKi(gi.kiPen)), n < 1 ? 1 : n));
  p.setBrush(Qt::NoBrush);
  if (nShape == 0)
    p.drawPoint(x2, y2);
  else if (nShape == 1)
    p.drawLine(x1, y1, x2, y2);
  else if (nShape == 2)
    p.drawRect(QRect(QPoint(x1, y1), QPoint(x2, y2)).normalized());
  else
    p.drawEllipse(QRect(QPoint(x1, y1), QPoint(x2, y2)).normalized());
  p.end();
  g_view->update();
}


// Close the main window and exit the event loop, if allowed.

static void DoClose(void)
{
  if (us.fNoQuit) {
    PrintWarning("Program exiting is not allowed now.");
    return;
  }
  if (wi.lTimer != 0)
    KillTimer(wi.hwnd, 1);
  g_fClosing = fTrue;
  if (g_win != NULL)
    g_win->close();
  PostQuitMessage(0);
}


// Process one Windows style message for the main window. This is the Qt
// version's equivalent of WndProc() in wdriver.cpp, and follows it closely.

static void QtDispatch(UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  int iParam, x, y, n;

  wi.hwnd = wi.hwndMain;
  switch (wMsg) {

  // A command, most likely a menu option or accelerator key, was given.
  case WM_COMMAND:
    iParam = (int)(wParam & 0xffff);
    n = NWmCommand((WORD)iParam);
    if (n == 1 || (n < 0 && iParam == cmdMacro40/*Alt+F4*/))
      DoClose();
    else if (n == 0)
      ProcessState();
    break;

  // The window has been resized. Change the chart size if need be.
  case WM_SIZE:
    wi.xClient = LOWORD(lParam);
    wi.yClient = HIWORD(lParam);
    if (wi.fWindowChart || wi.fChartWindow) {
      gs.xWin = wi.xClient; gs.yWin = wi.yClient;
      if (!wi.fNotManual) {
        gi.xWinResize = gs.xWin; gi.yWinResize = gs.yWin;
      }
    }
    break;

  // The mouse has been left clicked or dragged over the window.
  case WM_LBUTTONDOWN:
  case WM_MOUSEMOVE:
    x = (short)LOWORD(lParam);
    y = (short)HIWORD(lParam);
    if (wMsg == WM_MOUSEMOVE) {
      // Dragging with right mouse down rotates and tilts globes.
      if ((wParam & MK_RBUTTON) != 0 && us.fGraphics && (fMap ||
        gi.nMode == gMidpoint || gi.nMode == gLocal ||
        gi.nMode == gSphere || gi.nMode == gGlobe ||
        gi.nMode == gPolar || gi.nMode == gTelescope)) {
        gs.rRot += (real)(x-(short)WLo(wi.lParamRC)) * rDegHalf /
          (real)gs.xWin * (gi.nMode == gLocal || gi.nMode == gTelescope ?
          -gi.zViewRatio : 1.0);
        gs.rTilt += (real)(y-(short)WHi(wi.lParamRC)) * rDegHalf /
          (real)gs.yWin * (gi.nMode == gLocal || gi.nMode == gTelescope ?
          gi.zViewRatio : (gi.nMode == gGlobe ? -1.0 : 1.0));
        while (gs.rRot >= rDegMax)
          gs.rRot -= rDegMax;
        while (gs.rRot < 0.0)
          gs.rRot += rDegMax;
        if (gs.rTilt > rDegQuad)
          gs.rTilt = rDegQuad;
        if (gs.rTilt < -rDegQuad)
          gs.rTilt = -rDegQuad;
        if (gi.nMode == gMidpoint || gi.nMode == gTelescope) {
          if (gi.nMode == gMidpoint && gs.objTrack >= 0)
            gs.rRot = planet[gs.objTrack];
          gs.objTrack = -1;
        }
        wi.lParamRC = lParam;
        wi.fRedraw = fTrue;
        wi.fMoved = fTrue;
        ProcessState();
        break;
      }

      // Treat dragging with left mouse down as a Shift+left click.
      if ((wParam & MK_LBUTTON) == 0 ||
        (wParam & MK_SHIFT) || (wParam & MK_CONTROL))
        break;
      wParam = MK_SHIFT;
    }

    // Alt+click on a world map chart means relocate the chart there.
    if (wMsg == WM_LBUTTONDOWN && GetKeyState(VK_MENU) < 0) {
      if (fMap && !gs.fConstel && !gs.fMollweide) {
        Lon = rDegHalf -
          Mod((real)(x-gi.xOffset) / (real)gs.xWin*rDegMax - gs.rRot);
        if (Lon < -rDegHalf)
          Lon = -rDegHalf;
        else if (Lon > rDegHalf)
          Lon = rDegHalf;
        Lat = rDegQuad-(real)(y-gi.yOffset)/(real)gs.yWin*rDegHalf;
        if (Lat < -rDegQuad)
          Lat = -rDegQuad;
        else if (Lat > rDegQuad)
          Lat = rDegQuad;
        wi.xMouse = -1;
        ciCore = ciMain;
        wi.fCast = fTrue;
        ProcessState();
      }
      break;
    }

    // Ctrl+click means draw a rectangle. Ctrl+Shift+click does ellipse.
    if (wParam & MK_CONTROL)
      ScribbleDraw((wParam & MK_SHIFT) ? 3 : 2, wi.xMouse, wi.yMouse, x, y);

    // Shift+click means draw a line from the last to current position.
    else if (wParam & MK_SHIFT) {
      if (wi.xMouse >= 0) {
        ScribbleDraw(1, wi.xMouse, wi.yMouse, x, y);
        if (wMsg == WM_MOUSEMOVE) {
          wi.xMouse = x; wi.yMouse = y;
        }
      }

    // A simple click means set a pixel and remember that location.
    } else {
      ScribbleDraw(0, x, y, x, y);
      wi.xMouse = x; wi.yMouse = y;
    }
    break;

  // The mouse has been right clicked on the window.
  case WM_RBUTTONDOWN:
    wi.fMoved = fFalse;
    if (us.fGraphics) {
      if (fMap || gi.nMode == gMidpoint || gi.nMode == gLocal ||
        gi.nMode == gSphere || gi.nMode == gGlobe || gi.nMode == gPolar ||
        gi.nMode == gTelescope)
        wi.lParamRC = lParam;
      // Most graphics modes show popup right when mouse button clicked.
      switch (gi.nMode) {
      case gWheel: case gHouse:
        DoPopup(!gs.fIndianWheel ? menuV : menuV2, wi.hwnd, lParam); break;
      case gGrid:      DoPopup(menuG,  wi.hwnd, lParam); break;
      case gHorizon:   DoPopup(menuZ,  wi.hwnd, lParam); break;
      case gOrbit:     DoPopup(menuS,  wi.hwnd, lParam); break;
      case gSector:    DoPopup(menuH,  wi.hwnd, lParam); break;
      case gCalendar:  DoPopup(menuK,  wi.hwnd, lParam); break;
      case gDisposit:  DoPopup(menuJ,  wi.hwnd, lParam); break;
      case gEsoteric:  DoPopup(menu7,  wi.hwnd, lParam); break;
      case gEphemeris: DoPopup(menuE,  wi.hwnd, lParam); break;
      case gRising:    DoPopup(menuZd, wi.hwnd, lParam); break;
      case gMoons:     DoPopup(menu8,  wi.hwnd, lParam); break;
      case gBiorhythm: DoPopup(menuY,  wi.hwnd, lParam); break;
      case gTraTraGra: case gTraNatGra:
        DoPopup(menuB, wi.hwnd, lParam); break;
      }
    } else {
      if (us.nRel == rcBiorhythm) DoPopup(menu_Y,  wi.hwnd, lParam);
      else if (us.fListing)       DoPopup(menu_V,  wi.hwnd, lParam);
      else if (us.fWheel)         DoPopup(menu_W,  wi.hwnd, lParam);
      else if (us.fGrid)          DoPopup(menu_G,  wi.hwnd, lParam);
      else if (us.fAspList)       DoPopup(menu_A,  wi.hwnd, lParam);
      else if (us.fMidpoint)      DoPopup(menu_M,  wi.hwnd, lParam);
      else if (us.fHorizon)       DoPopup(menu_Z,  wi.hwnd, lParam);
      else if (us.fOrbit)         DoPopup(menu_S,  wi.hwnd, lParam);
      else if (us.fSector)        DoPopup(menu_H,  wi.hwnd, lParam);
      else if (us.fCalendar)      DoPopup(menu_K,  wi.hwnd, lParam);
      else if (us.fInfluence)     DoPopup(menu_J,  wi.hwnd, lParam);
      else if (us.fEsoteric)      DoPopup(menu_7,  wi.hwnd, lParam);
      else if (us.fAstroGraph)    DoPopup(menu_L,  wi.hwnd, lParam);
      else if (us.fEphemeris)     DoPopup(menu_E,  wi.hwnd, lParam);
      else if (us.fArabic)        DoPopup(menu_P,  wi.hwnd, lParam);
      else if (us.fHorizonSearch) DoPopup(menu_I,  wi.hwnd, lParam);
      else if (us.fAtlasNear)     DoPopup(menu_N,  wi.hwnd, lParam);
      else if (us.fMoonChart)     DoPopup(menu_8,  wi.hwnd, lParam);
      else if (us.fExoTransit)    DoPopup(menu_Ux, wi.hwnd, lParam);
      else if (us.fInDay    || us.fTransit)    DoPopup(menu_D, wi.hwnd, lParam);
      else if (us.fInDayInf || us.fTransitInf) DoPopup(menu_T, wi.hwnd, lParam);
      else if (us.fInDayGra || us.fTransitGra) DoPopup(menu_B, wi.hwnd, lParam);
    }
    break;

  // The right mouse button was clicked, but has now been lifted.
  case WM_RBUTTONUP:
    if (!wi.fMoved && us.fGraphics) {
      // Some graphics modes can't show popup until mouse button lifted,
      // because can click and drag right mouse in window to rotate chart.
      switch (gi.nMode) {
      case gMidpoint:   DoPopup(menuM,  wi.hwnd, lParam); break;
      case gAstroGraph: DoPopup(menuL,  wi.hwnd, lParam); break;
      case gLocal:      DoPopup(menuN,  wi.hwnd, lParam); break;
      case gTelescope:  DoPopup(menuXZ, wi.hwnd, lParam); break;
      case gSphere:     DoPopup(menuXX, wi.hwnd, lParam); break;
      case gWorldMap: case gGlobe: case gPolar:
        DoPopup(menuXG, wi.hwnd, lParam); break;
      }
    }
    break;

  // A timer message is received at a defined regular interval.
  case WM_TIMER:
    if (gs.nAnim < 1 || gi.fPause)
      break;
    Animate(gs.nAnim, gi.nDir);  // Update chart if animation mode on.
    wi.fRedraw = fTrue;
    ProcessState();
    break;

  // The horizontal or vertical scrollbar was clicked on or used.
  case WM_HSCROLL:
  case WM_VSCROLL:
    n = (wMsg == WM_HSCROLL ? wi.xScroll : wi.yScroll);
    x = n;
    switch (LOWORD(wParam)) {
    case SB_TOP:           n = 0;               break;
    case SB_BOTTOM:        n = nScrollDiv;      break;
    case SB_LINEUP:        n--;                 break;
    case SB_LINEDOWN:      n++;                 break;
    case SB_PAGEUP:        n -= nScrollPage;    break;
    case SB_PAGEDOWN:      n += nScrollPage;    break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:    n = HIWORD(wParam);  break;
    default:
      return;
    }
    n = n < 0 ? 0 : (n > nScrollDiv ? nScrollDiv : n);
    if (n == x)
      break;
    if (wMsg == WM_HSCROLL) {
      wi.xScroll = n;
      SetScrollPos(wi.hwnd, SB_HORZ, n, fTrue);
    } else {
      wi.yScroll = n;
      SetScrollPos(wi.hwnd, SB_VERT, n, fTrue);
    }
    // Scrolling a graphics chart only moves it, while a text chart needs to
    // be redrawn starting at a different row or column.
    if (us.fGraphics && !g_fImgText)
      g_view->update();
    else {
      wi.fRedraw = fTrue;
      ProcessState();
    }
    break;

  // The window is being terminated. Clean up and free program data.
  case WM_CLOSE:
    DoClose();
    break;
  }
}


// Schedule FRedraw() to be called soon from the event loop. Several requests
// before it runs are combined into one redraw.

static void QtScheduleRedraw(void)
{
  if (g_fRedrawPending || g_fClosing || g_app == NULL)
    return;
  g_fRedrawPending = fTrue;
  QTimer::singleShot(0, []() {
    g_fRedrawPending = fFalse;
    if (g_fClosing)
      return;
    if (!(wi.fNoUpdate & 1))
      FRedraw();
    if (g_view != NULL)
      g_view->update();
  });
}


/*
******************************************************************************
** Drawing the Window.
******************************************************************************
*/

// Copy a temporary file the chart was saved to onto the clipboard. This is
// how the Edit / Copy commands work, like the Windows version.

static void CopyTempFileToClipboard(WORD wCmd)
{
  QClipboard *clip = QGuiApplication::clipboard();
  QFile file(szFileTemp);
  QByteArray ba;
  QImage img;
  QMimeData *mime;

  if (!file.open(QIODevice::ReadOnly))
    return;
  ba = file.readAll();
  file.close();
  if (wCmd == cmdCopyBitmap) {
    if (img.loadFromData(ba))
      clip->setImage(img);
  } else if (wCmd == cmdCopyText && us.fTextHTML) {
    mime = new QMimeData;
    mime->setHtml(QString::fromUtf8(ba));
    clip->setMimeData(mime);
  } else if (wCmd == cmdCopySVG) {
    mime = new QMimeData;
    mime->setData("image/svg+xml", ba);
    mime->setText(QString::fromUtf8(ba));
    clip->setMimeData(mime);
  } else if (wCmd == cmdCopyPicture) {
    mime = new QMimeData;
    mime->setData("image/x-wmf", ba);
    clip->setMimeData(mime);
  } else
    clip->setText(QsFromSz(ba.constData(), (int)ba.size()));
  unlink(szFileTemp);
}


// This important routine is the bottleneck to redraw the window and call into
// the program to draw or do action with a particular chart type. This is the
// Qt version of FRedraw() in wdriver.cpp. Graphics charts get drawn into the
// gi.bmp memory bitmap (see FActionX), and text charts get drawn with
// TextOut() calls into a window sized image.

flag API FRedraw(void)
{
  HCURSOR hcurPrev = NULL;
  QPainter paint;
  QFont font;
  QImage imgText;
  char szFile[cchSzDef];
  int i;
  flag fSmartText = fFalse, fAnsiColor = fFalse, fAnsiChar = fFalse,
    fSmartHTML = fFalse, fInverse = fFalse, fText;

  if (us.fSmartSave && (wi.wCmd == cmdSaveText || wi.wCmd == cmdCopyText)) {
    if (!us.fTextHTML)
      fSmartText = fTrue;
    else
      fSmartHTML = fTrue;
  }
  HourglassOn;
  wi.fSmoothZoom = fFalse;
  fText = !us.fGraphics;

  if (!fText) {
    // Set up a graphics chart.
    if (wi.fWindowChart) {
      gs.xWin = wi.xClient; gs.yWin = wi.yClient;
    }
    gi.nScale = gs.nScale/100;
    gi.nScaleText = gs.nScaleText/50;
    gi.kiCur = -1;
    wi.hdc = hdcNil;
  } else {
    // Set up a text chart, drawn into an image the size of the window.
    gi.xOffset = gi.yOffset = 0;
    imgText = QImage(wi.xClient > 0 ? wi.xClient : 1,
      wi.yClient > 0 ? wi.yClient : 1, QImage::Format_RGB32);
    i = gs.nScale/100;
    if (gs.nFontTxt > 0) {
      font = QFont(rgszFontName[gs.nFontTxt]);
      font.setPixelSize((3 + 3*i)*2 - (gs.nFontTxt == fiLucida ? 5 : 0));
    } else {
      font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
      font.setPixelSize(i < 2 ? 8 : (i < 3 ? 12 : (i < 4 ? 16 : 18)));
    }
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(fTrue);
    font.setBold(gs.fThick && gs.nFontTxt > 0);
    {
      QFontMetrics fm(font);
      wi.xChar = fm.horizontalAdvance(QLatin1Char('M'));
      wi.yChar = fm.lineSpacing();
    }
    paint.begin(&imgText);
    paint.setFont(font);
    g_dcWin.img = &imgText;
    g_dcWin.paint = &paint;
    g_dcWin.kvText = KvFromKi(kLtGrayA);
    g_dcWin.nBkMode = TRANSPARENT;
    wi.hdc = &g_dcWin;
    TextClearScreen();
    if (wi.wCmd == cmdCopyText && us.fTextHTML)
      is.nHTML = -1;
  }
  if (fSmartText) {
    fAnsiColor = us.fAnsiColor; fAnsiChar = us.fAnsiChar;
    us.fAnsiColor = fFalse; us.fAnsiChar = fFalse;
  } else if (fSmartHTML) {
    fInverse = gs.fInverse; fAnsiChar = us.fAnsiChar;
    gs.fInverse = fTrue; us.fAnsiChar = fFalse;
    InitColorPalette(1);
  }

  Action();    // Actually go and create the chart here.
  if (fQtScreenChart) {
    // FActionX() returned early (e.g. out of memory) while drawing.
    fQtScreenChart = fFalse;
    gs.ft = ftNone;
    gi.fFile = fFalse;
  }

  // Create the chart again if also autosaving chart to file.

  if (wi.fAutoSave && us.fGraphics &&
    gs.ft != (!wi.fAutoSaveWire ? ftBmp : ftWire)) {
    gs.ft = !wi.fAutoSaveWire ? ftBmp : ftWire;
    if (!wi.fAutoSaveNum)
      sprintf(szFile, "%s",
        !wi.fAutoSaveWire ? szFileAutoCore : "astrolog.dw");
    else {
      sprintf(szFile, "ast%05d.%s", wi.nAutoSaveNum,
        !wi.fAutoSaveWire ? (gs.chBmpMode != 'P' ? "bmp" : "png") : "dw");
      wi.nAutoSaveNum++;
    }
    FCloneSz(szFile, &gi.szFileOut);
    Action();
    FCloneSz(NULL, &gi.szFileOut);
    if (gs.ft == ftWire) {
      gs.xWin /= WIREMUL; gs.yWin /= WIREMUL; gs.nScale /= WIREMUL;
    }
    gs.ft = ftNone;
  }

  // Cleanup and keep the result to show in the window.

  if (fSmartText) {
    us.fAnsiColor = fAnsiColor; us.fAnsiChar = fAnsiChar;
  } else if (fSmartHTML) {
    gs.fInverse = fInverse; us.fAnsiChar = fAnsiChar;
    InitColorPalette(fInverse);
  }
  if (fText) {
    paint.end();
    g_dcWin.paint = NULL;
    g_dcWin.img = NULL;
    wi.hdc = hdcNil;
    if (is.szFileScreen == NULL) {
      g_imgChart = imgText;
      g_fImgText = fTrue;
    }
  } else if (gs.ft == ftNone && gi.bmp.rgb != NULL && is.szFileScreen == NULL)
  {
    // Copy the chart from the 24 bit bitmap the core program drew it in.
    g_imgChart = QImage(gi.bmp.rgb, gi.bmp.x, gi.bmp.y, gi.bmp.clRow << 2,
      QImage::Format_BGR888).copy();
    g_fImgText = fFalse;
  }
  HourglassOff;
#ifdef EXPRESS
  // Notify AstroExpression the screen has just been redrawn.
  if (!us.fExpOff && FSzSet(us.szExpDisp3))
    ParseExpression(us.szExpDisp3);
#endif

  // If all text was scrolled off the top of the screen, scroll up.
  // If all text was scrolled off the left of the screen, scroll left.

  if (fText && is.S == stdout) {
    if (is.cchRow - wi.yScroll * 10 < 0)
      PostMessage(wi.hwnd, WM_VSCROLL,
        LFromWW(SB_THUMBPOSITION, is.cchRow / 10 - 2), 0);
    if (is.cchColMax - wi.xScroll * 10 < 0)
      PostMessage(wi.hwnd, WM_HSCROLL,
        LFromWW(SB_THUMBPOSITION, is.cchColMax / 10 - 2), 0);
  }

  // Sometimes creating a chart means saving it to a file instead of drawing
  // it on screen. If were in file mode, cleanup things here.

  if (is.szFileScreen != NULL || gs.ft != ftNone) {
    FCloneSz(NULL, &is.szFileScreen);
    gs.ft = ftNone;
    if (wi.wCmd == cmdCopyText || wi.wCmd == cmdCopyBitmap ||
      wi.wCmd == cmdCopyPicture || wi.wCmd == cmdCopyPS ||
      wi.wCmd == cmdCopySVG || wi.wCmd == cmdCopyWire) {
      CopyTempFileToClipboard(wi.wCmd);
      wi.wCmd = 0;
    }
    // Draw the chart again on screen, since it was saved to a file instead.
    wi.fRedraw = fTrue;
    ProcessState();
  }
  if (fSmartText || fSmartHTML)
    wi.wCmd = 0;

  wi.fRedraw = fFalse;
  if (wi.fNoUpdate != 0)
    wi.fNoUpdate = fTrue;
  if (g_view != NULL)
    g_view->update();
  return fTrue;
}


// Clear and erase the entire text screen. For graphics charts this is done
// within the 24 bit bitmap, so this is only called for text charts.

void WinClearScreen(KI ki)
{
  QtDC *pdc = wi.hdc;

  if (pdc == NULL || pdc->img == NULL)
    return;
  if (pdc->paint != NULL)
    pdc->paint->fillRect(pdc->img->rect(), QcFromKv(KvFromKi(ki)));
  else
    pdc->img->fill(QcFromKv(KvFromKi(ki)));
}


// Change the size of the window so its client area is the dimensions of the
// current graphics chart. This is the Qt version of the routine in xscreen.

void ResizeWindowToChart()
{
  QSize sizeWin, sizeView;
  QRect rcScr;
  int x, y;

  if (!us.fGraphics || g_win == NULL || g_view == NULL)
    return;
  if (gs.xWin == 0)
    gs.xWin = DEFAULTX;
  if (gs.yWin == 0)
    gs.yWin = DEFAULTY;
  sizeWin = g_win->size();
  sizeView = g_view->size();
  x = gs.xWin + (gi.nMode == 0 ? (SIDESIZE * gi.nScaleText) >> 1 : 0) +
    (sizeWin.width() - sizeView.width());
  y = gs.yWin + (sizeWin.height() - sizeView.height());
  if (g_win->screen() != NULL) {
    rcScr = g_win->screen()->availableGeometry();
    if (x > rcScr.width())
      x = rcScr.width();
    if (y > rcScr.height())
      y = rcScr.height();
  }
  wi.fNotManual = fTrue;
  g_win->resize(x, y);
  wi.fNotManual = fFalse;
}


// Implements the Edit / Paste menu command: Load text chart info or a bitmap
// from the clipboard. This is the Qt version of the routine in wdriver.cpp.

flag FFilePaste(void)
{
  QClipboard *clip = QGuiApplication::clipboard();
  const QMimeData *mime = clip->mimeData();
  QImage img;
  QFile file(szFileTemp);
  QByteArray ba;
  flag fText;

  if (mime == NULL)
    return fFalse;
  if (mime->hasImage())
    fText = fFalse;
  else if (mime->hasText())
    fText = fTrue;
  else {
    PrintWarning("There is nothing on the clipboard to paste.");
    return fFalse;
  }
  if (fText) {
    ba = mime->text().toUtf8();
    if (!file.open(QIODevice::WriteOnly))
      return fFalse;
    file.write(ba);
    file.close();
    FInputData(szFileTemp);
  } else {
    img = qvariant_cast<QImage>(mime->imageData());
    if (img.isNull() || !img.save(szFileTemp, "BMP"))
      return fFalse;
    FLoadBmp(szFileTemp, &gi.bmpBack, fTrue);
  }
  unlink(szFileTemp);
  return fTrue;
}


/*
******************************************************************************
** Win32 Subset: Messages and Windows.
******************************************************************************
*/

LRESULT SendMessage(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  if (hwnd == wi.hwndMain)
    QtDispatch(wMsg, wParam, lParam);
  return 0;
}

BOOL PostMessage(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  if (g_app == NULL)
    return fFalse;
  QTimer::singleShot(0, [=]() {
    if (!g_fClosing)
      QtDispatch(wMsg, wParam, lParam);
  });
  return fTrue;
}

BOOL SetWindowText(HWND hwnd, LPCSTR sz)
{
  if (hwnd != NULL && hwnd->widget != NULL)
    hwnd->widget->setWindowTitle(QsFromSz(sz));
  return fTrue;
}

BOOL GetWindowRect(HWND hwnd, LPRECT prc)
{
  QRect rc;

  if (hwnd == GetDesktopWindow()) {
    rc = QGuiApplication::primaryScreen()->geometry();
  } else if (hwnd != NULL && hwnd->widget != NULL)
    rc = hwnd->widget->frameGeometry();
  prc->left = rc.left(); prc->top = rc.top();
  prc->right = rc.left() + rc.width(); prc->bottom = rc.top() + rc.height();
  return fTrue;
}

BOOL GetClientRect(HWND hwnd, LPRECT prc)
{
  prc->left = prc->top = 0;
  prc->right = wi.xClient; prc->bottom = wi.yClient;
  return fTrue;
}

// Note that on Wayland, applications can't position their own windows, so
// only the size will change there.

BOOL SetWindowPos(HWND hwnd, HWND hwndAfter, int x, int y, int cx, int cy,
  UINT uFlags)
{
  if (hwnd == NULL || hwnd->widget == NULL)
    return fFalse;
  if (hwnd == wi.hwndMain && wi.fWindowFull) {
    hwnd->widget->showFullScreen();
    return fTrue;
  }
  if (hwnd->widget->isFullScreen())
    hwnd->widget->showNormal();
  hwnd->widget->move(x, y);
  hwnd->widget->resize(cx, cy);
  return fTrue;
}

BOOL MoveWindow(HWND hwnd, int x, int y, int cx, int cy, BOOL fRepaint)
{
  return SetWindowPos(hwnd, NULL, x, y, cx, cy, 0);
}

BOOL ClientToScreen(HWND hwnd, LPPOINT ppt)
{
  QPoint pt(ppt->x, ppt->y);

  if (hwnd == wi.hwndMain && g_view != NULL)
    pt = g_view->mapToGlobal(pt);
  ppt->x = pt.x(); ppt->y = pt.y();
  return fTrue;
}

BOOL ScreenToClient(HWND hwnd, LPPOINT ppt)
{
  QPoint pt(ppt->x, ppt->y);

  if (hwnd == wi.hwndMain && g_view != NULL)
    pt = g_view->mapFromGlobal(pt);
  ppt->x = pt.x(); ppt->y = pt.y();
  return fTrue;
}

BOOL GetCursorPos(LPPOINT ppt)
{
  QPoint pt = QCursor::pos();

  ppt->x = pt.x(); ppt->y = pt.y();
  return fTrue;
}

HWND GetDesktopWindow(void)
{
  static QtWnd wndDesktop = {NULL, NULL};
  return &wndDesktop;
}

HWND GetTopWindow(HWND hwnd)
{
  return NULL;
}

int SetScrollPos(HWND hwnd, int nBar, int nPos, BOOL fRedraw)
{
  QScrollBar *sb = (nBar == SB_HORZ ? g_sbHorz : g_sbVert);

  if (sb == NULL)
    return 0;
  sb->blockSignals(fTrue);
  sb->setValue(nPos);
  sb->blockSignals(fFalse);
  return nPos;
}

BOOL SetScrollRange(HWND hwnd, int nBar, int nMin, int nMax, BOOL fRedraw)
{
  if (nBar == SB_HORZ || nBar == SB_BOTH)
    if (g_sbHorz != NULL)
      g_sbHorz->setRange(nMin, nMax);
  if (nBar == SB_VERT || nBar == SB_BOTH)
    if (g_sbVert != NULL)
      g_sbVert->setRange(nMin, nMax);
  return fTrue;
}

BOOL ShowScrollBar(HWND hwnd, int nBar, BOOL fShow)
{
  if ((nBar == SB_HORZ || nBar == SB_BOTH) && g_sbHorz != NULL)
    g_sbHorz->setVisible(fShow);
  if ((nBar == SB_VERT || nBar == SB_BOTH) && g_sbVert != NULL)
    g_sbVert->setVisible(fShow);
  return fTrue;
}

BOOL RedrawWindow(HWND hwnd, const RECT *prc, HRGN hrgn, UINT uFlags)
{
  QtScheduleRedraw();
  return fTrue;
}

BOOL InvalidateRect(HWND hwnd, const RECT *prc, BOOL fErase)
{
  QtScheduleRedraw();
  return fTrue;
}

BOOL ShowWindow(HWND hwnd, int nCmdShow)
{
  if (hwnd == NULL || hwnd->widget == NULL)
    return fFalse;
  hwnd->widget->setVisible(nCmdShow != SW_HIDE);
  return fTrue;
}

BOOL EnableWindow(HWND hwnd, BOOL fEnable)
{
  if (hwnd == NULL || hwnd->widget == NULL)
    return fFalse;
  hwnd->widget->setEnabled(fEnable);
  return fTrue;
}

HWND SetFocus(HWND hwnd)
{
  if (hwnd != NULL && hwnd->widget != NULL)
    hwnd->widget->setFocus();
  return hwnd;
}

BOOL DestroyWindow(HWND hwnd)
{
  return fTrue;
}

short GetKeyState(int vk)
{
  Qt::KeyboardModifiers km = QGuiApplication::queryKeyboardModifiers();

  switch (vk) {
  case VK_SHIFT:   return (km & Qt::ShiftModifier)   ? (short)0x8000 : 0;
  case VK_CONTROL: return (km & Qt::ControlModifier) ? (short)0x8000 : 0;
  case VK_MENU:    return (km & Qt::AltModifier)     ? (short)0x8000 : 0;
  }
  return 0;
}

HCURSOR LoadCursor(HINSTANCE hinst, LPCSTR id)
{
  return id == IDC_WAIT ? HCURSOR_WAIT : HCURSOR_ARROW;
}

// The hourglass macros call SetCursor(wait) and later SetCursor(previous).

HCURSOR SetCursor(HCURSOR hcur)
{
  if (g_app == NULL)
    return HCURSOR_ARROW;
  if (hcur == HCURSOR_WAIT) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    g_cWait++;
  } else if (g_cWait > 0) {
    QApplication::restoreOverrideCursor();
    g_cWait--;
  }
  return HCURSOR_ARROW;
}

int ShowCursor(BOOL fShow)
{
  if (g_view != NULL)
    g_view->setCursor(fShow ? Qt::ArrowCursor : Qt::BlankCursor);
  return fShow ? 0 : -1;
}

uintptr_t SetTimer(HWND hwnd, uintptr_t id, UINT ms, TIMERPROC pfn)
{
  if (g_timer == NULL)
    return 0;
  g_timer->start((int)(ms > 0 ? ms : 1));
  return id;
}

BOOL KillTimer(HWND hwnd, uintptr_t id)
{
  if (g_timer != NULL)
    g_timer->stop();
  return fTrue;
}

int MessageBox(HWND hwnd, LPCSTR sz, LPCSTR szTitle, UINT uType)
{
  QMessageBox::Icon qicon = QMessageBox::NoIcon;
  QWidget *parent = QApplication::activeModalWidget();
  QMessageBox::StandardButtons sb = QMessageBox::Ok;
  int n;

  switch (uType & 0xf0) {
  case MB_ICONSTOP:        qicon = QMessageBox::Critical;    break;
  case MB_ICONQUESTION:    qicon = QMessageBox::Question;    break;
  case MB_ICONEXCLAMATION: qicon = QMessageBox::Warning;     break;
  case MB_ICONINFORMATION: qicon = QMessageBox::Information; break;
  }
  if ((uType & 0xf) == MB_OKCANCEL)
    sb = QMessageBox::Ok | QMessageBox::Cancel;
  else if ((uType & 0xf) == MB_YESNO)
    sb = QMessageBox::Yes | QMessageBox::No;
  if (g_app == NULL) {
    fprintf(stderr, "%s: %s\n", szTitle, sz);
    return IDOK;
  }
  if (parent == NULL && hwnd != NULL)
    parent = hwnd->widget;
  QMessageBox mb(qicon, QsFromSz(szTitle), QsFromSz(sz), sb, parent);
  n = mb.exec();
  return n == QMessageBox::Cancel ? IDCANCEL : (n == QMessageBox::Yes ? IDYES :
    (n == QMessageBox::No ? IDNO : IDOK));
}

BOOL MessageBeep(UINT uType)
{
  QApplication::beep();
  return fTrue;
}

void PostQuitMessage(int nExit)
{
  if (g_app != NULL)
    QTimer::singleShot(0, []() { QApplication::quit(); });
}


/*
******************************************************************************
** Win32 Subset: Menus.
******************************************************************************
*/

// Menus are built from the MENU resources in astrolog.rc, via the rgQtMenu
// table generated into rcqt.inc. Each menu item's QAction stores its command
// id, and is found again through the root's mapCmd for the functions below.

// Build the menu tree for the given menu resource. If mb is given, then the
// top level popups are added to that menu bar (for the main menu).

static QtMenu *PmenuBuild(int idMenu, QMenuBar *mb)
{
  QtMenu *root = new QtMenu, *node, *sub;
  std::vector<QtMenu *> stack;
  QAction *a;
  QMenu *qm;
  CONST QtMenuEntry *pme;
  QtMenuAction ma;

  root->qmenu = NULL;
  root->root = root;
  root->rgAll.push_back(root);
  stack.push_back(root);
  for (pme = rgQtMenu; pme->idMenu != 0; pme++) {
    if (pme->idMenu != idMenu)
      continue;
    node = stack.back();
    switch (pme->nKind) {
    case QTMENU_POPUP:
      sub = new QtMenu;
      sub->root = root;
      if (node->qmenu != NULL)
        qm = node->qmenu->addMenu(QString::fromUtf8(pme->sz));
      else if (mb != NULL)
        qm = mb->addMenu(QString::fromUtf8(pme->sz));
      else
        qm = new QMenu(QString::fromUtf8(pme->sz), g_win);
      sub->qmenu = qm;
      node->rgSub.push_back(sub);
      root->rgAll.push_back(sub);
      stack.push_back(sub);
      break;
    case QTMENU_END:
      if (stack.size() > 1)
        stack.pop_back();
      break;
    case QTMENU_SEP:
      if (node->qmenu != NULL)
        node->qmenu->addSeparator();
      node->rgSub.push_back(NULL);
      break;
    default:
      if (node->qmenu == NULL)
        break;
      a = node->qmenu->addAction(QString::fromUtf8(pme->sz));
      a->setData(pme->cmd);
      ma.action = a; ma.parent = node->qmenu;
      root->mapCmd.insert(std::make_pair((UINT)pme->cmd, ma));
      node->rgSub.push_back(NULL);
      // Qt toggles checkable actions when triggered, but Windows doesn't:
      // The command itself sets the new check state, so undo the toggle.
      if (mb != NULL)
        QObject::connect(a, &QAction::triggered, [a](bool fChecked) {
          if (a->isCheckable()) {
            a->blockSignals(fTrue);
            a->setChecked(!fChecked);
            a->blockSignals(fFalse);
          }
          QtDispatch(WM_COMMAND, (WPARAM)a->data().toInt(), 0);
        });
      break;
    }
  }
  return root;
}


// Hide any submenus that have become empty, e.g. after DeleteMenu().

static void HideEmptyMenus(QtMenu *root)
{
  QList<QAction *> list;
  size_t i;
  flag fAny;
  int j;

  // Go in reverse, so that nested submenus are checked before their parents.
  for (i = root->rgAll.size(); i-- > 0;) {
    if (root->rgAll[i]->qmenu == NULL)
      continue;
    list = root->rgAll[i]->qmenu->actions();
    fAny = fFalse;
    for (j = 0; j < list.size(); j++)
      if (!list[j]->isSeparator() && list[j]->isVisible())
        fAny = fTrue;
    root->rgAll[i]->qmenu->menuAction()->setVisible(fAny);
  }
}


// Return the action at the given position within a menu, counting items,
// separators, and submenus like Windows does.

static QAction *PactionFromPos(HMENU hmenu, UINT iPos)
{
  QList<QAction *> list;

  if (hmenu == NULL || hmenu->qmenu == NULL)
    return NULL;
  list = hmenu->qmenu->actions();
  return iPos < (UINT)list.size() ? list[iPos] : NULL;
}


HMENU LoadMenu(HINSTANCE hinst, LPCSTR id)
{
  if ((intptr_t)id == menu)
    return g_pmenuMain;
  if (g_win == NULL)
    return NULL;
  return PmenuBuild((int)(intptr_t)id, NULL);
}

HMENU GetSubMenu(HMENU hmenu, int i)
{
  if (hmenu == NULL || i < 0 || i >= (int)hmenu->rgSub.size())
    return NULL;
  return hmenu->rgSub[i];
}

// Free a context menu tree loaded with LoadMenu(). The main menu stays.

BOOL DestroyMenu(HMENU hmenu)
{
  QtMenu *root;
  size_t i;

  if (hmenu == NULL || hmenu->root == g_pmenuMain)
    return fFalse;
  root = hmenu->root;
  for (i = 0; i < root->rgSub.size(); i++)
    if (root->rgSub[i] != NULL && root->rgSub[i]->qmenu != NULL)
      root->rgSub[i]->qmenu->deleteLater();  // Deletes nested ones too.
  for (i = 0; i < root->rgAll.size(); i++)
    if (root->rgAll[i] != root)
      delete root->rgAll[i];
  delete root;
  return fTrue;
}

BOOL DrawMenuBar(HWND hwnd)
{
  return fTrue;
}

DWORD CheckMenuItem(HMENU hmenu, UINT id, UINT uCheck)
{
  std::multimap<UINT, QtMenuAction>::iterator it, itEnd;
  DWORD dwOld = (DWORD)-1;

  if (hmenu == NULL)
    return dwOld;
  std::tie(it, itEnd) = hmenu->root->mapCmd.equal_range(id);
  for (; it != itEnd; it++) {
    if (dwOld == (DWORD)-1)
      dwOld = it->second.action->isChecked() ? MF_CHECKED : MF_UNCHECKED;
    it->second.action->setCheckable(fTrue);
    it->second.action->setChecked((uCheck & MF_CHECKED) != 0);
  }
  return dwOld;
}

BOOL CheckMenuRadioItem(HMENU hmenu, UINT idFirst, UINT idLast, UINT idCheck,
  UINT uFlags)
{
  std::multimap<UINT, QtMenuAction>::iterator it, itEnd;

  if (hmenu == NULL)
    return fFalse;
  it = hmenu->root->mapCmd.lower_bound(idFirst);
  itEnd = hmenu->root->mapCmd.upper_bound(idLast);
  for (; it != itEnd; it++) {
    it->second.action->setCheckable(fTrue);
    it->second.action->setChecked(it->first == idCheck);
  }
  return fTrue;
}

BOOL EnableMenuItem(HMENU hmenu, UINT id, UINT uEnable)
{
  std::multimap<UINT, QtMenuAction>::iterator it, itEnd;
  QAction *a;

  if (hmenu == NULL)
    return fFalse;
  if (uEnable & MF_BYPOSITION) {
    a = PactionFromPos(hmenu, id);
    if (a != NULL)
      a->setEnabled(!(uEnable & (MF_GRAYED | MF_DISABLED)));
    return a != NULL;
  }
  std::tie(it, itEnd) = hmenu->root->mapCmd.equal_range(id);
  for (; it != itEnd; it++)
    it->second.action->setEnabled(!(uEnable & (MF_GRAYED | MF_DISABLED)));
  return fTrue;
}

// Change the text of a menu item, e.g. to give a macro a custom name.

BOOL ModifyMenu(HMENU hmenu, UINT id, UINT uFlags, uintptr_t idNew,
  LPCSTR sz)
{
  std::multimap<UINT, QtMenuAction>::iterator it, itEnd;
  QString qs = QsFromSz(sz);
  QAction *a;

  if (hmenu == NULL)
    return fFalse;
  if (uFlags & MF_BYPOSITION) {
    a = PactionFromPos(hmenu, id);
    if (a != NULL)
      a->setText(qs);
    return a != NULL;
  }
  std::tie(it, itEnd) = hmenu->root->mapCmd.equal_range(id);
  for (; it != itEnd; it++)
    it->second.action->setText(qs);
  return fTrue;
}

BOOL DeleteMenu(HMENU hmenu, UINT id, UINT uFlags)
{
  std::multimap<UINT, QtMenuAction>::iterator it, itEnd;
  QAction *a;

  if (hmenu == NULL)
    return fFalse;
  if (uFlags & MF_BYPOSITION) {
    a = PactionFromPos(hmenu, id);
    if (a != NULL)
      a->setVisible(fFalse);
    return a != NULL;
  }
  std::tie(it, itEnd) = hmenu->root->mapCmd.equal_range(id);
  for (; it != itEnd; it++) {
    it->second.parent->removeAction(it->second.action);
    it->second.action->deleteLater();
  }
  hmenu->root->mapCmd.erase(id);
  HideEmptyMenus(hmenu->root);
  return fTrue;
}

int GetMenuString(HMENU hmenu, UINT id, LPSTR sz, int cchMax, UINT uFlags)
{
  std::multimap<UINT, QtMenuAction>::iterator it;
  QAction *a = NULL;

  if (cchMax <= 0)
    return 0;
  *sz = chNull;
  if (hmenu == NULL)
    return 0;
  if (uFlags & MF_BYPOSITION)
    a = PactionFromPos(hmenu, id);
  else {
    it = hmenu->root->mapCmd.find(id);
    if (it != hmenu->root->mapCmd.end())
      a = it->second.action;
  }
  if (a == NULL)
    return 0;
  CopyQsToSz(a->text(), sz, cchMax);
  return (int)CchSz(sz);
}

// Show a context menu at a screen position. Like Windows, the chosen command
// is posted to the window as a WM_COMMAND message.

BOOL TrackPopupMenu(HMENU hmenu, UINT uFlags, int x, int y, int nReserved,
  HWND hwnd, const RECT *prc)
{
  QAction *a;

  if (hmenu == NULL || hmenu->qmenu == NULL)
    return fFalse;
  a = hmenu->qmenu->exec(QPoint(x, y));
  if (a == NULL || !a->data().isValid())
    return fFalse;
  PostMessage(hwnd, WM_COMMAND, (WPARAM)a->data().toInt(), 0);
  return fTrue;
}


// Create the main menu bar, and remove the commands not available in this
// build. This follows the same in WinMain() in wdriver.cpp, plus removes the
// commands that only make sense on Windows.

static void CreateMainMenu(void)
{
  g_pmenuMain = PmenuBuild(menu, g_win->menuBar());
  wi.hmenu = g_pmenuMain;
#ifndef PS
  DeleteMenu(wi.hmenu, cmdSavePS, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyPS, MF_BYCOMMAND);
#endif
#ifndef META
  DeleteMenu(wi.hmenu, cmdSavePicture, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyPicture, MF_BYCOMMAND);
#endif
#ifndef SVG
  DeleteMenu(wi.hmenu, cmdSaveSVG, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopySVG, MF_BYCOMMAND);
#endif
#ifndef WIRE
  DeleteMenu(wi.hmenu, cmdSaveWire, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyWire, MF_BYCOMMAND);
#endif
#ifndef INTERPRET
  DeleteMenu(wi.hmenu, cmdInterpret,   MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdHelpMeaning, MF_BYCOMMAND);
#endif
#ifndef ARABIC
  DeleteMenu(wi.hmenu, cmdChartArabic, MF_BYCOMMAND);
#endif
#ifndef CONSTEL
  DeleteMenu(wi.hmenu, cmdConstellation,     MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdGraphicsStarLine,  MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdHelpConstellation, MF_BYCOMMAND);
#endif
#ifndef BIORHYTHM
  DeleteMenu(wi.hmenu, cmdRelBiorhythm, MF_BYCOMMAND);
#endif
#ifndef ATLAS
  DeleteMenu(wi.hmenu, cmdGraphicsCity, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdDocAtlas,     MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdDocTimezone,  MF_BYCOMMAND);
#endif
  // Windows only setup, folder browsing, wallpaper, and print setup.
  DeleteMenu(wi.hmenu, cmdOpenDir,         MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSetupUser,       MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSetupAll,        MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSetupDesktop,    MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSetupExtension,  MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdUnsetup,         MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSaveWallTile,    MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSaveWallCenter,  MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSaveWallStretch, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSaveWallFit,     MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdSaveWallFill,    MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdPrintSetup,      MF_BYCOMMAND);
  wi.hmenuEdit = GetSubMenu(wi.hmenu, 1);
}


/*
******************************************************************************
** Win32 Subset: Dialogs.
******************************************************************************
*/

// Dialogs are built from the DIALOG resources in astrolog.rc, via the
// rgQtDlg table generated into rcqt.inc, then run with the unchanged dialog
// procedures in wdialog.cpp, which get WM_INITDIALOG and WM_COMMAND messages
// like on Windows. Controls are placed at the same positions, converting
// dialog units to pixels from the dialog font, like Win32 MapDialogRect().
// Like the Windows version, dialog text is in the Latin-1 character set.

// The dialog window, which sends Escape and the close box to the dialog
// procedure as IDCANCEL, like Windows does.

class AstroDialog : public QDialog
{
public:
  QtDlgState *pds;

  AstroDialog(QWidget *parent) : QDialog(parent), pds(NULL) {}

protected:
  void reject() override;
};


// Find a file that comes with the program, such as an icon, returning its
// full path, or an empty string if it can't be found.

static QString QsFindProgramFile(CONST char *szFile)
{
  char szPath[cchSzMax];
  FILE *file;
  QString qs;

  if (szFile == NULL || !*szFile)
    return QString();
  qs = QDir(QCoreApplication::applicationDirPath()).filePath(
    QString::fromLatin1(szFile));
  if (QFileInfo::exists(qs))
    return qs;
  file = FileOpen(szFile, 2, szPath);
  if (file == NULL)
    return QString();
  fclose(file);
  return QString::fromLocal8Bit(szPath);
}


// Send a WM_COMMAND message to a dialog's procedure.

static void SendDlgCommand(QtDlgState *pds, int id)
{
  if (pds == NULL || pds->fEnded || pds->pfn == NULL)
    return;
  (*pds->pfn)(&pds->wnd, WM_COMMAND, (WORD)id, 0);
}

void AstroDialog::reject()
{
  if (pds != NULL && !pds->fEnded)
    SendDlgCommand(pds, IDCANCEL);
}


// Remove the '&' mnemonic markers from a string, leaving any "&&" as "&".

static QString QsStripMnemonic(const QString &qs)
{
  QString qsOut;
  int i;

  for (i = 0; i < qs.size(); i++) {
    if (qs[i] == '&') {
      if (i+1 < qs.size() && qs[i+1] == '&')
        qsOut += '&';
      else
        continue;
      i++;
    } else
      qsOut += qs[i];
  }
  return qsOut;
}


// Create the dialog window and all its controls from the resource table.
// Return fFalse if the dialog isn't in the table.

static flag FBuildDialog(QtDlgState *pds, int idDlg)
{
  CONST QtDlgEntry *pde, *pdeDlg = NULL;
  AstroDialog *dlg;
  QWidget *w;
  QLabel *label;
  QComboBox *combo;
  QPushButton *button;
  QButtonGroup *group = NULL;
  QtWnd *pwnd;
  QString qs, qsFile;
  std::vector<std::pair<QLabel *, size_t> > rgLabel;
  std::vector<QWidget *> rgInput;
  real bx, by;
  int x, y, cx, cy, nGroup = 0, nGroupRadio = -1, id;
  size_t i;

  for (pde = rgQtDlg; pde->idDlg != 0; pde++)
    if (pde->idDlg == idDlg && pde->nKind == QTDLG_DIALOG) {
      pdeDlg = pde;
      break;
    }
  if (pdeDlg == NULL)
    return fFalse;

  dlg = new AstroDialog(QApplication::activeModalWidget() != NULL ?
    QApplication::activeModalWidget() : (QWidget *)g_win);
  dlg->pds = pds;
  pds->dlg = dlg;
  pds->wnd.widget = dlg;
  pds->wnd.pds = pds;
  dlg->setWindowTitle(QString::fromUtf8(pdeDlg->sz));

  // Dialog base units, as Windows computes them from the dialog font.
  {
    QFontMetrics fm(dlg->font());
    bx = (real)fm.horizontalAdvance(QString(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")) / 52.0;
    by = (real)fm.height();
  }
#define XDlg(n) ((int)((real)(n) * bx / 4.0 + 0.5))
#define YDlg(n) ((int)((real)(n) * by / 8.0 + 0.5))
  dlg->setFixedSize(XDlg(pdeDlg->cx), YDlg(pdeDlg->cy));

  for (pde = pdeDlg + 1; pde->idDlg == idDlg && pde->nKind != QTDLG_DIALOG;
    pde++) {
    x = XDlg(pde->x); y = YDlg(pde->y);
    cx = XDlg(pde->cx); cy = YDlg(pde->cy);
    qs = QString::fromUtf8(pde->sz);
    if (pde->nFlags & QTDLG_FGROUP)
      nGroup++;
    w = NULL;
    switch (pde->nKind) {
    case QTDLG_LTEXT:
    case QTDLG_RTEXT:
    case QTDLG_CTEXT:
    case QTDLG_STATIC:
      label = new QLabel(dlg);
      label->setText(qs);
      label->setWordWrap(fTrue);
      label->setAlignment(Qt::AlignTop | (pde->nKind == QTDLG_RTEXT ?
        Qt::AlignRight : (pde->nKind == QTDLG_CTEXT ? Qt::AlignHCenter :
        Qt::AlignLeft)));
      rgLabel.push_back(std::make_pair(label, rgInput.size()));
      w = label;
      break;
    case QTDLG_EDIT:
      w = new QLineEdit(dlg);
      if (pde->nFlags & QTDLG_FREADONLY)
        ((QLineEdit *)w)->setReadOnly(fTrue);
      if (pde->nFlags & QTDLG_FPASSWORD)
        ((QLineEdit *)w)->setEchoMode(QLineEdit::Password);
      rgInput.push_back(w);
      break;
    case QTDLG_COMBO:
      combo = new QComboBox(dlg);
      combo->setEditable(!(pde->nFlags & QTDLG_FDROPLIST));
      combo->setInsertPolicy(QComboBox::NoInsert);
      combo->setMaxVisibleItems(20);
      // The resource height is that of the dropdown list. The control
      // itself is as tall as a normal edit field.
      cy = combo->sizeHint().height();
      w = combo;
      rgInput.push_back(w);
      break;
    case QTDLG_LIST:
      w = new QListWidget(dlg);
      if (pde->nFlags & QTDLG_FSORT)
        ((QListWidget *)w)->setSortingEnabled(fTrue);
      rgInput.push_back(w);
      break;
    case QTDLG_BUTTON:
    case QTDLG_DEFBUTTON:
      button = new QPushButton(qs, dlg);
      button->setAutoDefault(fFalse);
      if (pde->nKind == QTDLG_DEFBUTTON) {
        button->setAutoDefault(fTrue);
        button->setDefault(fTrue);
      }
      w = button;
      break;
    case QTDLG_GROUPBOX:
      w = new QGroupBox(qs, dlg);
      break;
    case QTDLG_ICON:
      label = new QLabel(dlg);
      qsFile = QsFindProgramFile(pde->sz);
      if (!qsFile.isEmpty())
        label->setPixmap(QIcon(qsFile).pixmap(32, 32));
      cx = cy = 32;
      w = label;
      break;
    case QTDLG_CHECK:
      w = new QCheckBox(qs, dlg);
      break;
    case QTDLG_RADIO:
      // Like Windows, radio buttons are exclusive within a group that starts
      // at a control with the WS_GROUP style.
      if (group == NULL || nGroupRadio != nGroup) {
        group = new QButtonGroup(dlg);
        group->setExclusive(fTrue);
        nGroupRadio = nGroup;
      }
      w = new QRadioButton(qs, dlg);
      group->addButton((QRadioButton *)w);
      break;
    }
    if (w == NULL)
      continue;
    w->setGeometry(x, y, cx, cy);
    if (pde->nFlags & QTDLG_FDISABLED)
      w->setEnabled(fFalse);
    if (pde->nFlags & QTDLG_FHIDDEN)
      w->setVisible(fFalse);
    if (pde->nKind == QTDLG_GROUPBOX)
      w->lower();  // Group boxes go behind the controls within them.

    // Buttons, checkboxes, and radio buttons send WM_COMMAND when clicked.
    id = pde->id;
    if (pde->nKind == QTDLG_BUTTON || pde->nKind == QTDLG_DEFBUTTON ||
      pde->nKind == QTDLG_CHECK || pde->nKind == QTDLG_RADIO)
      QObject::connect((QAbstractButton *)w, &QAbstractButton::clicked,
        [pds, id](bool) { SendDlgCommand(pds, id); });

    // Remember the control by id. Static text with no id isn't remembered.
    if (id != -1 && pds->mapCtl.find(id) == pds->mapCtl.end()) {
      pwnd = new QtWnd;
      pwnd->widget = w;
      pwnd->pds = pds;
      pds->mapCtl[id] = pwnd;
    }
  }

  // A label with a mnemonic gives focus to the next input control, like on
  // Windows. Otherwise the '&' marker is removed from the label text.
  for (i = 0; i < rgLabel.size(); i++) {
    qs = rgLabel[i].first->text();
    if (QsStripMnemonic(qs) == qs)
      continue;
    if (rgLabel[i].second < rgInput.size())
      rgLabel[i].first->setBuddy(rgInput[rgLabel[i].second]);
    else
      rgLabel[i].first->setText(QsStripMnemonic(qs));
  }
#undef XDlg
#undef YDlg
  return fTrue;
}


// Free the dialog window and the control handles.

static void FreeDialog(QtDlgState *pds)
{
  std::map<int, QtWnd *>::iterator it;

  for (it = pds->mapCtl.begin(); it != pds->mapCtl.end(); it++)
    delete it->second;
  pds->mapCtl.clear();
  if (pds->dlg != NULL) {
    pds->dlg->pds = NULL;
    delete pds->dlg;
    pds->dlg = NULL;
  }
}


// Run a modal dialog with the given resource id and dialog procedure, and
// return the value the procedure passed to EndDialog().

intptr_t DialogBox(HINSTANCE hinst, LPCSTR id, HWND hwndParent,
  DLGPROC pfn)
{
  QtDlgState ds;
  intptr_t nResult;

  ds.dlg = NULL;
  ds.pfn = pfn;
  ds.fEnded = fFalse;
  ds.nResult = 0;
  ds.wnd.widget = NULL;
  ds.wnd.pds = &ds;
  if (!FBuildDialog(&ds, (int)(intptr_t)id)) {
    PrintWarning("This dialog isn't available in the Qt version.");
    return -1;
  }

  // Let the dialog procedure fill in the controls. If it returns true, then
  // focus goes on the first control (Qt does that when the dialog is shown),
  // otherwise the procedure set the focus itself.
  (*pfn)(&ds.wnd, WM_INITDIALOG, 0, 0);
  if (!ds.fEnded)
    ds.dlg->exec();
  nResult = ds.nResult;
  FreeDialog(&ds);
  return nResult;
}

BOOL EndDialog(HWND hdlg, intptr_t nResult)
{
  QtDlgState *pds;

  if (hdlg == NULL || (pds = hdlg->pds) == NULL || pds->dlg == NULL)
    return fFalse;
  pds->fEnded = fTrue;
  pds->nResult = nResult;
  pds->dlg->done((int)nResult);
  return fTrue;
}

HWND GetDlgItem(HWND hdlg, int id)
{
  std::map<int, QtWnd *>::iterator it;

  if (hdlg == NULL || hdlg->pds == NULL)
    return NULL;
  it = hdlg->pds->mapCtl.find(id);
  return it != hdlg->pds->mapCtl.end() ? it->second : NULL;
}

// Return a dialog's control as the given Qt widget class, or NULL.

template <class T> static T *PctlFromId(HWND hdlg, int id)
{
  HWND hwnd = GetDlgItem(hdlg, id);

  return hwnd != NULL ? dynamic_cast<T *>(hwnd->widget) : NULL;
}

BOOL SetDlgItemText(HWND hdlg, int id, LPCSTR sz)
{
  QWidget *w;
  QString qs = QString::fromLatin1(sz != NULL ? sz : "");

  if (GetDlgItem(hdlg, id) == NULL)
    return fFalse;
  w = GetDlgItem(hdlg, id)->widget;
  if (QLineEdit *e = dynamic_cast<QLineEdit *>(w))
    e->setText(qs);
  else if (QComboBox *c = dynamic_cast<QComboBox *>(w)) {
    if (c->isEditable())
      c->setEditText(qs);
    else
      c->setCurrentIndex(c->findText(qs));
  } else if (QLabel *l = dynamic_cast<QLabel *>(w))
    l->setText(qs);
  else if (QAbstractButton *b = dynamic_cast<QAbstractButton *>(w))
    b->setText(qs);
  else if (QGroupBox *g = dynamic_cast<QGroupBox *>(w))
    g->setTitle(qs);
  else
    return fFalse;
  return fTrue;
}

UINT GetDlgItemText(HWND hdlg, int id, LPSTR sz, int cchMax)
{
  QWidget *w;
  QString qs;
  QByteArray ba;
  int cch;

  if (cchMax <= 0)
    return 0;
  *sz = chNull;
  if (GetDlgItem(hdlg, id) == NULL)
    return 0;
  w = GetDlgItem(hdlg, id)->widget;
  if (QLineEdit *e = dynamic_cast<QLineEdit *>(w))
    qs = e->text();
  else if (QComboBox *c = dynamic_cast<QComboBox *>(w))
    qs = c->currentText();
  else if (QLabel *l = dynamic_cast<QLabel *>(w))
    qs = l->text();
  else if (QAbstractButton *b = dynamic_cast<QAbstractButton *>(w))
    qs = b->text();
  else if (QGroupBox *g = dynamic_cast<QGroupBox *>(w))
    qs = g->title();
  ba = qs.toLatin1();
  cch = (int)ba.size();
  if (cch > cchMax-1)
    cch = cchMax-1;
  memcpy(sz, ba.constData(), cch);
  sz[cch] = chNull;
  return (UINT)cch;
}

BOOL SetDlgItemInt(HWND hdlg, int id, UINT n, BOOL fSigned)
{
  char sz[cchSzDef];

  if (fSigned)
    sprintf(sz, "%d", (int)n);
  else
    sprintf(sz, "%u", n);
  return SetDlgItemText(hdlg, id, sz);
}

BOOL CheckDlgButton(HWND hdlg, int id, UINT uCheck)
{
  QAbstractButton *b = PctlFromId<QAbstractButton>(hdlg, id);
  QButtonGroup *group;
  flag fExclusive = fFalse;

  if (b == NULL)
    return fFalse;
  // An exclusive radio group can't have its checked button unchecked
  // directly, so allow that temporarily, like Windows does.
  group = b->group();
  if (group != NULL && uCheck == BST_UNCHECKED && b->isChecked()) {
    fExclusive = group->exclusive();
    group->setExclusive(fFalse);
  }
  b->setChecked(uCheck != BST_UNCHECKED);
  if (fExclusive)
    group->setExclusive(fTrue);
  return fTrue;
}

UINT IsDlgButtonChecked(HWND hdlg, int id)
{
  QAbstractButton *b = PctlFromId<QAbstractButton>(hdlg, id);

  return b != NULL && b->isChecked() ? BST_CHECKED : BST_UNCHECKED;
}

BOOL CheckRadioButton(HWND hdlg, int idFirst, int idLast, int idCheck)
{
  int id;

  for (id = idFirst; id <= idLast; id++)
    if (id != idCheck)
      CheckDlgButton(hdlg, id, BST_UNCHECKED);
  return CheckDlgButton(hdlg, idCheck, BST_CHECKED);
}

// Process the combobox and listbox messages Astrolog's dialogs use.

LRESULT SendDlgItemMessage(HWND hdlg, int id, UINT wMsg, WPARAM wParam,
  LPARAM lParam)
{
  QComboBox *c = PctlFromId<QComboBox>(hdlg, id);
  QListWidget *l = PctlFromId<QListWidget>(hdlg, id);
  QListWidgetItem *item;
  QString qs;
  int i;

  if (c != NULL) {
    switch (wMsg) {
    case CB_ADDSTRING:
    case CB_INSERTSTRING:
      // Adding the first item to a combobox would replace its edit text, so
      // keep the text, like Windows.
      qs = c->currentText();
      c->blockSignals(fTrue);
      i = (wMsg == CB_ADDSTRING ? c->count() :
        ((int)wParam < 0 ? c->count() : (int)wParam));
      c->insertItem(i, QString::fromLatin1((CONST char *)lParam));
      if (c->isEditable())
        c->setEditText(qs);
      c->blockSignals(fFalse);
      return i;
    case CB_DELETESTRING:
      c->removeItem((int)wParam);
      return c->count();
    case CB_RESETCONTENT:
      c->clear();
      return 0;
    case CB_GETCOUNT:
      return c->count();
    case CB_GETCURSEL:
      return c->currentIndex();
    case CB_SETCURSEL:
      c->setCurrentIndex((int)wParam);
      return c->currentIndex();
    case CB_FINDSTRING:
      return c->findText(QString::fromLatin1((CONST char *)lParam),
        Qt::MatchStartsWith);
    case CB_SELECTSTRING:
      i = c->findText(QString::fromLatin1((CONST char *)lParam),
        Qt::MatchStartsWith);
      if (i >= 0)
        c->setCurrentIndex(i);
      return i;
    case CB_GETLBTEXT:
      if ((int)wParam < 0 || (int)wParam >= c->count())
        return CB_ERR;
      qs = c->itemText((int)wParam);
      strcpy((char *)lParam, qs.toLatin1().constData());
      return qs.size();
    }
    return 0;
  }
  if (l != NULL) {
    switch (wMsg) {
    case LB_ADDSTRING:
    case LB_INSERTSTRING:
      i = (wMsg == LB_ADDSTRING ? l->count() :
        ((int)wParam < 0 ? l->count() : (int)wParam));
      l->insertItem(i, QString::fromLatin1((CONST char *)lParam));
      return l->row(l->item(i));
    case LB_DELETESTRING:
      delete l->takeItem((int)wParam);
      return l->count();
    case LB_RESETCONTENT:
      l->clear();
      return 0;
    case LB_GETCOUNT:
      return l->count();
    case LB_GETCURSEL:
      return l->currentItem() != NULL && l->currentItem()->isSelected() ?
        l->currentRow() : LB_ERR;
    case LB_SETCURSEL:
      if ((int)wParam < 0) {
        l->clearSelection();
        return LB_ERR;
      }
      l->setCurrentRow((int)wParam);
      if (l->currentItem() != NULL)
        l->scrollToItem(l->currentItem());
      return l->currentRow();
    case LB_GETITEMDATA:
      item = l->item((int)wParam);
      return item != NULL ? (LRESULT)item->data(Qt::UserRole).toLongLong() :
        LB_ERR;
    case LB_SETITEMDATA:
      item = l->item((int)wParam);
      if (item == NULL)
        return LB_ERR;
      item->setData(Qt::UserRole, (qlonglong)lParam);
      return 0;
    case LB_GETTEXT:
      item = l->item((int)wParam);
      if (item == NULL)
        return LB_ERR;
      strcpy((char *)lParam, item->text().toLatin1().constData());
      return item->text().size();
    case LB_INITSTORAGE:
      return 0;
    }
  }
  return 0;
}


/*
******************************************************************************
** Win32 Subset: Common Dialogs.
******************************************************************************
*/


// Convert a Win32 filter string ("Desc\0*.ext\0Desc2\0*.ext2\0\0") to a Qt
// one ("Desc (*.ext);;Desc2 (*.ext2)"), returning the selected entry too.

static QString QsFilterFromOfn(LPOPENFILENAME pofn, QString *pqsSel)
{
  QStringList list;
  CONST char *pch = pofn->lpstrFilter, *pchPat;
  QString qsDesc, qsPat;
  int i = 1;

  if (pch == NULL)
    return QString();
  while (*pch) {
    qsDesc = QString::fromLatin1(pch);
    pchPat = pch + CchSz(pch) + 1;
    qsPat = QString::fromLatin1(pchPat).replace(';', ' ');
    if (qsDesc.contains('('))
      qsDesc = qsDesc.left(qsDesc.indexOf('(')).trimmed();
    if (qsPat == "*.*")
      qsPat = "*";
    list << QString("%1 (%2)").arg(qsDesc, qsPat);
    if (i == (int)pofn->nFilterIndex && pqsSel != NULL)
      *pqsSel = list.last();
    pch = pchPat + CchSz(pchPat) + 1;
    i++;
  }
  return list.join(";;");
}

// Store a chosen file name into the OPENFILENAME's buffers: the full path in
// lpstrFile, and the name alone in lpstrFileTitle.

static void SetOfnResult(LPOPENFILENAME pofn, const QString &qs)
{
  QByteArray ba = QDir::toNativeSeparators(qs).toLocal8Bit(),
    baTitle = QFileInfo(qs).fileName().toLocal8Bit();
  int cch;

  if (pofn->lpstrFile != NULL && pofn->nMaxFile > 0) {
    cch = (int)ba.size();
    if (cch >= (int)pofn->nMaxFile)
      cch = pofn->nMaxFile - 1;
    memcpy(pofn->lpstrFile, ba.constData(), cch);
    pofn->lpstrFile[cch] = chNull;
  }
  // Astrolog opens lpstrFileTitle relative to the current directory, so for
  // a file in another folder give the full path there too.
  if (pofn->lpstrFileTitle != NULL && pofn->nMaxFileTitle > 0) {
    if (QFileInfo(qs).absolutePath() != QDir::currentPath())
      baTitle = ba;
    cch = (int)baTitle.size();
    if (cch >= (int)pofn->nMaxFileTitle)
      cch = pofn->nMaxFileTitle - 1;
    memcpy(pofn->lpstrFileTitle, baTitle.constData(), cch);
    pofn->lpstrFileTitle[cch] = chNull;
  }
}

static BOOL FDoFileDialog(LPOPENFILENAME pofn, flag fSave)
{
  QWidget *parent = (g_win != NULL ? (QWidget *)g_win : NULL);
  QString qsSel, qsFilter = QsFilterFromOfn(pofn, &qsSel), qs, qsDir;
  char *pch = pofn->lpstrFile;

  qsDir = (pofn->lpstrInitialDir != NULL ?
    QString::fromLocal8Bit(pofn->lpstrInitialDir) : QDir::currentPath());
  // Default file name, if it isn't just a wildcard pattern.
  if (pch != NULL && *pch && strchr(pch, '*') == NULL)
    qsDir = QDir(qsDir).filePath(QString::fromLocal8Bit(pch));
  if (!fSave)
    qs = QFileDialog::getOpenFileName(parent,
      QsFromSz(pofn->lpstrTitle), qsDir, qsFilter, &qsSel);
  else
    qs = QFileDialog::getSaveFileName(parent,
      QsFromSz(pofn->lpstrTitle), qsDir, qsFilter, &qsSel);
  if (qs.isEmpty())
    return fFalse;
  if (fSave && pofn->lpstrDefExt != NULL && QFileInfo(qs).suffix().isEmpty())
    qs += QString(".") + QString::fromLatin1(pofn->lpstrDefExt);
  SetOfnResult(pofn, qs);
  return fTrue;
}

BOOL GetOpenFileName(LPOPENFILENAME pofn)
{
  return FDoFileDialog(pofn, fFalse);
}

BOOL GetSaveFileName(LPOPENFILENAME pofn)
{
  return FDoFileDialog(pofn, fTrue);
}

BOOL ChooseColor(LPCHOOSECOLOR pcc)
{
  QWidget *parent = (g_win != NULL ? (QWidget *)g_win : NULL);
  QColor qc;
  int i;

  // Make Astrolog's palette available as the custom colors.
  if (pcc->lpCustColors != NULL)
    for (i = 0; i < 16 && i < QColorDialog::customCount(); i++)
      QColorDialog::setCustomColor(i, QcFromKv(pcc->lpCustColors[i]));
  qc = QColorDialog::getColor(QcFromKv(pcc->rgbResult), parent);
  if (!qc.isValid())
    return fFalse;
  pcc->rgbResult = Rgb(qc.red(), qc.green(), qc.blue());
  return fTrue;
}

BOOL PrintDlg(LPPRINTDLG ppd)
{
  return fFalse;
}


/*
******************************************************************************
** Win32 Subset: GDI Text Output.
******************************************************************************
*/

HDC GetDC(HWND hwnd)
{
  g_dcWin.img = (!g_imgChart.isNull() ? &g_imgChart : NULL);
  g_dcWin.paint = NULL;
  return &g_dcWin;
}

int ReleaseDC(HWND hwnd, HDC hdc)
{
  if (g_view != NULL)
    g_view->update();
  if (hdc == &g_dcWin && hdc->paint == NULL)
    hdc->img = NULL;
  return 1;
}

int GetDeviceCaps(HDC hdc, int nIndex)
{
  QScreen *scr = QGuiApplication::primaryScreen();

  if (scr == NULL)
    return 0;
  switch (nIndex) {
  case HORZRES:    return scr->geometry().width();
  case VERTRES:    return scr->geometry().height();
  case LOGPIXELSX: return (int)scr->logicalDotsPerInchX();
  case LOGPIXELSY: return (int)scr->logicalDotsPerInchY();
  }
  return 0;
}

// Text output for text charts. The characters come from PrintSz() in the IBM
// or Latin-1 character set, and are drawn one per fixed width cell.

BOOL TextOut(HDC hdc, int x, int y, LPCSTR sz, int cch)
{
  QString qs;
  QFontMetrics *pfm;
  int i;

  if (hdc == NULL || hdc->paint == NULL)
    return fFalse;
  for (i = 0; i < cch; i++)
    qs += QChar((ushort)(us.nCharsetOut != ccLatin ?
      WchFromChIBM((uchar)sz[i]) : WchFromChLatin((uchar)sz[i])));
  pfm = new QFontMetrics(hdc->paint->font());
  if (hdc->nBkMode == OPAQUE)
    hdc->paint->fillRect(x, y, wi.xChar * cch, wi.yChar,
      QcFromKv(KvFromKi(gs.fInverse ? kWhiteA : kBlackA)));
  hdc->paint->setPen(QcFromKv(hdc->kvText));
  for (i = 0; i < qs.size(); i++)
    hdc->paint->drawText(x + i*wi.xChar, y + pfm->ascent(), QString(qs[i]));
  delete pfm;
  return fTrue;
}

COLORREF SetTextColor(HDC hdc, COLORREF kv)
{
  COLORREF kvOld;

  if (hdc == NULL)
    return 0;
  kvOld = hdc->kvText;
  hdc->kvText = kv;
  return kvOld;
}

int SetBkMode(HDC hdc, int nMode)
{
  int nOld;

  if (hdc == NULL)
    return 0;
  nOld = hdc->nBkMode;
  hdc->nBkMode = nMode;
  return nOld;
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ hobj)
{
  return NULL;
}

BOOL DeleteObject(HGDIOBJ hobj)
{
  return fTrue;
}

HGDIOBJ GetStockObject(int n)
{
  return NULL;
}

int SetMapMode(HDC hdc, int nMode)
{
  return MM_TEXT;
}

BOOL SetWindowOrgEx(HDC hdc, int x, int y, LPPOINT ppt)
{
  return fTrue;
}

BOOL SetWindowExtEx(HDC hdc, int x, int y, SIZE *psize)
{
  return fTrue;
}

BOOL SetViewportOrgEx(HDC hdc, int x, int y, LPPOINT ppt)
{
  return fTrue;
}

BOOL SetViewportExtEx(HDC hdc, int x, int y, SIZE *psize)
{
  return fTrue;
}


/*
******************************************************************************
** Win32 Subset: System.
******************************************************************************
*/

DWORD GetModuleFileName(HINSTANCE hinst, LPSTR sz, DWORD cchMax)
{
  QByteArray ba;
  ssize_t cch;

  if (cchMax == 0)
    return 0;
  if (g_app != NULL)
    ba = QCoreApplication::applicationFilePath().toLocal8Bit();
  else {
    cch = readlink("/proc/self/exe", sz, cchMax-1);
    if (cch < 0)
      cch = 0;
    sz[cch] = chNull;
    return (DWORD)cch;
  }
  if ((DWORD)ba.size() >= cchMax)
    ba.truncate(cchMax-1);
  memcpy(sz, ba.constData(), ba.size());
  sz[ba.size()] = chNull;
  return (DWORD)ba.size();
}

// Open a file with the desktop's default application for it. For Windows
// Internet Shortcut (.url) files, open the URL they contain instead.

static void OpenWithDesktop(const QString &qsFile)
{
  QFile file(qsFile);
  QString qsLine;

  if (qsFile.endsWith(".url", Qt::CaseInsensitive) &&
    file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    while (!file.atEnd()) {
      qsLine = QString::fromLocal8Bit(file.readLine()).trimmed();
      if (qsLine.startsWith("URL=", Qt::CaseInsensitive)) {
        QDesktopServices::openUrl(QUrl(qsLine.mid(4)));
        return;
      }
    }
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(qsFile));
}

// WinExec is only used to open a text file in "Notepad": Use the desktop's
// default text editor instead.

UINT WinExec(LPCSTR szCmd, UINT uShow)
{
  CONST char *pch = strchr(szCmd, ' ');

  OpenWithDesktop(QString::fromLocal8Bit(pch != NULL ? pch+1 : szCmd));
  return 32;
}

HINSTANCE ShellExecute(HWND hwnd, LPCSTR szOp, LPCSTR szFile, LPCSTR szParam,
  LPCSTR szDir, int nShow)
{
  OpenWithDesktop(QString::fromLocal8Bit(szFile));
  return (HINSTANCE)(intptr_t)33;
}

UINT GetWindowsDirectory(LPSTR sz, UINT cchMax)
{
  QByteArray ba = QDir::homePath().toLocal8Bit();

  if (cchMax == 0)
    return 0;
  if ((UINT)ba.size() >= cchMax)
    ba.truncate(cchMax-1);
  memcpy(sz, ba.constData(), ba.size());
  sz[ba.size()] = chNull;
  return (UINT)ba.size();
}

HANDLE CreateMutex(void *psa, BOOL fOwner, LPCSTR szName)
{
  return NULL;
}

DWORD WaitForSingleObject(HANDLE h, DWORD ms)
{
  return 0;
}

BOOL ReleaseMutex(HANDLE h)
{
  return fTrue;
}

BOOL CloseHandle(HANDLE h)
{
  return fTrue;
}


/*
******************************************************************************
** TrueType Fonts in Charts.
******************************************************************************
*/

// Astrolog's astrology fonts come with the program in its font directory,
// and are loaded for the program only, like the Windows version does with
// FEnsureFontInstalled(). The text fonts (Arial, Courier New, etc.) are
// taken from the system, with Qt substituting a similar one if needed.

int fQtScreenChart = fFalse;

static CONST char *rgszFontFileQt[cFont] = {"", "", "Astro.ttf",
  "EnigmaAstrology.ttf", "HamburgSymbols.ttf", "Astronomicon.ttf",
  "StarFontSans.ttf", "StarFontSerif.ttf", "HanksNakshatra.ttf",
  "", "", "", "", ""};
static int rgnFontLoaded[cFont];        // 0 = not tried, 1 = ok, -1 = no.
static QString rgqsFontFamily[cFont];


// Make sure the given font is available, loading it from the program's
// font files if need be. Return whether it can be drawn with.

static flag FEnsureFontQt(int fi)
{
  QString qsFile;
  QStringList list;
  int id;

  if (fi <= 0 || fi >= cFont)
    return fFalse;
  if (rgnFontLoaded[fi] != 0)
    return rgnFontLoaded[fi] > 0;
  rgnFontLoaded[fi] = -1;
  rgqsFontFamily[fi] = QString::fromLatin1(rgszFontName[fi]);

  // Text fonts always work, since Qt substitutes a similar font if needed.
  if (fi >= fiArial) {
    rgnFontLoaded[fi] = 1;
    return fTrue;
  }

  // Symbol fonts need the actual font: Use it if installed on the system,
  // else load it from the program's files.
  if (QFontDatabase::families().contains(rgqsFontFamily[fi])) {
    rgnFontLoaded[fi] = 1;
    return fTrue;
  }
  if (!*rgszFontFileQt[fi])
    return fFalse;
  qsFile = QsFindProgramFile(rgszFontFileQt[fi]);
  if (qsFile.isEmpty()) {
    qsFile = QString("font/") + QString::fromLatin1(rgszFontFileQt[fi]);
    qsFile = QsFindProgramFile(qsFile.toLatin1().constData());
  }
  if (qsFile.isEmpty())
    return fFalse;
  id = QFontDatabase::addApplicationFont(qsFile);
  if (id < 0)
    return fFalse;
  list = QFontDatabase::applicationFontFamilies(id);
  if (!list.isEmpty())
    rgqsFontFamily[fi] = list[0];
  rgnFontLoaded[fi] = 1;
  return fTrue;
}


// Return the Unicode character for an 8 bit character code in the Windows
// ANSI character set, which is what Astrolog's astrology fonts use.

static int WchFromChAnsi(int ch)
{
  static CONST ushort rgwch[32] = {
    0x20AC, 0x81, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x8D, 0x017D, 0x8F,
    0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D, 0x017E, 0x0178};

  return ch >= 0x80 && ch < 0xA0 ? rgwch[ch - 0x80] : ch;
}


// Draw a string in a TrueType font into the window chart bitmap, either
// centered at or with its upper left corner at the given point. The height
// is the character cell height, like the Windows CreateFont() takes.

static flag FDrawFontSz(const QString &qs, int x, int y, int fi, int nHeight,
  flag fCenter)
{
  QFont font;
  int nPixel, cx, cy;

  if (!fQtScreenChart || gi.bmp.rgb == NULL || nHeight <= 0 ||
    !FEnsureFontQt(fi))
    return fFalse;
  QImage img(gi.bmp.rgb, gi.bmp.x, gi.bmp.y, gi.bmp.clRow << 2,
    QImage::Format_BGR888);
  font = QFont(rgqsFontFamily[fi]);
  font.setStyleStrategy(QFont::PreferAntialias);
  font.setWeight(!gs.fThick ? QFont::Normal : QFont::ExtraBold);
  if (fi >= fiArial && fi != fiArial)
    font.setStyleHint(QFont::TypeWriter);

  // Qt sizes fonts by their em height, so scale that until the full height
  // of the font (ascent plus descent) matches the requested cell height.
  font.setPixelSize(nHeight);
  {
    QFontMetrics fm(font);
    nPixel = fm.height() > 0 ? nHeight * nHeight / fm.height() : nHeight;
  }
  font.setPixelSize(nPixel > 0 ? nPixel : 1);
  QFontMetrics fm(font);
  QPainter p(&img);
  p.setFont(font);
  p.setPen(QcFromKv(gi.kvCur));
  if (fCenter) {
    cx = fm.horizontalAdvance(qs);
    cy = fm.height();
    x -= cx >> 1;
    y -= cy >> 1;
  }
  p.drawText(x, y + fm.ascent(), qs);
  return fTrue;
}


// Draw an astrology character from a special font in the chart. Used to draw
// sign, planet, aspect, and Nakshatra glyphs from these fonts within charts.
// This is the Qt version of the routine in xgeneral.cpp.

flag DrawGlyph(int ch, int x, int y, int fi, int nScale)
{
  QString qs;

  if (fi == fiArial && ch < 0)
    qs = QString::number(-ch);    // A multi-character number.
  else if (fi >= fiArial)
    qs = QString(QChar((char16_t)ch));
  else if (fi == fiWingding || fi == fiNakshatr)
    qs = QString(QChar((char16_t)(uchar)ch));  // Symbol character set.
  else
    qs = QString(QChar((char16_t)WchFromChAnsi((uchar)ch)));
  return FDrawFontSz(qs, x, y, fi, 12*gi.nScale*nScale/100, fTrue);
}


// Draw one character of chart text in a TrueType font, for DrawSz().

void QtDrawFontChar(int wch, int x, int y, int fi, int nHeight, int fCenter)
{
  if (fi == fiLucida)
    nHeight = nHeight*90/100;
  FDrawFontSz(QString(QChar((char16_t)wch)), x, y, fi, nHeight, fCenter);
}


/*
******************************************************************************
** Printing.
******************************************************************************
*/

// Implements the File / Print menu command: Bring up the print dialog, and
// print the current chart, scaled to fit the page. Like the Windows version,
// if smart saving (-YOO) is on, graphics charts print on a white background.
// Text charts are printed as text, using a fixed width font.

flag API DlgPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  QPrintDialog dlg(&printer, g_win);
  QPainter p;
  QRect rcPage;
  QImage img;
  QFile file;
  QStringList listLine;
  QFont font;
  flag fInverse = gs.fInverse, fAnsiColor, fAnsiChar, fHTML;
  int y, i, cy;

  wi.fNoUpdate = fFalse;
  printer.setDocName(QString(szAppName));
  if (dlg.exec() != QDialog::Accepted)
    return fTrue;

  if (us.fGraphics) {
    // Draw the chart as displayed, on a white background if so set.
    if (us.fSmartSave && !gs.fInverse) {
      gs.fInverse = fTrue;
      FRedraw();
    }
    img = g_imgChart;
    if (gs.fInverse != fInverse) {
      gs.fInverse = fInverse;
      FRedraw();
    }
    if (img.isNull())
      return fFalse;
    if (!p.begin(&printer))
      return fFalse;
    rcPage = p.viewport();
    QSize size = img.size().scaled(rcPage.size(), Qt::KeepAspectRatio);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawImage(QRect(rcPage.x() + (rcPage.width() - size.width())/2,
      rcPage.y(), size.width(), size.height()), img);
    p.end();
    return fTrue;
  }

  // For text charts, create the chart as plain text in a temporary file,
  // like Export Chart Text Output does, then print the lines of that file.
  fAnsiColor = us.fAnsiColor; fAnsiChar = us.fAnsiChar; fHTML = us.fTextHTML;
  us.fAnsiColor = us.fAnsiChar = us.fTextHTML = fFalse;
  FCloneSz(szFileTemp, &is.szFileScreen);
  FRedraw();
  us.fAnsiColor = fAnsiColor; us.fAnsiChar = fAnsiChar; us.fTextHTML = fHTML;
  file.setFileName(QString::fromLocal8Bit(szFileTemp));
  if (!file.open(QIODevice::ReadOnly))
    return fFalse;
  while (!file.atEnd()) {
    QByteArray ba = file.readLine();
    while (ba.endsWith('\n') || ba.endsWith('\r'))
      ba.chop(1);
    listLine << QsFromSz(ba.constData(), (int)ba.size());
  }
  file.close();
  unlink(szFileTemp);
  if (!p.begin(&printer))
    return fFalse;
  font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPointSize(9);
  p.setFont(font);
  QFontMetrics fm(p.font());
  rcPage = p.viewport();
  cy = fm.lineSpacing();
  y = 0;
  for (i = 0; i < listLine.size(); i++) {
    if (y + cy > rcPage.height()) {
      printer.newPage();
      y = 0;
    }
    p.drawText(0, y + fm.ascent(), listLine[i]);
    y += cy;
  }
  p.end();
  return fTrue;
}


/*
******************************************************************************
** Main Program.
******************************************************************************
*/

// Combine command line arguments into one string for FProcessCommandLine(),
// quoting any arguments containing spaces.

static void SzFromArgv(int argc, char **argv, char *sz, int cchMax)
{
  std::string s;
  int i;

  for (i = 1; i < argc; i++) {
    if (i > 1)
      s += ' ';
    if (strchr(argv[i], ' ') != NULL) {
      s += '"'; s += argv[i]; s += '"';
    } else
      s += argv[i];
  }
  strncpy(sz, s.c_str(), cchMax-1);
  sz[cchMax-1] = chNull;
}


// The main program, the starting point of the Qt version of Astrolog. This
// follows WinMain() in wdriver.cpp: Initialize, process the astrolog.as file
// and command line, bring up the window, then process events until done.

int main(int argc, char **argv)
{
  char szCmdLine[cchSzMax];
  int nRet;
#ifdef BETA
  char sz[cchSzMax];
#endif

  // Set the names before creating the application object, since Qt 6
  // already registers the app ID (desktop file name) with the desktop
  // portal while constructing it.
  QCoreApplication::setApplicationName(QString(szAppName));
  QCoreApplication::setApplicationVersion(QString(szVersionCore));
  QGuiApplication::setDesktopFileName(QString("astrolog"));
  QApplication app(argc, argv);
  g_app = &app;

  // Create the window and the globals that refer to it.

  g_win = new AstroMainWindow();
  g_wndMain.widget = g_win;
  wi.hwndMain = wi.hwnd = &g_wndMain;
  CreateMainMenu();
  g_timer = new QTimer(g_win);
  QObject::connect(g_timer, &QTimer::timeout, []()
    { QtDispatch(WM_TIMER, 1, 0); });

  // Set up some globals that can't be initialized at compile time.

  InitProgram();
  is.szProgName = argv[0];
  ofn.hwndOwner = wi.hwndMain;
  ofn.lpstrFile = szFileName;
  ofn.lpstrFileTitle = szFileTitle;
  prd.hwndOwner = wi.hwndMain;

  // Process the astrolog.as file and the command line.

  FProcessSwitchFile(DEFAULT_INFOFILE, NULL);
  ciTran = ciHexa = ciFive = ciFour = ciThre = ciTwin = ciMain = ciCore;
  SzFromArgv(argc, argv, szCmdLine, cchSzMax);
  FProcessCommandLine(szCmdLine);
  InitRestrictions(fTrue);

  // Actually bring up and display the window for the first time.

  if (gs.xWin <= 0)
    gs.xWin = DEFAULTX;
  if (gs.yWin <= 0)
    gs.yWin = DEFAULTY;
  g_win->resize(gs.xWin + g_sbVert->sizeHint().width(),
    gs.yWin + g_sbHorz->sizeHint().height());
  g_win->setWindowIcon(QIcon(QsFindProgramFile("astrlog1.ico")));
  g_win->show();
  g_view->setFocus();
  ResizeWindowToChart();
  RedoMenu();
  wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
#ifdef BETA
  sprintf(sz, "This is a beta version of %s %s! "
    "That means changes are still being made and testing is not complete. "
    "If this is being run after %s %d, %d, "
    "it should be replaced with the finished release.",
    szAppName, szVersionCore, szMonth[ciSave.mon], ciSave.day, ciSave.yea);
  PrintWarning(sz);
#endif
  wi.fRedraw = fTrue;
  ProcessState();

  // Process events until the program is told to terminate.

  nRet = app.exec();
  g_fClosing = fTrue;
  g_timer->stop();
  wi.hwndMain = wi.hwnd = NULL;
  g_view = NULL;
  delete g_win;
  g_win = NULL;
  g_app = NULL;

  // Cleanup and exit Astrolog.

  FinalizeProgram(fFalse);
  return nRet;
}
#endif // QT

/* wqt.cpp */

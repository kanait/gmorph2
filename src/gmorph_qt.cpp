/* Qt6 GUI — no Motif/X11 headers here (macro clash with Qt). */

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QIcon>
#include <QtGui/QCloseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtGui/QKeySequence>
#include <QtGui/QShortcut>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QWheelEvent>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

#include <cstdlib>

#include <QtCore/QSize>
#include <QtCore/QString>

#include "gmorph_qt_bridge.h"

/* Mirror X11 event / mask values (avoid including Xlib before Qt). */
namespace X11Vals {
enum {
  ButtonPress = 4,
  ButtonRelease = 5,
  MotionNotify = 6,
  Button1 = 1,
  Button2 = 2,
  Button3 = 3,
  ShiftMask = 1 << 0,
  ControlMask = 1 << 2,
  Button1Mask = 1 << 8,
  Button2Mask = 1 << 9,
  Button3Mask = 1 << 10,
};
}

/* File modes (smd.h) */
enum {
  SMDOPEN = 0,
  SMDSAVE = 1,
  SMDFILEPPD1 = 0,
  SMDFILEPPD2 = 1,
  SMDFILEGMH = 2,
  SMDFILEGPPD = 3,
  SCREEN1 = 0,
  SCREEN2 = 1,
  SMD_ON = 1,
  SMD_OFF = 0,
};

extern "C" {
struct Swin;
extern struct Swin *swin;
extern char versionshort[];
void free_swin(struct Swin *);
void drawwindow(int);
void gmorph_gl_pointer_event(int, int, unsigned int, unsigned int, int, int);
void gmorph_file_dialog_ok(const char *);
int gmorph_compute_morph(void);
int time_last_processed(double *real_sec, double *user_sec, double *sys_sec);
void gmorph_play_morph_animation(void);
void gmorph_reset_morph_view(void);
void gmorph_view_original_meshes(void);
void gmorph_change_edit_type(int kind);
void gqt_shortest_paths_create(void);
void gqt_shortest_paths_clear(void);
}

static QOpenGLWidget *g_glWidgets[2];
static QMainWindow *g_mainWin;

namespace {

/* Keep in sync with EDIT_* / EDIT_NONE in smd.h */
constexpr int kEditNone = 12;

struct EditModeSpec {
  int kind;
  const char *iconBase;
  const char *tip;
};

const EditModeSpec kEditModes[] = {
    {0, "edit_create_ppdloop", "Create PPD loop"},
    {1, "edit_delete_ppdloop", "Delete PPD loop"},
    {2, "edit_create_ppdface", "Create PPD face"},
    {3, "edit_delete_ppdface", "Delete PPD face"},
    {4, "edit_create_cvp", "Create harmonic control point"},
    {5, "edit_move_cvp", "Move control point"},
    {6, "edit_delete_cvp", "Delete harmonic control point"},
    {7, "edit_create_pcm_face", "Create harmonic face"},
    {8, "edit_delete_pcm_face", "Delete harmonic face"},
    {9, "edit_make_shortestpath", "Select vertices for shortest path"},
    {10, "edit_delete_shortestpath", "Delete shortest path"},
    {11, "edit_manu_shortestpath", "Manual shortest path"},
};

static QIcon editToolbarIcon(const char *base)
{
  QIcon icon;
  const QString p = QStringLiteral(":/gmorph/icons/");
  icon.addFile(p + QString::fromUtf8(base) + QStringLiteral("_32.png"), QSize(32, 32));
  icon.addFile(p + QString::fromUtf8(base) + QStringLiteral("_96.png"), QSize(32, 32));
  return icon;
}

static void qtShowMorphComputeResult(QMainWindow &parent, int ok)
{
  if (!ok) {
    QMessageBox::warning(&parent, QStringLiteral("Morph"),
                          QStringLiteral("Morph failed. Check the message log for details."));
    return;
  }
  double real_s = 0, user_s = 0, sys_s = 0;
  QString text = QStringLiteral("Morph completed successfully.");
  if (time_last_processed(&real_s, &user_s, &sys_s)) {
    text += QStringLiteral("\n\nProcessed time:\n"
                           "\treal:\t%1 (s)\n"
                           "\tuser:\t%2 (s)\n"
                           "\tsys:\t%3 (s)")
                .arg(real_s, 0, 'f', 2)
                .arg(user_s, 0, 'f', 2)
                .arg(sys_s, 0, 'f', 2);
  }
  QMessageBox::information(&parent, QStringLiteral("Morph"), text);
}

} /* namespace */

extern "C" void gmorph_qt_make_gl_current(int screen_idx)
{
  if (screen_idx >= 0 && screen_idx < 2 && g_glWidgets[screen_idx] != nullptr)
    g_glWidgets[screen_idx]->makeCurrent();
}

extern "C" void gmorph_qt_request_update(int screen_idx)
{
  if (screen_idx >= 0 && screen_idx < 2 && g_glWidgets[screen_idx] != nullptr)
    g_glWidgets[screen_idx]->update();
}

extern "C" void gmorph_qt_process_events(void)
{
  QCoreApplication::processEvents();
}

extern "C" void gmorph_qt_set_window_title(const char *title)
{
  if (g_mainWin != nullptr)
    g_mainWin->setWindowTitle(QString::fromUtf8(title));
}

class MorphGLWidget final : public QOpenGLWidget {
public:
  explicit MorphGLWidget(int screenIdx, QWidget *parent = nullptr)
      : QOpenGLWidget(parent), screenIdx_(screenIdx)
  {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
  }

protected:
  void initializeGL() override
  {
    gmorph_qt_bridge_init_gl_screen(screenIdx_);
  }

  void resizeGL(int w, int h) override
  {
    /* Qt calls paintGL after resizeGL; do not draw here (would miss QWidget::update). */
    gmorph_qt_bridge_set_screen_geom(screenIdx_, w, h);
  }

  void paintGL() override
  {
    gqt_set_qt_in_paint_gl(1);
    drawwindow(screenIdx_);
    gqt_set_qt_in_paint_gl(0);
  }

  static unsigned xState(Qt::KeyboardModifiers m, Qt::MouseButtons b)
  {
    unsigned st = 0;
    if (m.testFlag(Qt::ShiftModifier))
      st |= X11Vals::ShiftMask;
    if (m.testFlag(Qt::ControlModifier))
      st |= X11Vals::ControlMask;
    if (b.testFlag(Qt::LeftButton))
      st |= X11Vals::Button1Mask;
    if (b.testFlag(Qt::MiddleButton))
      st |= X11Vals::Button2Mask;
    if (b.testFlag(Qt::RightButton))
      st |= X11Vals::Button3Mask;
    return st;
  }

  void mousePressEvent(QMouseEvent *e) override
  {
    unsigned st = xState(e->modifiers(), e->buttons());
    unsigned btn = 0;
    if (e->button() == Qt::LeftButton)
      btn = X11Vals::Button1;
    else if (e->button() == Qt::MiddleButton)
      btn = X11Vals::Button2;
    else if (e->button() == Qt::RightButton)
      btn = X11Vals::Button3;
    gmorph_gl_pointer_event(screenIdx_, X11Vals::ButtonPress, btn, st,
                            static_cast<int>(e->position().x()),
                            static_cast<int>(e->position().y()));
  }

  void mouseReleaseEvent(QMouseEvent *e) override
  {
    unsigned st = xState(e->modifiers(), e->buttons());
    unsigned btn = 0;
    if (e->button() == Qt::LeftButton)
      btn = X11Vals::Button1;
    else if (e->button() == Qt::MiddleButton)
      btn = X11Vals::Button2;
    else if (e->button() == Qt::RightButton)
      btn = X11Vals::Button3;
    gmorph_gl_pointer_event(screenIdx_, X11Vals::ButtonRelease, btn, st,
                            static_cast<int>(e->position().x()),
                            static_cast<int>(e->position().y()));
  }

  void mouseMoveEvent(QMouseEvent *e) override
  {
    unsigned st = xState(e->modifiers(), e->buttons());
    gmorph_gl_pointer_event(screenIdx_, X11Vals::MotionNotify, 0, st,
                            static_cast<int>(e->position().x()),
                            static_cast<int>(e->position().y()));
  }

  void wheelEvent(QWheelEvent *e) override
  {
    int dy = e->angleDelta().y();
    if (dy == 0)
      dy = e->pixelDelta().y() * 12;
    gqt_screen_apply_wheel_zoom(screenIdx_, dy);
    drawwindow(screenIdx_);
    if (*gqt_ptr_qt_sync_views() == SMD_ON)
      drawwindow(1 - screenIdx_);
    e->accept();
  }

private:
  int screenIdx_;
};

class GMorphMainWindow final : public QMainWindow {
public:
  GMorphMainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {}

protected:
  void closeEvent(QCloseEvent *e) override
  {
    static bool freed = false;
    if (!freed && swin != nullptr) {
      freed = true;
      free_swin(swin);
      swin = nullptr;
    }
    e->accept();
  }
};

static void qtRedrawBoth()
{
  drawwindow(SCREEN1);
  drawwindow(SCREEN2);
}

static void qtOpenFile(int filed)
{
  QString filter;
  QString title;
  switch (filed) {
  case SMDFILEGMH:
    title = QStringLiteral("Open GMH");
    filter = QStringLiteral("GMH (*.gmh);;All files (*)");
    break;
  case SMDFILEPPD1:
    title = QStringLiteral("Open PPD (left)");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  case SMDFILEPPD2:
    title = QStringLiteral("Open PPD (right)");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  case SMDFILEGPPD:
    title = QStringLiteral("Open morph PPD");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  default:
    return;
  }
  QString path = QFileDialog::getOpenFileName(g_mainWin, title, QString(), filter);
  if (path.isEmpty())
    return;
  gqt_set_file_context(filed, SMDOPEN);
  gmorph_file_dialog_ok(path.toUtf8().constData());
}

static void qtSaveFile(int filed)
{
  QString filter;
  QString title;
  switch (filed) {
  case SMDFILEGMH:
    title = QStringLiteral("Save GMH");
    filter = QStringLiteral("GMH (*.gmh);;All files (*)");
    break;
  case SMDFILEPPD1:
    title = QStringLiteral("Save PPD (left)");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  case SMDFILEPPD2:
    title = QStringLiteral("Save PPD (right)");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  case SMDFILEGPPD:
    title = QStringLiteral("Save morph PPD");
    filter = QStringLiteral("PPD (*.ppd);;All files (*)");
    break;
  default:
    return;
  }
  QString path = QFileDialog::getSaveFileName(g_mainWin, title, QString(), filter);
  if (path.isEmpty())
    return;
  gqt_set_file_context(filed, SMDSAVE);
  gmorph_file_dialog_ok(path.toUtf8().constData());
}

extern "C" void gmorph_run_qt6_gui(int argc, char **argv, const char *loaded_gmh_path)
{
#if defined(__APPLE__)
  /* Must match main.c: never use xcb here (invalid Display* → XGetXCBConnection crash). */
  qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("cocoa"));
#endif
  QApplication app(argc, argv);

  QSurfaceFormat fmt;
  fmt.setDepthBufferSize(24);
  fmt.setStencilBufferSize(8);
  fmt.setSamples(4);
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
  fmt.setOption(QSurfaceFormat::DeprecatedFunctions, true);
  QSurfaceFormat::setDefaultFormat(fmt);

  GMorphMainWindow mainWin;
  g_mainWin = &mainWin;

  if (loaded_gmh_path != nullptr && loaded_gmh_path[0] != '\0') {
    char title[8192];
    std::snprintf(title, sizeof(title), "%s: %s", versionshort, loaded_gmh_path);
    mainWin.setWindowTitle(QString::fromUtf8(title));
  } else {
    mainWin.setWindowTitle(QString::fromUtf8(versionshort));
  }

  auto *splitter = new QSplitter(Qt::Horizontal);
  g_glWidgets[0] = new MorphGLWidget(SCREEN1);
  g_glWidgets[1] = new MorphGLWidget(SCREEN2);
  splitter->addWidget(g_glWidgets[0]);
  splitter->addWidget(g_glWidgets[1]);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 1);
  mainWin.setCentralWidget(splitter);

  auto *shortcutPlayMorph =
      new QShortcut(QKeySequence(Qt::Key_Space), splitter);
  shortcutPlayMorph->setContext(Qt::WidgetWithChildrenShortcut);
  QObject::connect(shortcutPlayMorph, &QShortcut::activated, []() {
    gmorph_play_morph_animation();
  });

  QMenuBar *mb = mainWin.menuBar();
  QMenu *fileMenu = mb->addMenu(QStringLiteral("File"));
  {
    QAction *openGmh = fileMenu->addAction(QStringLiteral("Open GMH…"));
    /* Qt::CTRL → Command on macOS, Ctrl on Windows/Linux (same as other cross-platform apps). */
    openGmh->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    openGmh->setShortcutContext(Qt::ApplicationShortcut);
    QObject::connect(openGmh, &QAction::triggered, []() { qtOpenFile(SMDFILEGMH); });
  }
  fileMenu->addAction(QStringLiteral("Open PPD (left)…"), [] { qtOpenFile(SMDFILEPPD1); });
  fileMenu->addAction(QStringLiteral("Open PPD (right)…"), [] { qtOpenFile(SMDFILEPPD2); });
  fileMenu->addAction(QStringLiteral("Open morph PPD…"), [] { qtOpenFile(SMDFILEGPPD); });
  fileMenu->addSeparator();
  fileMenu->addAction(QStringLiteral("Save GMH…"), [] { qtSaveFile(SMDFILEGMH); });
  fileMenu->addAction(QStringLiteral("Save morph PPD…"), [] { qtSaveFile(SMDFILEGPPD); });
  fileMenu->addSeparator();
  fileMenu->addAction(QStringLiteral("Quit"), QKeySequence::Quit, &mainWin, &QMainWindow::close);

  QMenu *morphMenu = mb->addMenu(QStringLiteral("Morph"));
  morphMenu->addAction(QStringLiteral("Compute harmonic morph…"), [&mainWin] {
    const int ok = gmorph_compute_morph();
    qtRedrawBoth();
    qtShowMorphComputeResult(mainWin, ok);
  });
  morphMenu->addAction(QStringLiteral("Play morph animation (Space)"),
                       [] { gmorph_play_morph_animation(); });
  morphMenu->addAction(QStringLiteral("Reset morph mesh"), [] { gmorph_reset_morph_view(); });
  morphMenu->addAction(QStringLiteral("Show source / target meshes"), [] {
    gmorph_view_original_meshes();
  });

  QMenu *editMenu = mb->addMenu(QStringLiteral("Edit"));
  auto *editGroup = new QActionGroup(&mainWin);
  editGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  auto *editTb = new QToolBar(QStringLiteral("Edit"));
  editTb->setMovable(false);
  editTb->setIconSize(QSize(28, 28));

  for (const auto &em : kEditModes) {
    const QString label = QString::fromUtf8(em.tip);
    auto *act = new QAction(editToolbarIcon(em.iconBase), label, &mainWin);
    act->setToolTip(label);
    act->setCheckable(true);
    editGroup->addAction(act);
    editTb->addAction(act);
    editMenu->addAction(act);
    const int k = em.kind;
    QObject::connect(act, &QAction::triggered, [k](bool checked) {
      gmorph_change_edit_type(checked ? k : kEditNone);
      qtRedrawBoth();
    });
  }

  editMenu->addSeparator();
  editTb->addSeparator();

  QAction *spathBuild = new QAction(QStringLiteral("SPaths Create"), &mainWin);
  spathBuild->setToolTip(QStringLiteral(
      "Calculate shortest paths on the harmonic map (all hedge pairs)"));
  QObject::connect(spathBuild, &QAction::triggered, [] {
    gqt_shortest_paths_create();
    qtRedrawBoth();
  });
  editMenu->addAction(spathBuild);
  editTb->addAction(spathBuild);

  QAction *spathClear = new QAction(QStringLiteral("SPaths Clear"), &mainWin);
  spathClear->setToolTip(QStringLiteral("Clear precomputed shortest paths"));
  QObject::connect(spathClear, &QAction::triggered, [] {
    gqt_shortest_paths_clear();
    qtRedrawBoth();
  });
  editMenu->addAction(spathClear);
  editTb->addAction(spathClear);

  mainWin.addToolBar(Qt::TopToolBarArea, editTb);

  auto *morphTb = new QToolBar(QStringLiteral("Morph"));
  morphTb->setMovable(false);
  QAction *mkMorph = new QAction(QStringLiteral("Make morph"), &mainWin);
  QObject::connect(mkMorph, &QAction::triggered, [&mainWin] {
    const int ok = gmorph_compute_morph();
    qtRedrawBoth();
    qtShowMorphComputeResult(mainWin, ok);
  });
  morphTb->addAction(mkMorph);
  QAction *startMorph = new QAction(QStringLiteral("Start"), &mainWin);
  QObject::connect(startMorph, &QAction::triggered, [] { gmorph_play_morph_animation(); });
  morphTb->addAction(startMorph);
  QAction *interpMesh = new QAction(QStringLiteral("Interp. mesh"), &mainWin);
  QObject::connect(interpMesh, &QAction::triggered, [] { gmorph_reset_morph_view(); });
  morphTb->addAction(interpMesh);
  QAction *origMesh = new QAction(QStringLiteral("Orig. meshes"), &mainWin);
  QObject::connect(origMesh, &QAction::triggered, [] { gmorph_view_original_meshes(); });
  morphTb->addAction(origMesh);
  mainWin.addToolBar(Qt::TopToolBarArea, morphTb);

  auto *tb = new QToolBar(QStringLiteral("Display"));
  tb->setMovable(false);
  mainWin.addToolBar(tb);

  auto addToggle = [&](const char *label, int *(*getter)(void)) {
    int *flag = getter();
    auto *cb = new QCheckBox(QString::fromUtf8(label));
    cb->setChecked(*flag == SMD_ON);
    QObject::connect(cb, &QCheckBox::toggled, [flag](bool on) {
      *flag = on ? SMD_ON : SMD_OFF;
      qtRedrawBoth();
    });
    tb->addWidget(cb);
  };

  addToggle("Link views", gqt_ptr_qt_sync_views);
  tb->addSeparator();

  addToggle("Wire", gqt_ptr_dis3d_wire);
  addToggle("Shade", gqt_ptr_dis3d_shading);
  addToggle("Ctrl pts", gqt_ptr_dis3d_cpoint);
  addToggle("C mesh", gqt_ptr_dis3d_cmesh);
  addToggle("Loop", gqt_ptr_dis3d_loop);
  addToggle("Tile grp", gqt_ptr_dis3d_group);
  addToggle("SPath", gqt_ptr_dis3d_spath);
  addToggle("H map", gqt_ptr_dis3d_hmap);
  addToggle("Axes", gqt_ptr_dis3d_coaxis);

  int *extp = gqt_ptr_ext_display();
  auto *enh = new QCheckBox(QStringLiteral("Enhanced"));
  enh->setChecked(*extp == SMD_ON);
  QObject::connect(enh, &QCheckBox::toggled, [extp](bool on) {
    *extp = on ? SMD_ON : SMD_OFF;
    qtRedrawBoth();
  });
  tb->addWidget(enh);

  tb->addSeparator();
  tb->addWidget(new QLabel(QStringLiteral("mdiv")));
  int *mdivp = gqt_ptr_mdiv();
  auto *spin = new QSpinBox;
  spin->setRange(2, 100000);
  spin->setValue(*mdivp);
  QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                    [mdivp](int v) { *mdivp = v; });
  tb->addWidget(spin);

  int *smoothp = gqt_ptr_smooth_shading();
  auto *smooth = new QCheckBox(QStringLiteral("Smooth"));
  smooth->setChecked(*smoothp == SMD_ON);
  QObject::connect(smooth, &QCheckBox::toggled, [smoothp](bool on) {
    *smoothp = on ? SMD_ON : SMD_OFF;
    qtRedrawBoth();
  });
  tb->addWidget(smooth);

  mainWin.resize(1100, 560);
  mainWin.show();
  g_glWidgets[0]->setFocus(Qt::OtherFocusReason);
  (void)app.exec();

  g_mainWin = nullptr;
  g_glWidgets[0] = nullptr;
  g_glWidgets[1] = nullptr;
}

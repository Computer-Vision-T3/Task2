#include <QApplication>
#include <QFontDatabase>
#include "frontend/MainWindow.h"
#include "backend/Module2_HoughLines/HoughLines.h" // Added your module

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    QFont appFont("DM Sans", 13);
    appFont.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(appFont);
    QString styleSheet = R"(
        /* ═══════════════════════════════════════════════
           VISION STUDIO — LIGHT PALETTE UI THEME
           Palette: Pearl, Ivory, Sage, Warm Slate, Peach
           ═══════════════════════════════════════════════ */

        * { outline: none; }
        QMainWindow { background-color: #F5F3EF; }
        QWidget { color: #2C2825; font-family: 'DM Sans', 'Segoe UI', system-ui, sans-serif; font-size: 13px; }

        /* ── Sidebar / Left Rail ── */
        QWidget#leftRail { background-color: #FDFCFA; border-right: 1px solid #E8E2DA; }
        QWidget#taskListContainer { background-color: transparent; }

        /* ── Task Item Buttons ── */
        QPushButton#taskItem { background-color: transparent; color: #7A7268; border: none; border-radius: 10px; padding: 10px 14px; text-align: left; font-size: 12px; font-weight: 500; }
        QPushButton#taskItem:hover { background-color: #F0EDE8; color: #2C2825; }
        QPushButton#taskItem[active="true"] { background-color: #EDE8FF; color: #5B4FCF; font-weight: 700; }

        /* ── Top Header Bar ── */
        QWidget#headerBar { background-color: #FDFCFA; border-bottom: 1px solid #E8E2DA; min-height: 56px; max-height: 56px; }

        /* ── Parameter Strip ── */
        QWidget#paramStrip { background-color: #FAFAF8; border-bottom: 1px solid #EDEAE4; min-height: 117px; max-height: 117px; }

        /* ── Canvas Area ── */
        QWidget#canvasArea { background-color: #F5F3EF; }

        /* ── Image Panels ── */
        QWidget#imagePanel { background-color: #FFFFFF; border: 1px solid #E4DFD8; border-radius: 16px; }
        QWidget#imagePanelHeader { background-color: transparent; border-bottom: 1px solid #F0ECE6; }
        QLabel#panelTitle { font-size: 11px; font-weight: 700; letter-spacing: 0.08em; text-transform: uppercase; color: #A09890; }
        QLabel#panelBadge { background-color: #EDE8FF; color: #5B4FCF; border-radius: 4px; padding: 2px 7px; font-size: 10px; font-weight: 700; letter-spacing: 0.05em; }
        QLabel#panelBadgeInput { background-color: #E8F5F0; color: #2D9B6F; border-radius: 4px; padding: 2px 7px; font-size: 10px; font-weight: 700; letter-spacing: 0.05em; }
        QLabel#imageDisplay { background-color: #F9F7F4; border-radius: 10px; color: #C4BDB4; font-size: 12px; }

        /* ── Buttons ── */
        QPushButton#primaryBtn { background-color: #7C3AED; color: #FFFFFF; border: 1px solid #6D28D9; border-radius: 10px; padding: 9px 24px; font-weight: 800; font-size: 13px; letter-spacing: 0.02em; min-height: 34px; }
        QPushButton#primaryBtn:hover { background-color: #8B5CF6; border-color: #7C3AED; }
        QPushButton#primaryBtn:pressed { background-color: #6D28D9; border-color: #5B21B6; }
        QPushButton#primaryBtn:disabled { background-color: #C9C3EF; color: #FDFCFA; border-color: #C9C3EF; }

        QPushButton#secondaryBtn { background-color: #FFFFFF; color: #5B4FCF; border: 1.5px solid #CCC8F0; border-radius: 10px; padding: 8px 18px; font-weight: 600; font-size: 13px; }
        QPushButton#secondaryBtn:hover { background-color: #EDE8FF; border-color: #5B4FCF; }
        QPushButton#secondaryBtn:pressed { background-color: #DDD8FF; }

        QPushButton#ghostBtn { background-color: transparent; color: #A09890; border: 1.5px solid #DDD8D2; border-radius: 10px; padding: 8px 18px; font-weight: 600; font-size: 13px; }
        QPushButton#ghostBtn:hover { background-color: #F5F0EB; color: #5B4FCF; border-color: #CCC8F0; }

        QPushButton#iconBtn { background-color: transparent; color: #A09890; border: none; border-radius: 8px; padding: 6px; font-size: 16px; }
        QPushButton#iconBtn:hover { background-color: #F0ECE6; color: #5B4FCF; }

        QPushButton#loadBtn { background-color: #F4F1FF; color: #4D42B8; border: 1.5px solid #BDB4F3; border-radius: 10px; padding: 10px 16px; font-weight: 700; font-size: 12px; min-height: 28px; }
        QPushButton#loadBtn:hover { background-color: #E8E2FF; color: #3D348F; border-color: #8E84E8; }
        QPushButton#loadBtn:pressed { background-color: #DDD5FF; color: #2F2870; }

        /* ── Dropdowns ── */
        QComboBox { background-color: #FFFFFF; border: 1.5px solid #DDD8D2; border-radius: 9px; padding: 6px 32px 6px 12px; color: #2C2825; font-weight: 500; font-size: 13px; min-height: 34px; selection-background-color: #EDE8FF; }
        QComboBox:hover { border-color: #9B92E8; }
        QComboBox:focus { border-color: #5B4FCF; border-width: 1.5px; }
        QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 28px; border-left: 1px solid #E8E2DA; border-radius: 0 9px 9px 0; }
        QComboBox::down-arrow { width: 0; height: 0; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 5px solid #A09890; margin-right: 8px; }
        QComboBox QAbstractItemView { background-color: #FFFFFF; border: 1.5px solid #DDD8D2; border-top-left-radius: 0px; border-top-right-radius: 0px; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; padding: 0px; selection-background-color: #EDE8FF; selection-color: #5B4FCF; outline: none; }
        QComboBox QAbstractItemView:focus { outline: 0; }
        QComboBox QAbstractItemView::viewport { background-color: #FFFFFF; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; }
        QComboBox QAbstractItemView::item { padding: 8px 12px; border-radius: 7px; min-height: 30px; }

        /* ── Spin Boxes ── */
        QSpinBox { background-color: #FFFFFF; border: 1.5px solid #DDD8D2; border-radius: 9px; padding: 6px 10px; color: #2C2825; font-weight: 500; min-height: 34px; min-width: 70px; }
        QSpinBox:hover { border-color: #9B92E8; }
        QSpinBox:focus { border-color: #5B4FCF; }
        QSpinBox::up-button, QSpinBox::down-button { background-color: #F5F3EF; border: none; width: 22px; }
        QSpinBox::up-button { border-radius: 0 9px 0 0; border-left: 1px solid #E8E2DA; }
        QSpinBox::down-button { border-radius: 0 0 9px 0; border-left: 1px solid #E8E2DA; border-top: 1px solid #E8E2DA; }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover { background-color: #EDE8FF; }

        /* ── Sliders ── */
        QSlider::groove:horizontal { height: 4px; background: #E8E2DA; border-radius: 2px; }
        QSlider::handle:horizontal { background: #5B4FCF; border: 2px solid #FFFFFF; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px; }
        QSlider::handle:horizontal:hover { background: #4D42B8; width: 18px; height: 18px; margin: -7px 0; }
        QSlider::sub-page:horizontal { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #9B92E8, stop:1 #5B4FCF); border-radius: 2px; }

        /* ── Labels ── */
        QLabel { background-color: transparent; }
        QLabel#sectionLabel { font-size: 10px; font-weight: 800; letter-spacing: 0.12em; text-transform: uppercase; color: #C4BDB4; padding: 0 4px; }
        QLabel#statusIndicator { background-color: #E8F5F0; color: #2D9B6F; border-radius: 6px; padding: 4px 10px; font-size: 11px; font-weight: 700; }
        QLabel#paramLabel { font-size: 12px; font-weight: 600; color: #7A7268; }
        QLabel#valueLabel { font-size: 12px; font-weight: 700; color: #5B4FCF; min-width: 28px; }

        /* ── Info Sidebar ── */
        QTextBrowser { background-color: #FDFCFA; border: none; border-left: 1px solid #E8E2DA; color: #2C2825; font-size: 13px; padding: 20px 18px; selection-background-color: #EDE8FF; }
        QTextBrowser:focus { border: none; }

        /* ── Splitter ── */
        QSplitter::handle { background-color: #E8E2DA; }
        QSplitter::handle:horizontal { width: 1px; }
        QSplitter::handle:vertical { height: 1px; }
        QSplitter::handle:hover { background-color: #9B92E8; }

        /* ── Scrollbars ── */
        QScrollBar:vertical { background: transparent; width: 6px; margin: 0; }
        QScrollBar::handle:vertical { background: #D4CEC6; border-radius: 3px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #A09890; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        /* ── Tooltips ── */
        QToolTip { background-color: #FDFCFA; color: #2C2825; border: 1px solid #CFC7F4; border-radius: 8px; padding: 9px 12px; font-size: 12px; font-weight: 600; opacity: 240; }

        /* ── Help Bubble ── */
        QLabel#helpBubble { background-color: #EDE8FF; color: #5B4FCF; border-radius: 11px; font-weight: 800; font-size: 13px; min-width: 22px; min-height: 22px; max-width: 22px; max-height: 22px; padding: 0px; }
        QLabel#helpBubble:hover { background-color: #5B4FCF; color: #FFFFFF; }
    )";


    app.setStyleSheet(styleSheet);


    MainWindow window;
    window.setWindowTitle("Vision Studio — Task 2: Hough & Active Contours");
    window.resize(1440, 900);
    window.show();

    return app.exec();
}

#ifndef PADNAV_TRAYCONTROLLER_H
#define PADNAV_TRAYCONTROLLER_H

#include <QObject>

#include <memory>

class QAction;
class QApplication;
class QIcon;
class QMenu;
class QSystemTrayIcon;

namespace pn::application {
class PadNavController;
}

namespace pn::gui {

class SettingsWindow;

class TrayController final : public QObject {
    Q_OBJECT

public:
    TrayController(pn::application::PadNavController& controller,
                   SettingsWindow& settingsWindow,
                   QApplication& application,
                   QObject* parent = nullptr);
    ~TrayController() override;

private:
    void initTrayIcon();
    void initMenu();
    void initSigSlots();
    void showTrayIcon();
    void updateMappingActionText();
    [[nodiscard]] static auto createTrayIcon() -> QIcon;

private:
    pn::application::PadNavController& m_controller;
    SettingsWindow& m_settingsWindow;
    QApplication& m_application;
    QSystemTrayIcon* m_trayIcon = nullptr;
    std::unique_ptr<QMenu> m_menu;
    QAction* m_toggleMappingAction = nullptr;
    QAction* m_openSettingsAction = nullptr;
    QAction* m_exitAction = nullptr;
};

} // namespace pn::gui

#endif // PADNAV_TRAYCONTROLLER_H

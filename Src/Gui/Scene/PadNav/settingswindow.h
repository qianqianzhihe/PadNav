#ifndef PADNAV_SETTINGSWINDOW_H
#define PADNAV_SETTINGSWINDOW_H

#include "PadNav/profile.h"

#include <QWidget>

#include <array>

class QCheckBox;
class QCloseEvent;
class QPushButton;
class QSpinBox;

namespace pn::application {
class PadNavController;
}

namespace pn::gui {

class InputMonitorWindow;
class JsonProfileRepository;

class SettingsWindow final : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(pn::application::PadNavController& controller,
                   JsonProfileRepository& repository,
                   InputMonitorWindow& monitorWindow,
                   QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void initControlAttribute();
    void initLayout();
    void initSigSlots();
    void loadFromControllerProfile();
    void setControlsFromProfile(const pn::core::ChromeProfile& profile);
    [[nodiscard]] auto profileFromControls() const -> pn::core::ChromeProfile;

private slots:
    void onApply();
    void onCancel();
    void onOpenInputMonitor();

private:
    pn::application::PadNavController& m_controller;
    JsonProfileRepository& m_repository;
    InputMonitorWindow& m_monitorWindow;
    QSpinBox* m_deadZoneSpinBox = nullptr;
    QSpinBox* m_mouseSpeedSpinBox = nullptr;
    QSpinBox* m_scrollSpeedSpinBox = nullptr;
    std::array<QCheckBox*, 16> m_actionCheckBoxes{};
    QPushButton* m_applyButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
    QPushButton* m_openInputMonitorButton = nullptr;
};

} // namespace pn::gui

#endif // PADNAV_SETTINGSWINDOW_H

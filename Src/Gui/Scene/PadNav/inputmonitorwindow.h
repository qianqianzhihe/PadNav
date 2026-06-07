#ifndef PADNAV_INPUTMONITORWINDOW_H
#define PADNAV_INPUTMONITORWINDOW_H

#include <QWidget>

class QLabel;
class QTimer;

namespace pn::application {
class PadNavController;
}

namespace pn::gui {

class InputMonitorWindow final : public QWidget {
    Q_OBJECT

public:
    explicit InputMonitorWindow(pn::application::PadNavController& controller, QWidget* parent = nullptr);

private:
    void initControlAttribute();
    void initLayout();
    void initSigSlots();
    void updateDisplay();

private:
    pn::application::PadNavController& m_controller;
    QTimer* m_refreshTimer = nullptr;
    QLabel* m_connectedValueLabel = nullptr;
    QLabel* m_mappingEnabledValueLabel = nullptr;
    QLabel* m_activeLayerValueLabel = nullptr;
    QLabel* m_leftStickValueLabel = nullptr;
    QLabel* m_rightStickValueLabel = nullptr;
    QLabel* m_buttonsValueLabel = nullptr;
};

} // namespace pn::gui

#endif // PADNAV_INPUTMONITORWINDOW_H

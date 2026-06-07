#include "inputmonitorwindow.h"

#include "Application/padnavcontroller.h"

#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

namespace pn::gui {

namespace {

constexpr auto refreshIntervalMs = 100;

[[nodiscard]] auto boolText(bool value) -> QString {
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

[[nodiscard]] auto activeLayerText(pn::core::ActiveLayer activeLayer) -> QString {
    switch (activeLayer) {
    case pn::core::ActiveLayer::Base:
        return QStringLiteral("Base");
    case pn::core::ActiveLayer::Navigation:
        return QStringLiteral("Navigation");
    case pn::core::ActiveLayer::Tab:
        return QStringLiteral("Tab");
    case pn::core::ActiveLayer::Ambiguous:
        return QStringLiteral("Ambiguous");
    }

    return QStringLiteral("Unknown");
}

[[nodiscard]] auto stickText(const pn::core::Stick& stick) -> QString {
    return QStringLiteral("%1 / %2").arg(stick.x, 0, 'f', 3).arg(stick.y, 0, 'f', 3);
}

} // namespace

InputMonitorWindow::InputMonitorWindow(pn::application::PadNavController& controller, QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_refreshTimer(new QTimer(this)) {
    initControlAttribute();
    initLayout();
    initSigSlots();
    updateDisplay();
}

void InputMonitorWindow::initControlAttribute() {
    do {
        setWindowTitle(tr("Input Monitor"));
        resize(360, 220);
    } while (false);

    if (m_refreshTimer) {
        m_refreshTimer->setInterval(refreshIntervalMs);
    }
}

void InputMonitorWindow::initLayout() {
    auto* layout = new QGridLayout(this);

    auto addRow = [layout](int row, const QString& title, QLabel*& valueLabel) {
        auto* titleLabel = new QLabel(title);
        valueLabel = new QLabel();
        valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        layout->addWidget(titleLabel, row, 0);
        layout->addWidget(valueLabel, row, 1);
    };

    addRow(0, tr("Connected"), m_connectedValueLabel);
    addRow(1, tr("Mapping enabled"), m_mappingEnabledValueLabel);
    addRow(2, tr("Active layer"), m_activeLayerValueLabel);
    addRow(3, tr("Left stick x/y"), m_leftStickValueLabel);
    addRow(4, tr("Right stick x/y"), m_rightStickValueLabel);
    addRow(5, tr("Button bit mask"), m_buttonsValueLabel);
    layout->setColumnStretch(1, 1);
}

void InputMonitorWindow::initSigSlots() {
    if (!m_refreshTimer) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: m_refreshTimer";
        return;
    }

    connect(m_refreshTimer, &QTimer::timeout, this, &InputMonitorWindow::updateDisplay);
    m_refreshTimer->start();
}

void InputMonitorWindow::updateDisplay() {
    if (!m_connectedValueLabel || !m_mappingEnabledValueLabel || !m_activeLayerValueLabel
        || !m_leftStickValueLabel || !m_rightStickValueLabel || !m_buttonsValueLabel) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: monitor labels";
        return;
    }

    const auto snapshot = m_controller.monitorSnapshot();
    m_connectedValueLabel->setText(boolText(snapshot.controller.connected));
    m_mappingEnabledValueLabel->setText(boolText(snapshot.mappingEnabled));
    m_activeLayerValueLabel->setText(activeLayerText(snapshot.activeLayer));
    m_leftStickValueLabel->setText(stickText(snapshot.controller.leftStick));
    m_rightStickValueLabel->setText(stickText(snapshot.controller.rightStick));
    m_buttonsValueLabel->setText(QStringLiteral("0x%1").arg(snapshot.controller.buttons, 0, 16));
}

} // namespace pn::gui

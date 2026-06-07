#include "settingswindow.h"

#include "Application/padnavcontroller.h"
#include "inputmonitorwindow.h"
#include "jsonprofilerepository.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace pn::gui {

namespace {

struct ActionDisplay final {
    pn::core::ChromeActionId id;
    const char* text;
};

constexpr auto actionDisplays = std::array{
    ActionDisplay{pn::core::ChromeActionId::Back, "Back"},
    ActionDisplay{pn::core::ChromeActionId::Forward, "Forward"},
    ActionDisplay{pn::core::ChromeActionId::Reload, "Reload"},
    ActionDisplay{pn::core::ChromeActionId::StopLoading, "Stop loading"},
    ActionDisplay{pn::core::ChromeActionId::PageTop, "Page top"},
    ActionDisplay{pn::core::ChromeActionId::PageBottom, "Page bottom"},
    ActionDisplay{pn::core::ChromeActionId::ZoomIn, "Zoom in"},
    ActionDisplay{pn::core::ChromeActionId::ZoomOut, "Zoom out"},
    ActionDisplay{pn::core::ChromeActionId::ResetZoom, "Reset zoom"},
    ActionDisplay{pn::core::ChromeActionId::FocusAddressBar, "Focus address bar"},
    ActionDisplay{pn::core::ChromeActionId::NewTab, "New tab"},
    ActionDisplay{pn::core::ChromeActionId::CloseTab, "Close tab"},
    ActionDisplay{pn::core::ChromeActionId::PreviousTab, "Previous tab"},
    ActionDisplay{pn::core::ChromeActionId::NextTab, "Next tab"},
    ActionDisplay{pn::core::ChromeActionId::RestoreClosedTab, "Restore closed tab"},
    ActionDisplay{pn::core::ChromeActionId::FindInPage, "Find in page"},
};

} // namespace

SettingsWindow::SettingsWindow(pn::application::PadNavController& controller,
                               JsonProfileRepository& repository,
                               InputMonitorWindow& monitorWindow,
                               QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_repository(repository)
    , m_monitorWindow(monitorWindow)
    , m_deadZoneSpinBox(new QSpinBox(this))
    , m_mouseSpeedSpinBox(new QSpinBox(this))
    , m_scrollSpeedSpinBox(new QSpinBox(this))
    , m_applyButton(new QPushButton(tr("Apply"), this))
    , m_cancelButton(new QPushButton(tr("Cancel"), this))
    , m_openInputMonitorButton(new QPushButton(tr("Open Input Monitor"), this)) {
    initControlAttribute();
    initLayout();
    initSigSlots();
    loadFromControllerProfile();
}

void SettingsWindow::closeEvent(QCloseEvent* event) {
    if (!event) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: event";
        return;
    }

    hide();
    event->ignore();
}

void SettingsWindow::initControlAttribute() {
    do {
        setWindowTitle(tr("PadNav Settings"));
        resize(420, 520);
    } while (false);

    if (m_deadZoneSpinBox) {
        m_deadZoneSpinBox->setRange(0, 40);
        m_deadZoneSpinBox->setSuffix(tr("%"));
    }

    if (m_mouseSpeedSpinBox) {
        m_mouseSpeedSpinBox->setRange(300, 2400);
        m_mouseSpeedSpinBox->setSuffix(tr(" px/s"));
    }

    if (m_scrollSpeedSpinBox) {
        m_scrollSpeedSpinBox->setRange(120, 1440);
        m_scrollSpeedSpinBox->setSuffix(tr(" wheel units/s"));
    }
}

void SettingsWindow::initLayout() {
    auto* rootLayout = new QVBoxLayout(this);

    auto* profileLayout = new QFormLayout();
    profileLayout->addRow(tr("Dead Zone"), m_deadZoneSpinBox);
    profileLayout->addRow(tr("Mouse Speed"), m_mouseSpeedSpinBox);
    profileLayout->addRow(tr("Scroll Speed"), m_scrollSpeedSpinBox);
    rootLayout->addLayout(profileLayout);

    auto* actionsGroupBox = new QGroupBox(tr("Chrome Actions"), this);
    auto* actionsLayout = new QVBoxLayout(actionsGroupBox);
    for (auto index = std::size_t{}; index < actionDisplays.size(); ++index) {
        const auto& action = actionDisplays.at(index);
        auto* checkBox = new QCheckBox(tr(action.text), actionsGroupBox);
        m_actionCheckBoxes.at(index) = checkBox;
        actionsLayout->addWidget(checkBox);
    }

    auto* actionsScrollArea = new QScrollArea(this);
    actionsScrollArea->setWidgetResizable(true);
    actionsScrollArea->setWidget(actionsGroupBox);
    rootLayout->addWidget(actionsScrollArea, 1);

    auto* monitorLayout = new QHBoxLayout();
    monitorLayout->addWidget(m_openInputMonitorButton);
    monitorLayout->addStretch(1);
    rootLayout->addLayout(monitorLayout);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);
    rootLayout->addLayout(buttonLayout);
}

void SettingsWindow::initSigSlots() {
    if (!m_applyButton || !m_cancelButton || !m_openInputMonitorButton) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: settings buttons";
        return;
    }

    connect(m_applyButton, &QPushButton::clicked, this, &SettingsWindow::onApply);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsWindow::onCancel);
    connect(m_openInputMonitorButton, &QPushButton::clicked, this, &SettingsWindow::onOpenInputMonitor);
}

void SettingsWindow::loadFromControllerProfile() {
    setControlsFromProfile(m_controller.profile());
}

void SettingsWindow::setControlsFromProfile(const pn::core::ChromeProfile& profile) {
    if (!m_deadZoneSpinBox || !m_mouseSpeedSpinBox || !m_scrollSpeedSpinBox) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: settings spin boxes";
        return;
    }

    m_deadZoneSpinBox->setValue(profile.deadZonePercent);
    m_mouseSpeedSpinBox->setValue(profile.mouseMaxSpeedPixelsPerSecond);
    m_scrollSpeedSpinBox->setValue(profile.scrollSpeedWheelUnitsPerSecond);

    for (auto index = std::size_t{}; index < actionDisplays.size(); ++index) {
        auto* checkBox = m_actionCheckBoxes.at(index);
        if (!checkBox) {
            qDebug() << __FILE__ << __LINE__ << "null pointer error: action checkbox";
            return;
        }

        checkBox->setChecked(pn::core::isEnabled(profile, actionDisplays.at(index).id));
    }
}

auto SettingsWindow::profileFromControls() const -> pn::core::ChromeProfile {
    auto profile = pn::core::makeDefaultChromeProfile();

    if (m_deadZoneSpinBox && m_mouseSpeedSpinBox && m_scrollSpeedSpinBox) {
        profile.deadZonePercent = m_deadZoneSpinBox->value();
        profile.mouseMaxSpeedPixelsPerSecond = m_mouseSpeedSpinBox->value();
        profile.scrollSpeedWheelUnitsPerSecond = m_scrollSpeedSpinBox->value();
    }

    for (auto index = std::size_t{}; index < actionDisplays.size(); ++index) {
        const auto* checkBox = m_actionCheckBoxes.at(index);
        profile.bindings.at(index) = pn::core::ActionBinding{
            .id = actionDisplays.at(index).id,
            .enabled = checkBox ? checkBox->isChecked() : true,
        };
    }

    return profile;
}

void SettingsWindow::onApply() {
    const auto profile = profileFromControls();
    if (!pn::core::isValid(profile)) {
        qDebug() << __FILE__ << __LINE__ << "invalid profile";
        return;
    }

    m_controller.applyProfile(profile);
    if (!m_repository.save(profile)) {
        qDebug() << __FILE__ << __LINE__ << "profile save failed";
    }
}

void SettingsWindow::onCancel() {
    loadFromControllerProfile();
    hide();
}

void SettingsWindow::onOpenInputMonitor() {
    m_monitorWindow.show();
    m_monitorWindow.raise();
    m_monitorWindow.activateWindow();
}

} // namespace pn::gui

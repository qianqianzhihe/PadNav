#include "traycontroller.h"

#include "Application/padnavcontroller.h"
#include "settingswindow.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QMetaObject>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>

#include <memory>

namespace pn::gui {

namespace {

constexpr auto iconSize = 64;
constexpr auto cornerRadius = 14;

} // namespace

TrayController::TrayController(pn::application::PadNavController& controller,
                               SettingsWindow& settingsWindow,
                               QApplication& application,
                               QObject* parent)
    : QObject(parent)
    , m_controller(controller)
    , m_settingsWindow(settingsWindow)
    , m_application(application) {
    initTrayIcon();
    initMenu();
    initSigSlots();
    showTrayIcon();
}

TrayController::~TrayController() = default;

void TrayController::initTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(createTrayIcon());
    m_trayIcon->setToolTip(tr("PadNav"));
}

void TrayController::initMenu() {
    if (!m_trayIcon) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: m_trayIcon";
        return;
    }

    m_menu = std::make_unique<QMenu>();
    m_menu->setObjectName(QStringLiteral("PadNavTrayMenu"));
    m_menu->setToolTipsVisible(true);

    m_toggleMappingAction = m_menu->addAction(QString{});
    m_openSettingsAction = m_menu->addAction(tr("Open settings"));
    m_menu->addSeparator();
    m_exitAction = m_menu->addAction(tr("Exit"));
    m_trayIcon->setContextMenu(m_menu.get());
    updateMappingActionText();
}

void TrayController::initSigSlots() {
    if (!m_menu || !m_toggleMappingAction || !m_openSettingsAction || !m_exitAction) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: tray actions";
        return;
    }

    connect(m_menu.get(), &QMenu::aboutToShow, this, &TrayController::updateMappingActionText);
    connect(m_toggleMappingAction, &QAction::triggered, this, [this]() {
        m_controller.setMappingEnabled(!m_controller.mappingEnabled());
        updateMappingActionText();
    });
    connect(m_openSettingsAction, &QAction::triggered, this, [this]() {
        m_settingsWindow.show();
        m_settingsWindow.raise();
        m_settingsWindow.activateWindow();
    });
    connect(m_exitAction, &QAction::triggered, this, [this]() {
        m_controller.stop();
        QMetaObject::invokeMethod(&m_application, "quit", Qt::QueuedConnection);
    });
}

void TrayController::showTrayIcon() {
    if (!m_trayIcon) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: m_trayIcon";
        return;
    }

    m_trayIcon->show();
}

void TrayController::updateMappingActionText() {
    if (!m_toggleMappingAction) {
        qDebug() << __FILE__ << __LINE__ << "null pointer error: m_toggleMappingAction";
        return;
    }

    m_toggleMappingAction->setText(m_controller.mappingEnabled() ? tr("Pause mapping") : tr("Enable mapping"));
}

auto TrayController::createTrayIcon() -> QIcon {
    auto pixmap = QPixmap(iconSize, iconSize);
    pixmap.fill(Qt::transparent);

    auto painter = QPainter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#8fbc8f")));
    painter.drawRoundedRect(pixmap.rect(), cornerRadius, cornerRadius);

    auto font = QFont(QStringLiteral("Segoe UI"));
    font.setBold(true);
    font.setPixelSize(30);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QStringLiteral("pn"));

    return {pixmap};
}

} // namespace pn::gui

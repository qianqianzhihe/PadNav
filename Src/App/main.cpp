#include <QApplication>
#include <QCoreApplication>

#include "Application/padnavcontroller.h"
#include "WindowsPlatform/crashdump.h"
#include "inputmonitorwindow.h"
#include "jsonprofilerepository.h"
#include "settingswindow.h"
#include "singleinstanceguard.h"
#include "traycontroller.h"

#ifdef _DEBUG
#include <iostream>
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName(QStringLiteral("PadNav"));
    QCoreApplication::setApplicationName(QStringLiteral("PadNav"));

    auto singleInstanceGuard = pn::app::SingleInstanceGuard(QStringLiteral("PadNav.SingleInstance"));
    if (!singleInstanceGuard.tryAcquire()) {
        return 0;
    }

    pn::platform::CrashDump::pruneOldDumps(3U);
    pn::platform::CrashDump::install();

#ifdef _DEBUG
    std::clog << "PadNav: startup\n";
#endif

    pn::application::PadNavController controller;
    pn::gui::JsonProfileRepository repository;
    controller.applyProfile(repository.load());
    controller.start();

    pn::gui::InputMonitorWindow inputMonitorWindow(controller);
    pn::gui::SettingsWindow settingsWindow(controller, repository, inputMonitorWindow);
    pn::gui::TrayController trayController(controller, settingsWindow, app);

    const auto result = QApplication::exec();
    controller.stop();
#ifdef _DEBUG
    std::clog << "PadNav: shutdown\n";
#endif
    return result;
}

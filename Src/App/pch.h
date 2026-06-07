#pragma once
#include <QObject>
#include <QDebug>

#define LOG_GG_ERROR qWarning ().noquote() << __FILE__ << ":" << __LINE__
#define LOG_GG_DEBUG qDebug().noquote() << __FILE__ << ":" << __LINE__
#define LOG_GG_INFO qInfo().noquote() << __FILE__ << ":" << __LINE__

#ifndef ANDROIDAUTO_LOGGING_H
#define ANDROIDAUTO_LOGGING_H

#include <QDebug>
#include <QLoggingCategory>

static const QLoggingCategory &LOG_PLUGINS_ANDROIDAUTO()
{
    static const QLoggingCategory category("plugins.android-auto");
    return category;
}

#endif

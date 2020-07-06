#ifndef FIREFOXPROFILERUNNER_UTILS_H
#define FIREFOXPROFILERUNNER_UTILS_H

#include <QString>
#include <QIcon>

QIcon getFirefoxIcon() {
    static auto icon = QIcon::fromTheme(QStringLiteral("firefox-esr") ,
                                QIcon::fromTheme(QStringLiteral("firefox")));
    return icon;
}

QIcon getFirefoxPrivateIcon() {
    auto static icon = QIcon::fromTheme(QStringLiteral("private_browsing_firefox"),
                                QIcon::fromTheme(QStringLiteral("view-private")));
    return icon;
}

#endif //FIREFOXPROFILERUNNER_UTILS_H

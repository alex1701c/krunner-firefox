#pragma once

#include <QMetaType>
#include <QString>

enum ProfileType {
    Normal,
    Private,
};

struct ProfileData {
    QString path;
    bool isDefault;
    ProfileType type;
    int priority;
};
Q_DECLARE_METATYPE(ProfileData)

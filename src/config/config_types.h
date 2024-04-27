#pragma once

#include <QMetaType>
#include <QString>

enum ProfileType {
    Normal,
    Private,
    ProxychainsNormal,
    ProxychainsPrivate,
    TMP,
};

struct ProfileData {
    QString path;
    bool isDefault;
    ProfileType type;
    int priority;
};
Q_DECLARE_METATYPE(ProfileData)

namespace Proxychains
{
enum ProxychainsSelection {
    Existing = 2,
    Extra = 1,
    None = -1,
    Disabled = 0,
};
}
Q_DECLARE_METATYPE(Proxychains::ProxychainsSelection)

struct ProxychainsData {
    QString path;
    ProfileType type;
    bool isDisplayed;
};
Q_DECLARE_METATYPE(ProxychainsData)

#ifndef CONFIG_TYPES_H
#define CONFIG_TYPES_H

enum ProfileType {
    Normal, Private, ProxychainsNormal, ProxychainsPrivate, TMP
};

struct ProfileData {
    QString path;
    bool isDefault;
    ProfileType type;
    int priority;
};
Q_DECLARE_METATYPE(ProfileData)

struct ProxychainsData {
    QString path;
    ProfileType type;
    bool isDisplayed;
};
Q_DECLARE_METATYPE(ProxychainsData)

#endif //CONFIG_TYPES_H

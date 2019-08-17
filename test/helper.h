#ifndef FIREFOXPROFILERUNNER_HELPER_H
#define FIREFOXPROFILERUNNER_HELPER_H

#include <QtCore/QString>

inline bool stringToBool(const QString &value) {
    return value == "true";
}

inline const char *boolToString(bool value) {
    return value ? "true" : "false";
}

inline int getSplitCount(const QString str) {
    return str.split(";", QString::SplitBehavior::SkipEmptyParts).count();
}

#endif //FIREFOXPROFILERUNNER_HELPER_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemberFunctionCanBeStatic"

#include <QTest>

class ProfileManagerTest : public QObject {
Q_OBJECT
private
    slots:

private slots:

    void initTestCase() {
        qDebug("Called before everything else.");
    }

    void myFirstTest() {
        QVERIFY(true);
        QCOMPARE(1, 1);
    }

    void mySecondTest() {
        QVERIFY(42 * 2 == 84);
        QVERIFY(1 != 2);
    }

    void cleanupTestCase() {
        qDebug("Called after myFirstTest and mySecondTest.");
    }
};

QTEST_MAIN(ProfileManagerTest)


#pragma clang diagnostic pop
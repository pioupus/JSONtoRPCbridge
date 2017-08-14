#ifndef JSONINPUTTEST_H
#define JSONINPUTTEST_H

#include "autotest.h"
#include <QObject>

class JsonInputTest : public QObject {
    Q_OBJECT

    public:
    private slots:
    void initTestCase();
    void cleanupTestCase();

    void JSonEscapingTest();
};

DECLARE_TEST(JsonInputTest)

#endif // JSONINPUTTEST_H

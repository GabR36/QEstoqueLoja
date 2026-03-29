#include "test_dbutil.h"
#include <QTest>
#include "util/dbutil.h"

void TestDbUtil::initTestCase() {}
void TestDbUtil::testConnection()
{
    DBUtil db;
    QVERIFY(db.isValido());
}
void TestDbUtil::cleanupTestCase() {}

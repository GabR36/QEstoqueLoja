#include <QCoreApplication>
#include <QtTest>

#include "db/test_db_factory.h"
#include "infra/databaseconnection_service.h"
#include "services/schemamigration_service.h"

#include "services/test_produto_service.h"
#include "services/test_barcode_service.h"
#include "util/test_dbutil.h"
#include "services/test_manifestadordfe.h"
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    int status = 0;
#ifdef TEST_ENV
    TestDbFactory::create();
#endif

    status |= QTest::qExec(new TestProdutoService, argc, argv);
    status |= QTest::qExec(new test_barcode_service, argc, argv);
    status |= QTest::qExec(new test_manifestadordfe, argc, argv);


    return status;
}

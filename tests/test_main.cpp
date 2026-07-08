#include <QCoreApplication>
#include <QtTest>

#include "db/test_db_factory.h"
#include "infra/databaseconnection_service.h"
#include "services/schemamigration_service.h"
#include "services/test_cliente_service.h"
#include "services/test_produto_service.h"
#include "services/test_barcode_service.h"
#include "util/test_dbutil.h"
#include "services/test_manifestadordfe.h"
#include "services/test_vendas_service.h"
#include "services/test_produtovenda_service.h"
#include "services/test_fiscalemitter_service.h"
#include "services/test_eventofiscal_service.h"
#include <QSqlDatabase>
#include <QDebug>
#include "nota/acbrmanager.h"

int main(int argc, char *argv[])
{
    AcbrManager::setTestMode(true);
    QApplication app(argc, argv);

    int status = 0;
#ifdef TEST_ENV
#ifdef TEST_POSTGRES
    // Se a varivel de compilação "TEST_WITH_POSTGRES=ON" estiver sendo usada, criará um banco de dados postgre usando as
    // credenciais definidas em variaveis de sistema (ver definição da função)
    TestDbFactory::createPostgres();
#else
    TestDbFactory::create();
#endif
#endif//TEST_ENV

    status |= QTest::qExec(new TestProdutoService, argc, argv);
    status |= QTest::qExec(new test_barcode_service, argc, argv);
    status |= QTest::qExec(new test_manifestadordfe, argc, argv);
    status |= QTest::qExec(new test_cliente_service, argc, argv);
    status |= QTest::qExec(new TestVendasService, argc, argv);
    status |= QTest::qExec(new TestProdutoVendaService, argc, argv);
    status |= QTest::qExec(new TestFiscalEmitterService, argc, argv);
    status |= QTest::qExec(new test_eventofiscal_service, argc, argv);

#ifdef TEST_POSTGRES
    TestDbFactory::removerBDAtual();
#endif

    return status;
}

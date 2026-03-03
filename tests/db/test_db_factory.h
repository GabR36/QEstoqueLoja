#ifndef TEST_DB_FACTORY_H
#define TEST_DB_FACTORY_H

#include <QSqlDatabase>

class TestDbFactory
{
public:
    static QSqlDatabase create();
};

#endif

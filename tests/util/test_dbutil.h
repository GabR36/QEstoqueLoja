#pragma once

#include <QObject>

class TestDbUtil : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testConnection();
    void cleanupTestCase();
};

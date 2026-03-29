#ifndef GRAFICOHELPER_H
#define GRAFICOHELPER_H

#include <QChart>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChartView>
#include <QWidget>
#include <QVBoxLayout>
#include <QStringList>
#include <QList>
#include <QPainter>
#include <algorithm>

namespace GraficoHelper {

inline QChartView* criarBarChart(
    const QString &titulo,
    const QStringList &categorias,
    QList<QBarSet*> barSets,
    double maxY = -1)
{
    QBarSeries *series = new QBarSeries();
    for (QBarSet *set : barSets) {
        series->append(set);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(titulo);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categorias);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    if (maxY < 0) {
        maxY = 1.0;
        for (QBarSet *set : barSets) {
            for (int i = 0; i < set->count(); ++i) {
                maxY = std::max(maxY, set->at(i));
            }
        }
    }

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxY);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    return chartView;
}

inline void inserirChartNaPagina(QWidget *pagina, QChartView *chartView, int primeiroRemovivel = 1)
{
    QLayout *layoutPagina = pagina->layout();

    if (!layoutPagina) {
        layoutPagina = new QVBoxLayout(pagina);
        pagina->setLayout(layoutPagina);
    }

    QLayoutItem *item;
    while ((item = layoutPagina->takeAt(primeiroRemovivel)) != nullptr) {
        delete item->widget();
        delete item;
    }

    layoutPagina->addWidget(chartView);
}

} // namespace GraficoHelper

#endif // GRAFICOHELPER_H

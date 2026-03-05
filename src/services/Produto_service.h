#ifndef PRODUTO_SERVICE_H
#define PRODUTO_SERVICE_H

#include <QSqlDatabase>
#include <QLocale>
#include "../dto/Produto_dto.h"
#include "../util/ibptutil.h"
#include <QSqlQueryModel>
#include "../repository/Produto_repository.h"

enum class ProdutoErro {
    Nenhum,
    CampoVazio,
    CodigoBarrasExistente,
    NcmInvalido,
    NcmAviso,
    ConversaoPrecoInvalida,
    QuantidadeInvalida,
    ErroBanco,
    PrecoInvalido,
    Update
};

class Produto_Service : QObject
{
    Q_OBJECT
public:
  struct Resultado {
    bool ok;
    ProdutoErro erro = ProdutoErro::Nenhum;
    QString msg;
  };
  explicit Produto_Service(QObject *parent = nullptr);

  Resultado validar(const ProdutoDTO &p);
  Resultado inserir(const ProdutoDTO &p);
  bool codigoBarrasExiste(const QString &codigo);

  double calcularPrecoFinal(double precoFornecedor, double percentualLucro);

  double calcularPercentualLucro(double precoFornecedor, double precoFinal);
  static double round2(double v);
  static float round2f(float v);
  QSqlQueryModel *listarProdutos();
  QSqlQueryModel *getProdutoPeloCodigo(const QString &codigoBarras);
  Resultado deletar(const QString &id);
  static QString normalizeText(const QString &text);
  QSqlQueryModel *pesquisar(const QString &texto);
  QSqlDatabase db;
  QLocale portugues;
  IbptUtil ibpt;
  ProdutoDTO converterDadosParaDB(const ProdutoDTO &p);
  Produto_Service::Resultado validarConversao(const ProdutoDTO &p);
  Produto_Repository repo;
  Resultado alterar(const ProdutoDTO &p, const QString &id);
  Resultado alterarVerificarCodigoBarras(const ProdutoDTO &p, const QString &codigo, const QString &id);
  QStringList obterSugestoesLocal();
  Resultado atualizarLocalProduto(qlonglong id, const QString &novoLocal);
  Produto_Service::Resultado updateAumentarQuantidadeProduto(qlonglong idprod, double quantia);
  ProdutoDTO getProduto(qlonglong id);
  private slots:
};

#endif

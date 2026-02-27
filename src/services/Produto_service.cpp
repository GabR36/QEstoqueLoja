#include "Produto_service.h"
#include <QSqlQuery>
#include <cmath>
Produto_Service::Produto_Service(QObject *parent)
    : QObject{parent},
    portugues(QLocale(QLocale::Portuguese, QLocale::Brazil))

{

}

Produto_Service::Resultado Produto_Service::validar(const ProdutoDTO &p)
{
    if (p.descricao.trimmed().isEmpty()) {
        return {false, ProdutoErro::CampoVazio, "O campo 'Descrição' não pode estar vazio."};
    }

    if (p.percentLucro <= 0) {
        return {false, ProdutoErro::CampoVazio, "O campo 'Percentual de Lucro' está incorreto."};
    }

    if (p.nf && (p.ncm.trimmed().isEmpty() || p.ncm.length() != 8)) {
        return {false, ProdutoErro::NcmInvalido, "NCM inválido para NF."};
    }

    return {true, ProdutoErro::Nenhum, ""};
}

bool Produto_Service::codigoBarrasExiste(const QString &codigo)
{
    return repo.codigoBarrasExiste(codigo);
}
ProdutoDTO Produto_Service::converterDadosParaDB(const ProdutoDTO &p)
{
    ProdutoDTO out = p; // cópia

    // normalização numérica
    out.preco           = round2(p.preco);
    out.quantidade      = round2(p.quantidade);
    out.precoFornecedor = round2(p.precoFornecedor);
    out.percentLucro    = round2(p.percentLucro);
    out.aliquotaIcms = round2(p.aliquotaIcms);

    return out;
}

double Produto_Service::round2(double v)
{
    return std::round(v * 100.0) / 100.0;
}

float Produto_Service::round2f(float v)
{
    return std::round(v * 100.0f) / 100.0f;
}
Produto_Service::Resultado Produto_Service::validarConversao(const ProdutoDTO &p)
{
    // validações básicas
    if (p.preco < 0) {
        return {false, ProdutoErro::PrecoInvalido,
                "Por favor, insira um preço válido."};
    }

    if (p.quantidade <= 0) {
        return {false, ProdutoErro::QuantidadeInvalida,
                "Por favor, insira uma quantidade válida."};
    }

    // arredondamento
    double preco = round2(p.preco);
    float quant  = round2(p.quantidade);

    qDebug() << "[validarConversao]";
    qDebug() << "preco in :" << p.preco << " -> out:" << preco;
    qDebug() << "quant in :" << p.quantidade << " -> out:" << quant;

    return {true, ProdutoErro::Nenhum, ""};
}

double Produto_Service::calcularPrecoFinal(double precoFornecedor, double percentualLucro)
{
    return precoFornecedor * (1.0 + percentualLucro / 100.0);
}


double Produto_Service::calcularPercentualLucro(double precoFornecedor, double precoFinal)
{
    return ((precoFinal - precoFornecedor) / precoFornecedor) * 100.0;
}

Produto_Service::Resultado Produto_Service::inserir(const ProdutoDTO &p)
{
    Resultado v1 = validar(p);
    if (!v1.ok) return v1;

    Resultado v2 = validarConversao(p);
    if (!v2.ok) return v2;

    if (repo.codigoBarrasExiste(p.codigoBarras)) {
        return {false, ProdutoErro::CodigoBarrasExistente,
                "Código de barras já existe."};
    }

    ProdutoDTO dbData = converterDadosParaDB(p);

    QString erroSQL;
    if (!repo.inserir(dbData, erroSQL)) {
        return {false, ProdutoErro::ErroBanco, erroSQL};
    }

    return {true, ProdutoErro::Nenhum, ""};
}

Produto_Service::Resultado Produto_Service::deletar(const QString &id){
    QString errosql = "";

    if(!repo.deletar(id, errosql)){
        return {false, ProdutoErro::ErroBanco, errosql};
    }else{
        return {true, ProdutoErro::Nenhum, ""};
    }

}

QSqlQueryModel* Produto_Service::listarProdutos()
{
    return repo.listarProdutos();
}

QSqlQueryModel* Produto_Service::getProdutoPeloCodigo(const QString &codigoBarras){
    return repo.getProdutoPeloCodigo(codigoBarras);
}

QString Produto_Service::normalizeText(const QString &text) {
    QString normalized = text.normalized(QString::NormalizationForm_D);
    QString result;
    for (const QChar &c : normalized) {
        if (!c.isMark()) {
            QChar replacement;
            switch (c.unicode()) {
            case ';':
            case '\'':
            case '\"':
                // Remover os caracteres ; ' "
                continue;
            case '<':
                replacement = '(';
                break;
            case '>':
                replacement = ')';
                break;
            case '&':
                replacement = 'e';
                break;
            default:
                result.append(c.toUpper());
                continue;
            }
            result.append(replacement);
        }
    }
    return result;

}

QSqlQueryModel* Produto_Service::pesquisar(const QString &texto)
{
    QString normalizado = normalizeText(texto);

    if (normalizado.trimmed().isEmpty()) {
        return repo.listarProdutos(); // fallback padrão
    }

    QStringList palavras = normalizado.split(" ", Qt::SkipEmptyParts);

    return repo.pesquisar(palavras, normalizado);
}

Produto_Service::Resultado Produto_Service::alterarVerificarCodigoBarras(const ProdutoDTO &p,
                                                                         const QString &codigo,
                                                                         const QString &id){
    Resultado v1 = validar(p);
    if (!v1.ok) return v1;

    Resultado v2 = validarConversao(p);
    if (!v2.ok) return v2;

    if(codigo != p.codigoBarras){
        if (repo.codigoBarrasExiste(p.codigoBarras)) {
            return {false, ProdutoErro::CodigoBarrasExistente,
                    "Esse código de barras já foi registrado."};
        }

    }



    ProdutoDTO dbData = converterDadosParaDB(p);

    QString erroSQL;
    if (!repo.alterar(dbData, id, erroSQL)) {
        return {false, ProdutoErro::ErroBanco, erroSQL};
    }

    return {true, ProdutoErro::Nenhum, ""};
}

Produto_Service::Resultado Produto_Service::alterar(const ProdutoDTO &p, const QString &id){
    Resultado v1 = validar(p);
    if (!v1.ok) return v1;

    Resultado v2 = validarConversao(p);
    if (!v2.ok) return v2;

    if (repo.codigoBarrasExiste(p.codigoBarras)) {
        return {false, ProdutoErro::CodigoBarrasExistente,
                "Esse código de barras já foi registrado."};
    }


    ProdutoDTO dbData = converterDadosParaDB(p);

    QString erroSQL;
    if (!repo.alterar(dbData, id, erroSQL)) {
        return {false, ProdutoErro::ErroBanco, erroSQL};
    }

    return {true, ProdutoErro::Nenhum, ""};
}

QStringList Produto_Service::obterSugestoesLocal()
{
    return repo.listarLocais();
}

Produto_Service::Resultado Produto_Service::atualizarLocalProduto(qlonglong id, const QString &novoLocal)
{
    if (novoLocal.trimmed().isEmpty()) {
        return {false, ProdutoErro::CampoVazio, "Local não pode ser vazio."};
    }

    QString erroSQL;
    if (!repo.atualizarLocal(id, novoLocal.trimmed(), erroSQL)) {
        return {false, ProdutoErro::ErroBanco, erroSQL};
    }

    return {true, ProdutoErro::Nenhum, ""};
}

Produto_Service::Resultado Produto_Service::updateAumentarQuantidadeProduto(qlonglong idprod,
                                                                            double quantia ){
    if(!repo.updateAumentarQuantidadeProduto(idprod, quantia)){
        return {false, ProdutoErro::Update, "Erro ao atualizar quantia produto."};
    }else{
        return {true, ProdutoErro::Nenhum, "Quantia atualizada com sucesso"};
    }
}


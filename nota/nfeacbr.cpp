#include "nfeacbr.h"
#include "../configuracao.h"
#include <QStandardPaths>
#include <qdir.h>
#include <fstream>
#include <qrandom.h>
#include <QSqlError>

NfeACBR::NfeACBR(QObject *parent, bool saida, bool devolucao)
    : QObject{parent}
{
    //pega o ponteiro da lib acbr do singleton
    nfe = AcbrManager::instance()->nfe();
    db = QSqlDatabase::database();
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    produtosValues = Configuracao::get_All_Produto_Values();


    // se a nota é de devolução e entrada
    if(!saida && devolucao){

        tipo = devolucaoVenda;
        natOp = "Retorno de mercadoria de venda varejo";
        tpNf = "0"; //entrada = 0
        finNfe = "4"; //devolucao = 4
        cfop = "1202";
        tPagNf = "90";
    }else if(saida && !devolucao){                          //se a nota é venda de mercadoria normal
        tipo = saidaNormal;
        natOp = "Venda de Mercadoria Adquirida";
        tpNf = "1"; //saida = 1
        finNfe = "1"; //nfe normal = 1
        cfop = "5102";
        tPagNf = "01"; //01 = dinheiro, usar setpagamento
    }else if(saida && devolucao){
        tipo = devolucaoFornecedor;
        natOp = "DEVOLUCAO COMPRA PARA COMERCIALIZACAO EM OPERACAO";
        tpNf = "1";
        finNfe = "4";
        cfop = "";
        tPagNf = "90";

    }

    cnpjEmit = empresaValues.value("cnpj_empresa").toStdString();
    //valores padrao
    serieNf = "1";
    tpEmis = 1;
    tpAmb = (fiscalValues.value("tp_amb") == "0" ? "1" : "0");

    caminhoXml = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/xmlNf";
    nfe->LimparLista();//evita acumular notas

    usarIBS = fiscalValues.value("usar_ibs").toInt();

}

void NfeACBR::setNfRef(QString chnfe){
    refNfe = chnfe;
}

int NfeACBR::getNNF(){
    return std::stoi(nnf);
}
QString NfeACBR::getChaveNf(){
    std::string raw = cnf;
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    QString cnfString = QString::fromStdString(raw).trimmed();
    return cnfString;
}
int NfeACBR::getSerie(){
    return serieNf.toInt();
}
QString NfeACBR::getXmlPath() {
    // Obtém o caminho bruto do ACBR
    std::string raw = nfe->GetPath(0);

    // Remove caracteres nulos (caso GetPath use buffer fixo)
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());

    // Converte para QString e remove espaços extras
    QString path = QString::fromStdString(raw).trimmed();

    //Normaliza saída do ACBR (corrige barras no Windows)
    path.replace("\\", "/");           // converte todas as barras invertidas
    path = QDir::cleanPath(path);      // remove barras duplas
    if (path.endsWith('/'))
        path.chop(1);                  // remove barra final, se houver

    // Trata o CNF (chave numérica da nota)
    raw = cnf;
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    QString cnfString = QString::fromStdString(raw).trimmed();

    // Monta o caminho final do XML
    QString ret = QString("%1/%2-nfe.xml").arg(path, cnfString);

    return ret;
}



QString NfeACBR::getVersaoLib(){
    return QString::fromStdString(nfe->Versao());
}

int NfeACBR::getProximoNNF(){
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados em getProximoNNF";
        return -1;
    }

    int tpAmb = fiscalValues.value("tp_amb").toInt(); // 1 = produção, 0 = homologação
    QString chaveConfig = (tpAmb == 0) ? "nnf_homolog" : "nnf_prod";
    QSqlQuery query;
    query.prepare(
        "SELECT nnf FROM notas_fiscais "
        "WHERE modelo = :modelo AND serie = :serie AND tp_amb = :tp_amb "
        "ORDER BY nnf DESC "
        "LIMIT 1"
        );
    query.bindValue(":modelo", "55");             // NF-e
    query.bindValue(":serie", serieNf);           // série
    query.bindValue(":tp_amb", tpAmb);            // ambiente atual

    if (query.exec()) {
        if (query.next()) {
            int ultimoNNF = query.value(0).toInt();
            return ultimoNNF + 1;
        } else {
            // Nenhuma nota no banco para este ambiente usa valor configurado manualmente
            bool ok;
            int ultimaConfigurada = fiscalValues.value(chaveConfig).toInt(&ok);
            if (ok && ultimaConfigurada > 0) {
                return ultimaConfigurada + 1;
            } else {
                return 1; // valor de fallback
            }
        }
    } else {
        qWarning() << "Erro na consulta NNF:" << query.lastError().text();
        return -1;
    }


}
QString NfeACBR::getCnpjEmit(){
    return QString::fromStdString(cnpjEmit);
}

QString NfeACBR::getTpAmb(){
    QString tpAmbBD = (QString::fromStdString(tpAmb) == "1" ? "0" : "1");

    return tpAmbBD;
}

QString NfeACBR::getCuf(){
    return QString::fromStdString(cuf);
}
double NfeACBR::getVNF(){
    return vNf;
}

void NfeACBR::setNNF(int nNF){
    qDebug() << "Setando NNF para:" << nNF;
    nnf = std::to_string(nNF);
    qDebug() << "NNF após set:" << QString::fromStdString(nnf);
}

void NfeACBR::setCliente(bool ehPf, QString cpf, QString nome, int indiedest,
                          QString email, QString lgr, QString nro, QString bairro, QString cmun, QString xmun,
                          QString uf, QString cep, QString ie){

    ehPfCli = ehPf;
    nomeCli = nome;
    indiedestCli = indiedest;
    emailCli = email;
    lgrCli = lgr;
    nroCli = nro;
    bairroCli = bairro;
    cmunCli = cmun;
    xmunCli = xmun;
    ufCli = uf;
    cepCli = cep;
    ieCli = ie;
    cpfCli = cpf;
    qDebug() << cpfCli << ehPfCli;

}

bool NfeACBR::isValidGTIN(const QString& gtin) {
    QString clean = gtin.trimmed();

    // Verifica se é numérico e tamanho válido
    if (clean.isEmpty() || !clean.contains(QRegularExpression("^\\d{8}$|^\\d{12}$|^\\d{13}$|^\\d{14}$"))) {
        return false;
    }

    // Cálculo do dígito verificador (mod 10, peso 3-1)
    int sum = 0;
    for (int i = clean.length() - 2; i >= 0; --i) {
        int digit = clean[i].digitValue();
        sum += digit * (((clean.length() - 2 - i) % 2 == 0) ? 3 : 1);
    }
    int dv = (10 - (sum % 10)) % 10;
    return dv == clean.right(1).toInt();
}

QString NfeACBR::cfopDevolucao(const QString &cfopOriginal)
{
    if (cfopOriginal.isEmpty() || cfopOriginal.length() < 4)
        return ""; // inválido

    QChar primeira = cfopOriginal.at(0);

    // devolução de compra -> sempre x411
    if (primeira == '6')
        return "6411";

    if (primeira == '5')
        return "5411";

    // Entrada (1xxx ou 2xxx) também gera nota de saída
    if (primeira == '1' || primeira == '2')
        return "5411"; // devolução interna por padrão

    // fallback (caso improvável)
    return "5411";
}

void NfeACBR::setProdutosNota(QList<qlonglong> &idsProduto)
{
    if (!db.isOpen()) {
        if(!db.open()){
            qDebug() << "Erro ao abrir banco em setProdutosNota()";
            return;
        }
    }

    QLocale usa(QLocale::English, QLocale::UnitedStates);
    QList<QList<QVariant>> produtosFinal;

    for (int id_prod_nota : idsProduto)
    {
        QSqlQuery q(db);

        // -------------------------------
        // BUSCA DADOS DO produtos_nota
        // -------------------------------
        q.prepare(R"(
            SELECT descricao,
                   quantidade,
                   preco,
                   codigo_barras,
                   un_comercial,
                   ncm,
                   cfop,
                   csosn,
                   pis,
                    aliquota_imposto
            FROM produtos_nota
            WHERE id = :id
        )");
        q.bindValue(":id", id_prod_nota);

        if (!q.exec() || !q.next()) {
            qWarning() << "Produto_nota não encontrado:" << id_prod_nota;
            continue;
        }

        QString descricao     = q.value("descricao").toString();
        QString quantidadeStr = q.value("quantidade").toString();
        QString precoStr      = q.value("preco").toString();
        QString codBarras     = q.value("codigo_barras").toString();
        QString unComercial   = q.value("un_comercial").toString();
        QString ncm           = q.value("ncm").toString();
        QString csosn         = q.value("csosn").toString();
        QString pis           = q.value("pis").toString();
        QString aliquota      = q.value("aliquota_imposto").toString();
        QString cfop          = q.value("cfop").toString();

        // -------------------------------
        // CONVERSÕES
        // -------------------------------
        double quantidade    = usa.toDouble(quantidadeStr);
        double precoUnitario = usa.toDouble(precoStr);
        double valorTotal    = quantidade * precoUnitario;

        // GTIN obrigatório formatado
        if (!isValidGTIN(codBarras))
            codBarras = "SEM GTIN";

        double aliquotaImposto = aliquota.toDouble();
        QString cfopFinal;
        if(tipo == devolucaoFornecedor){
            cfopFinal = cfopDevolucao(cfop);
            qDebug() << "CfopFinal:" << cfopFinal;
        }

        // -------------------------------
        // BUSCAR CEST E ALIQUOTAS EM PRODUTOS (se existir)
        // -------------------------------

        // montar lista EXACTAMENTE igual ao setProdutosVendidos()
        QList<QVariant> produto;

        produto << id_prod_nota;        // [0]
        produto << quantidade;          // [1]
        produto << descricao;           // [2]
        produto << precoUnitario;       // [3]
        produto << valorTotal;          // [4]
        produto << codBarras;           // [5]
        produto << unComercial;         // [6]
        produto << ncm;                 // [7]
        produto << "";                  // [8] CEST opcional (vazio por padrão)
        produto << aliquotaImposto;     // [9]
        produto << csosn;               // [10]
        produto << pis;                 // [11]
        produto << cfopFinal;           // [12]

        //manda o ultimo cfop para global apenas para verificar se é interestadual
        this->cfop = cfopFinal;

        produtosFinal.append(produto);
    }

    listaProdutos = produtosFinal;
    quantProds = produtosFinal.size();
    vTotTribProduto.resize(produtosFinal.size());

    qDebug() << "Produtos carregados de produtos_nota:" << quantProds;
}

void NfeACBR::setProdutosVendidos(QList<QList<QVariant>> produtosVendidos, bool emitirTodos){
    if (!db.open()) {
        qDebug() << "não abriu bd setprodutosvendidos";
        return;
    }

    emitirApenasNf = !emitirTodos;

    QLocale usa(QLocale::English, QLocale::UnitedStates);  // <<< NOVO

    QList<QList<QVariant>> produtosFiltrados;

    for (int i = 0; i < produtosVendidos.size(); ++i) {
        QList<QVariant> produto = produtosVendidos[i];

        // Verificação do campo nf
        QSqlQuery nfQuery;
        nfQuery.prepare("SELECT nf FROM produtos WHERE id = :idprod");
        nfQuery.bindValue(":idprod", produto[0]);
        if (!nfQuery.exec() || !nfQuery.next()) {
            qWarning() << "Erro ao consultar campo 'nf' do produto ID:" << produto[0].toString()
            << nfQuery.lastError().text();
            continue;
        }

        int nf = nfQuery.value("nf").toInt();
        if (!emitirTodos && nf != 1) {
            continue;
        }

        // Agora os valores já vêm no formato americano (3.50, 12.90)
        QString quantidadeStr    = produto[1].toString();
        QString valorUnitarioStr = produto[3].toString();

        // Converter usando locale USA
        double quantidade    = usa.toDouble(quantidadeStr);
        double valorUnitario = usa.toDouble(valorUnitarioStr);
        double valorTotal    = quantidade * valorUnitario;

        produto[1] = quantidade;
        produto[3] = valorUnitario;
        produto.append(QVariant(valorTotal));   // [4] valor total

        // Consulta adicional
        QSqlQuery query;
        query.prepare("SELECT codigo_barras, un_comercial, ncm, cest, csosn, pis,"
                      " aliquota_imposto FROM produtos WHERE id = :idprod");
        query.bindValue(":idprod", produto[0]);

        if (query.exec() && query.next()) {
            QString codigoBarras   = query.value("codigo_barras").toString();
            QString unComercial    = query.value("un_comercial").toString();
            QString ncm            = query.value("ncm").toString();
            QString cest           = query.value("cest").toString();
            QString csosn          = query.value("csosn").toString();
            QString pis            = query.value("pis").toString();
            double aliquotaImposto = query.value("aliquota_imposto").toDouble();
            QString cfopFinal = "5102";

            if (!isValidGTIN(codigoBarras)) {
                codigoBarras = "SEM GTIN";
            }
            if(tipo == devolucaoVenda){
                cfopFinal = "1202";
            }else if(tipo == devolucaoFornecedor){
                cfopFinal = "5411";
            }else if(tipo == saidaNormal){
                cfopFinal = "5102";
            }

            produto.append(codigoBarras);     // [5]
            produto.append(unComercial);      // [6]
            produto.append(ncm);              // [7]
            produto.append(cest);             // [8]
            produto.append(aliquotaImposto);  // [9]
            produto.append(csosn);            // [10]
            produto.append(pis);              // [11]
            produto.append(cfopFinal);        // [12]


        } else {
            qWarning() << "Produto ID não encontrado:" << produto[0].toString()
                       << query.lastError().text();

            produto.append("");       // [5]
            produto.append("");       // [6]
            produto.append("");       // [7]
            produto.append("");       // [8]
            produto.append(0.0);      // [9]
            produto.append("");       // [10]
            produto.append("");       // [11]
            produto.append("");         //[12]
        }

        produtosFiltrados.append(produto);
    }

    quantProds = produtosFiltrados.size();
    vTotTribProduto.resize(produtosFiltrados.size());
    listaProdutos = produtosFiltrados;
}

float NfeACBR::corrigirTaxa(float taxaAntiga, float desconto){
    float taxaConvertida = (taxaAntiga / 100) + 1;
    float taxaNova = 0.0;
    float valorTotalProdutos = 0.0;
    for (int i = 0; i < listaProdutos.size(); ++i) {
        valorTotalProdutos += listaProdutos[i][4].toDouble();
    }
    taxaNova = (valorTotalProdutos - desconto) * taxaConvertida + desconto;
    return ((taxaNova/valorTotalProdutos) - 1) * 100;
}


void NfeACBR::aplicarDescontoTotal(float descontoTotal) {
    descontoProd.clear();  // Limpa o vetor, caso já tenha valores anteriores

    double totalGeral = 0.0;

    // 1. Soma total da venda (valor dos produtos)
    for (const QList<QVariant>& produto : listaProdutos) {
        totalGeral += produto[4].toDouble(); // produto[4] é valorTotal
    }

    double descontoAcumulado = 0.0;

    for (int i = 0; i < listaProdutos.size(); ++i) {
        double descontoItem = 0.0;
        double valorTotalProduto = listaProdutos[i][4].toDouble();

        if (i < listaProdutos.size() - 1) {
            // 2. Calcula desconto proporcional e arredonda com 2 casas
            double proporcao = valorTotalProduto / totalGeral;
            descontoItem = qRound64(descontoNf * proporcao * 100.0) / 100.0;
            descontoAcumulado += descontoItem;
        } else {
            // 3. Último produto compensa a diferença para fechar o total
            descontoItem = qRound64((descontoNf - descontoAcumulado) * 100.0) / 100.0;
        }

        // 4. Adiciona no vetor
        descontoProd.append(descontoItem);
    }
}

void NfeACBR::setPagamentoValores(QString formaPag, float desconto,float recebido, float troco, float taxa){
    if(formaPag == "Prazo"){
        indPagNf = "1";
        tPagNf = "15";
    }else if(formaPag == "Dinheiro"){
        indPagNf = "0";
        tPagNf = "01";
    }else if(formaPag == "Crédito"){
        indPagNf = "0";
        tPagNf = "03";
    }else if(formaPag == "Débito"){
        indPagNf = "0";
        tPagNf = "04";
    }else if(formaPag == "Pix"){
        indPagNf = "0";
        tPagNf = "17";
    }else if(formaPag == "Não Sei"){
        indPagNf = "0";
        tPagNf = "01";
    }
    taxaPercentual = corrigirTaxa(taxa, desconto);
    vPagNf = recebido;
    trocoNf = troco;
    descontoNf = desconto; //corrigirDescontoParaAplicacaoPosTaxa(desconto, taxa);

    qDebug() << "valores setpagamento " << formaPag << desconto << recebido << troco << taxa;

}

void NfeACBR::aplicarAcrescimoProporcional(float taxaPercentual)
{
    double totalOriginal = 0.0;
    for (const QList<QVariant>& produto : listaProdutos) {
        double valorTotalProd = produto[4].toDouble();
        totalOriginal += valorTotalProd;
    }

    double totalComAcrescimo = qRound64(totalOriginal * (1.0 + taxaPercentual / 100.0) * 100.0) / 100.0;
    double somaDistribuida = 0.0;

    for (int i = 0; i < listaProdutos.size(); ++i) {
        QList<QVariant>& produto = listaProdutos[i];

        float quant = produto[1].toFloat();
        double valorTotalProdOriginal = produto[4].toDouble();

        // Distribui proporcionalmente com base no total original
        double proporcao = valorTotalProdOriginal / totalOriginal;
        double valorTotalComAcrescimo = qRound64((totalComAcrescimo * proporcao) * 100.0) / 100.0;
        double valorUnitarioComAcrescimo = qRound64((valorTotalComAcrescimo / quant) * 100.0) / 100.0;

        produto[4] = valorTotalComAcrescimo;
        produto[3] = valorUnitarioComAcrescimo;

        somaDistribuida += valorTotalComAcrescimo;
    }

    // Compensar diferença de centavos no último item
    double diferenca = totalComAcrescimo - somaDistribuida;
    if (!listaProdutos.isEmpty()) {
        QList<QVariant>& ultimo = listaProdutos.last();
        float quant = ultimo[1].toFloat();

        double novoTotal = ultimo[4].toDouble() + diferenca;
        double novoUnit = qRound64((novoTotal / quant) * 100.0) / 100.0;

        ultimo[4] = novoTotal;
        ultimo[3] = novoUnit;
    }
}
void NfeACBR::carregarConfig(){
    nfe->ConfigGravarValor("NFe", "ModeloDF", "0");  // NFe = 0
    nfe->ConfigGravarValor("NFe", "VersaoDF", "3");
    nfe->ConfigGravarValor("NFe", "VersaoQRCode", "3");
    nfe->ConfigGravarValor("NFe", "FormaEmissao", "0");
    nfe->ConfigGravarValor("NFe", "Ambiente", tpAmb);
}
QString NfeACBR::getDhEmiConvertida(){
    QString dhemi = QString::fromStdString(dataHora);
    QDateTime dt = QDateTime::fromString(dhemi, "dd/MM/yyyy hh:mm");
    QString dhemiConvertida = dt.toString("yyyy-MM-dd HH:mm:ss");

    return dhemiConvertida;
}


void NfeACBR::ide()
{
    cuf = fiscalValues.value("cuf").toStdString();
    std::string data = QDateTime::currentDateTime().toString("dd/MM/yyyy").toStdString();
    dataHora = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm").toStdString();
    mod = 55;
    int numeroAleatorio8dig = QRandomGenerator::global()->bounded(10000000, 99999999);
    if (nnf == "") {
        qDebug() << "Erro: NNF não foi definido!";
        return;
    }else{
        qDebug() << "NNF foi definido!";

    }
    cnf = nfe->GerarChave(
        std::stoi(cuf), numeroAleatorio8dig, mod,
        serieNf.toInt(), std::stoi(nnf), tpEmis, data, cnpjEmit
        );
    std::string tpAmbIde = (tpAmb == "1") ? "2" : "1";
    qDebug() << "TPAMBIDE:" << tpAmbIde + "tpamb:" << tpAmb;
    qDebug() << "cnf:" <<  std::to_string(numeroAleatorio8dig);
    idDest = "1";

    if(tipo == devolucaoFornecedor && cfop.startsWith("6")){
        idDest="2"; //operação interestadual
    }

    ini << "[Identificacao]\n";
    ini << "cUF=" << cuf << "\n";
    ini << "tpAmb=" << tpAmbIde << "\n";
    ini << "mod=" << QString::number(mod).toStdString() << "\n";
    ini << "serie=" << serieNf.toStdString() << "\n";
    ini << "nNF=" << nnf << "\n";
    ini << "cNF=" << std::to_string(numeroAleatorio8dig) << "\n";
    ini << "dhEmi=" << dataHora << "\n";
    ini << "natOp=" << natOp.toStdString() << "\n";
    ini << "tpNF=" << tpNf.toStdString() << "\n";
    ini << "idDest=" << idDest.toStdString() << "\n";
    ini << "finNFe=" << finNfe.toStdString() << "\n";
    ini << "indFinal=1\n";
    ini << "indPres=1\n";
    ini << "tpImp=1\n"; //nfe
    ini << "procEmi=0\n";
    ini << "verProc=1.0\n\n";
}

void NfeACBR::nfRef(){
    ini << "[NFRef001]\n";
    ini << "refNFe=" << refNfe.toStdString() << "\n";
}

void NfeACBR::emite()
{
    std::string xNome = empresaValues.value("nome_empresa").toStdString();
    std::string xFant = empresaValues.value("nfant_empresa").toStdString();
    std::string ie = fiscalValues.value("iest").toStdString();
    std::string xLgr = empresaValues.value("endereco_empresa").toStdString();
    std::string nro = empresaValues.value("numero_empresa").toStdString();
    std::string cMun = fiscalValues.value("cmun").toStdString();
    std::string xMun = empresaValues.value("cidade_empresa").toStdString();
    std::string uf = empresaValues.value("estado_empresa").toStdString();
    std::string fone = empresaValues.value("telefone_empresa").toStdString();
    std::string xBairro = empresaValues.value("bairro_empresa").toStdString();
    std::string cep = empresaValues.value("cep_empresa").toStdString();

    ini << "[Emitente]\n";
    ini << "CNPJCPF=" << cnpjEmit << "\n";
    ini << "xNome=" << xNome << "\n";
    ini << "xFant=" << xFant << "\n";
    ini << "IE=" << ie << "\n";
    ini << "CRT=1\n";
    ini << "xLgr=" << xLgr << "\n";
    ini << "nro=" << nro << "\n";
    ini << "xBairro=" << xBairro << "\n";
    ini << "cMun=" << cMun << "\n";
    ini << "xMun=" << xMun << "\n";
    ini << "CEP=" << cep << "\n";
    ini << "UF=" << uf << "\n";
    ini << "cPais=1058\n";
    ini << "xPais=BRASIL\n";
    ini << "Fone=" << fone << "\n\n";
}

void NfeACBR::dest()
{
    if(tipo == devolucaoVenda){
        std::string xNome = empresaValues.value("nome_empresa").toStdString();
        std::string xFant = empresaValues.value("nfant_empresa").toStdString();
        std::string ie = fiscalValues.value("iest").toStdString();
        std::string xLgr = empresaValues.value("endereco_empresa").toStdString();
        std::string nro = empresaValues.value("numero_empresa").toStdString();
        std::string cMun = fiscalValues.value("cmun").toStdString();
        std::string xMun = empresaValues.value("cidade_empresa").toStdString();
        std::string uf = empresaValues.value("estado_empresa").toStdString();
        std::string fone = empresaValues.value("telefone_empresa").toStdString();
        std::string xBairro = empresaValues.value("bairro_empresa").toStdString();
        std::string cep = empresaValues.value("cep_empresa").toStdString();

        ini << "[Destinatario]\n";
        ini << "CNPJCPF=" << cnpjEmit << "\n";
        ini << "xNome=" << xNome << "\n";
        ini << "xFant=" << xFant << "\n";
        ini << "IE=" << ie << "\n";

        ini << "IEST=\n";
        ini << "IM=\n";

        ini << "CRT=1\n";
        ini << "xLgr=" << xLgr << "\n";
        ini << "nro=" << nro << "\n";
        ini << "xBairro=" << xBairro << "\n";
        ini << "cMun=" << cMun << "\n";
        ini << "xMun=" << xMun << "\n";
        ini << "CEP=" << cep << "\n";
        ini << "UF=" << uf << "\n";
        ini << "cPais=1058\n";
        ini << "xPais=BRASIL\n";
        ini << "Fone=" << fone << "\n\n";

    }else{
        qDebug() << "cpfclie nfe: " << cpfCli;
        if(ehPfCli == false && cpfCli != ""){
            ini << "[Destinatario]\n";
            ini << "CNPJCPF=" << cpfCli.toStdString() << "\n";
        }

        if(ehPfCli == true && cpfCli != ""){
            ini << "[Destinatario]\n";
            ini << "CNPJCPF=" << cpfCli.toStdString() << "\n";
        }
        if(fiscalValues.value("tp_amb") == "0" && cpfCli != ""){
            ini << "xNome=NF-E EMITIDA EM AMBIENTE DE HOMOLOGACAO - SEM VALOR FISCAL\n";
        }
        // m_nfe->notafiscal->NFe->obj->infNFe->dest->set_CPF(""); //PARA PESSOA FISICA
        // //m_nfe->notafiscal->NFe->obj->infNFe->dest->set_idEstrangeiro("ID ESTRANGEIRO")
        ini << "xNome=" << nomeCli.toStdString() << "\n";


        switch (indiedestCli) {
        case 0:
            ini << "indIEDest=" << "9" << "\n";    //nao contribuinte
            break;
        case 1:
            ini << "indIEDest=" << "1" << "\n";    // contribuinte
            ini << "IE=" << ieCli.toStdString() << "\n";    // contribuinte
            break;
        case 2:
            ini << "indIEDest=" << "2" << "\n";    // contribuinte
            ini << "IE=" << ieCli.toStdString() << "\n";    // contribuinte
            break;
        default:
            qDebug() << "Valor indIEDest inválido:" << indiedestCli;
            // Opcional: Definir um padrão seguro
            ini << "indIEDest=" << "9" << "\n";    //nao contribuinte
            break;
        }

        ini << "Email=" << emailCli.toStdString() << "\n";
        // //Endereço
        ini << "xLgr=" << lgrCli.toStdString() << "\n";
        ini << "nro=" << nroCli.toStdString() << "\n";
        // //m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xCpl("complemento");
        ini << "xBairro=" << bairroCli.toStdString() << "\n";
        ini << "cMun=" << cmunCli.toStdString() << "\n";
        ini << "xMun=" << xmunCli.toStdString() << "\n";
        ini << "UF=" << ufCli.toStdString() << "\n";
        ini << "CEP=" << cepCli.toStdString() << "\n";
        ini << "cPais=" << "1058" << "\n";
        ini << "xPais=" << "Brasil" << "\n";
        // ini << "Fone=" << .toStdString() << "\n";

        ini << "\n";

    }

}


void NfeACBR::carregarProds()
{
    aplicarDescontoTotal(descontoNf);
    aplicarAcrescimoProporcional(taxaPercentual);

    for (int i = 0; i < listaProdutos.size(); ++i) {
        QList<QVariant> produto = listaProdutos[i];
        int item = i + 1;

        // Garante numeração com 3 dígitos (001, 002, 003...)
        std::ostringstream oss;
        oss << std::setw(3) << std::setfill('0') << item;
        std::string idx = oss.str();

        std::string secProduto = "[Produto" + idx + "]";
        std::string secICMS    = "[ICMS" + idx + "]";
        std::string secPIS     = "[PIS" + idx + "]";
        std::string secCOFINS  = "[COFINS" + idx + "]";
        std::string secIBS     = "[IBSCBS" + idx + "]";
        std::string secGIBS    = "[gIBSCBS" + idx + "]";
        std::string secGIBSUF  = "[gIBSUF" + idx + "]";
        std::string secGIBSMUN = "[gIBSMun" + idx + "]";
        std::string secGCBS    = "[gCBS" + idx + "]";

        QString cProd = produto[0].toString();
        QString qCom  = QString::number(produto[1].toDouble(), 'f', 2);
        QString xProd = produto[2].toString();
        QString vUnCom = QString::number(produto[3].toDouble(), 'f', 2);
        QString vProd = QString::number(produto[4].toDouble(), 'f', 2);
        QString cEAN = produto[5].toString();
        QString uCom = produto[6].toString();
        QString ncm  = produto[7].toString();
        QString csosn = produto[10].toString();
        QString pis = produto[11].toString();
        float vDesc = descontoProd[i];
        QString CFOP = produto[12].toString();
        qDebug() << "CFOP do produto:" << CFOP;
        if(tipo == devolucaoFornecedor){
            csosn = produtosValues.value("csosn_padrao");
            pis = produtosValues.value("pis_padrao");
        }

        ini << secProduto << "\n";
        ini << "CFOP=" << CFOP.toStdString() << "\n";
        ini << "cProd=" << cProd.toStdString() << "\n";
        ini << "cEAN=" << cEAN.toStdString() << "\n";
        ini << "xProd=" << xProd.toStdString() << "\n";
        ini << "NCM=" << ncm.toStdString() << "\n";
        ini << "uCom=" << uCom.toStdString() << "\n";
        ini << "qCom=" << qCom.toStdString() << "\n";
        ini << "vUnCom=" << vUnCom.toStdString() << "\n";
        ini << "vProd=" << vProd.toStdString() << "\n";
        ini << "vDesc=" << QString::number(vDesc, 'f', 2).toStdString() << "\n";
        ini << "indEscala=N\n";
        ini << "CNPJFab=\n";
        ini << "uTrib=" << uCom.toStdString() << "\n";
        ini << "cEANTrib=" << cEAN.toStdString() << "\n\n";

        ini << secICMS << "\n";
        ini << "CSOSN=" << csosn.toStdString() << "\n";
        ini << "Origem=0\n";
        ini << "vBC=0.00\n";
        ini << "pICMS=0.00\n";
        ini << "vICMS=0.00\n\n";

        ini << secPIS << "\n";
        ini << "CST=" << pis.toStdString() << "\n";
        ini << "vBC=0.00\n";
        ini << "pPIS=0.00\n";
        ini << "vPIS=0.00\n\n";

        ini << secCOFINS << "\n";
        ini << "CST=" << pis.toStdString() << "\n";
        ini << "vBC=0.00\n";
        ini << "pCOFINS=0.00\n";
        ini << "vCOFINS=0.00\n\n";

        if(usarIBS){
            ini << secIBS << "\n";
            ini << "CST=000\n";
            ini << "cClassTrib=000001\n\n";

            ini << secGIBS << "\n";
            ini << "vBC=" << vProd.toStdString() << "\n";
            ini << "vIBS=0.00\n\n";

            ini << secGIBSUF << "\n";
            ini << "pIBSUF=0.00\n";
            ini << "vIBSUF=0.00\n\n";

            ini << secGIBSMUN << "\n";
            ini << "pIBSMun=0.00\n";
            ini << "vIBSMun=0.00\n\n";

            ini << secGCBS << "\n";
            ini << "pCBS=0.00\n";
            ini << "vCBS=0.00\n\n";
        }
    }
}

void NfeACBR::total()
{
    totalGeral = 0.0;
    double vSeg = 0.0;
    double vFrete = 0.0;
    double totalTributo = 0.0;

    for (const QList<QVariant>& produto : listaProdutos) {
        if (produto.size() >= 5) {
            totalGeral += produto[4].toDouble();
        }
    }

    vNf = totalGeral + vSeg + vFrete - descontoNf;

    for (int i = 0; i < vTotTribProduto.size(); ++i) {
        totalTributo += vTotTribProduto[i];
    }

    ini << "[Total]\n";
    ini << "vBC=0,00\n";
    ini << "vICMS=0,00\n";
    ini << "vICMSDeson=0,00\n";
    ini << "vBCST=0,00\n";
    ini << "vST=0,00\n";
    ini << "vProd=" << QString::number(totalGeral, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "vFrete=" << QString::number(vFrete, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "vSeg=" << QString::number(vSeg, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "vDesc=" << QString::number(descontoNf, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "vIPI=0,00\n";
    ini << "vPIS=0,00\n";
    ini << "vCOFINS=0,00\n";
    ini << "vOutro=0,00\n";
    ini << "vNF=" << QString::number(vNf, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "vFCP=0,00\n";
    ini << "vTotTrib=" << QString::number(totalTributo, 'f', 2).replace('.', ',').toStdString() << "\n\n";
    if(usarIBS){
        ini << "[IBSCBSTot]\n";
        ini << "vBCIBSCBS=" << QString::number(totalGeral, 'f', 2).replace('.', ',').toStdString() << "\n\n";

        ini << "[gIBS]\n";
        ini << "vIBS=0,00\n";
        ini << "vCredPres=0,00\n";
        ini << "vCredPresCondSus=0,00\n\n";

        ini << "[gCBSTot]\n";
        ini << "vCBS=0,00\n";
        ini << "vCredPres=0,00\n";
        ini << "vCredPresCondSus=0,00\n\n";
    }

}


void NfeACBR::transp()
{
    ini << "[Transportador]\n";
    ini << "modFrete=9\n\n";
}

void NfeACBR::pag()
{
    if (emitirApenasNf) {
        vPagNf = vNf;
        trocoNf = 0.0;
    }
    if(tipo == devolucaoVenda){
        vPagNf = 0.0;
        trocoNf = 0.0;
    }else if(tipo == devolucaoFornecedor){
        trocoNf = 0.0;
    }

    std::string indice = "001";
    std::string secao = "pag" + indice;

    ini << "[" << secao << "]\n";
    ini << "indPag=" << indPagNf.toStdString() << "\n";
    ini << "tPag=" << tPagNf.toStdString() << "\n";
    ini << "xPag=\n";
    ini << "vPag=" << QString::number(vPagNf, 'f', 2).replace('.', ',').toStdString() << "\n";
    ini << "dPag=\n";
    ini << "CNPJPag=\n";
    ini << "UFPag=\n";

    if (tPagNf == "03" || tPagNf == "04" || tPagNf == "17") {
        ini << "tpIntegra=2\n";
        ini << "tBand=99\n";
        ini << "cAut=000000\n";
        ini << "CNPJ=\n";
        ini << "CNPJReceb=\n";
        ini << "idTermPag=\n";
    } else {
        ini << "tpIntegra=\n";
        ini << "tBand=\n";
        ini << "cAut=\n";
        ini << "CNPJ=\n";
        ini << "CNPJReceb=\n";
        ini << "idTermPag=\n";
    }

    ini << "vTroco=" << QString::number(trocoNf, 'f', 2).replace('.', ',').toStdString() << "\n\n";
}

void NfeACBR::infRespTec()
{
    std::string cnpjRT = fiscalValues.value("cnpj_rt").toStdString();
    std::string xContato = fiscalValues.value("nome_rt").toStdString();
    std::string emailRT = fiscalValues.value("email_rt").toStdString();
    std::string foneRT = fiscalValues.value("fone_rt").toStdString();

    ini << "[infRespTec]\n";
    ini << "CNPJ=" << cnpjRT << "\n";
    ini << "xContato=" << xContato << "\n";
    ini << "email=" << emailRT << "\n";
    ini << "fone=" << foneRT << "\n\n";
}

void NfeACBR::ibscbsTotais()
{
    ini << "[IBSCBSTot]\n";
    ini << "vBCIBSCBS=" << QString::number(totalGeral, 'f', 2).replace('.', ',').toStdString() << "\n\n";

    ini << "[gIBS]\n";
    ini << "vIBS=0,00\n";
    ini << "vCredPres=0,00\n";
    ini << "vCredPresCondSus=0,00\n\n";

    ini << "[gCBSTot]\n";
    ini << "vCBS=0,00\n";
    ini << "vCredPres=0,00\n";
    ini << "vCredPresCondSus=0,00\n\n";
}

QString NfeACBR::gerarEnviar(){

    ini.str("");  // Limpar conteúdo anterior
    ini.clear();

    ini << "[infNFe]\n";
    ini << "versao=4.00\n\n";
    carregarConfig();
    ide();
    if(tipo == devolucaoFornecedor){
        nfRef();
    }
    emite();
    dest();
    carregarProds();
    total();
    transp();
    pag();
    infRespTec();
    if(usarIBS){
        ibscbsTotais();
    }

    try {
        nfe->CarregarINI(ini.str());
        nfe->Assinar();
        nfe->GravarXml(0, "xml_naoaut_nota_"+ nnf + ".xml", "./xml");

        nfe->Validar();

        std::string retorno = nfe->Enviar(1, false, true, false);
        QString ret = QString::fromUtf8(retorno.c_str());
        qDebug() << "nfe getpath: " << nfe->GetPath(0);
        qDebug() << "nfe chave: " << cnf;

        qDebug() << "Retorno SEFAZ:" << ret;
        return ret;
        //nfce->Imprimir("", 1, "", true, std::nullopt, std::nullopt, std::nullopt);

    }
    catch (std::exception &e) {
        qDebug() << "Erro std::exception:" << e.what();
        return QString("Erro ao enviar nota %1:\n%2").arg(QString::fromStdString(nnf), e.what());
    }
    catch (...) {
        qDebug() << "Erro desconhecido!";
        return QString("Erro desconhecido ao enviar nota %1.").arg(QString::fromStdString(nnf));
    }
}

std::string NfeACBR::getPdfDanfe(){
    std::string pdfBase64 = nfe->SalvarPDFBase64();
    return pdfBase64;
}








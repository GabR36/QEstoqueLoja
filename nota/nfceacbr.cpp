#include "nfceacbr.h"
#include "../configuracao.h"
#include <QStandardPaths>
#include <qdir.h>
#include <fstream>
#include <qrandom.h>

NfceACBR::NfceACBR(QObject *parent)
    : QObject{parent}
{
    //pega o ponteiro da lib acbr do singleton
    nfce = AcbrManager::instance()->nfe();
    db = QSqlDatabase::database();
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();

    cnpjEmit = empresaValues.value("cnpj_empresa").toStdString();
    //valores padrao
    serieNf = "1";
    tpEmis = 1;

    caminhoXml = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/xmlNf";
    nfce->LimparLista();//evita acumular notas
    carregarConfig();
}

int NfceACBR::getNNF(){
    return std::stoi(nnf);
}
int NfceACBR::getSerie(){
    return serieNf.toInt();
}
QString NfceACBR::getXmlPath(){
    std::string raw = nfce->GetPath(0);

    // Remove caracteres nulos (caso GetPath tenha buffer fixo)
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    QString path = QString::fromStdString(raw).trimmed();

    raw = cnf;
    raw.erase(std::find(raw.begin(), raw.end(), '\0'), raw.end());
    QString cnfString = QString::fromStdString(raw).trimmed();
    QString ret = QString("%1/%2-nfe.xml")
                      .arg(path, cnfString);

    return ret;
}


QString NfceACBR::getVersaoLib(){
    return QString::fromStdString(nfce->Versao());
}

int NfceACBR::getProximoNNF(){
    if (!db.open()) {
        qDebug() << "Erro ao abrir banco de dados em getProximoNNF";
        return -1;
    }

    int tpAmb = fiscalValues.value("tp_amb").toInt(); // 1 = produção, 0 = homologação
    QString chaveConfig = (tpAmb == 0) ? "nnf_homolog" : "nnf_prod";
    QSqlQuery query;
    query.prepare(
        "SELECT nnf FROM notas_fiscais "
        "WHERE cstat IN (100, 150) AND modelo = :modelo AND serie = :serie AND tp_amb = :tp_amb "
        "ORDER BY nnf DESC "
        "LIMIT 1"
        );
    query.bindValue(":modelo", "65");             // NFC-e
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

double NfceACBR::getVNF(){
    return vNf;
}

void NfceACBR::setNNF(int nNF){
    qDebug() << "Setando NNF para:" << nNF;
    nnf = std::to_string(nNF);
    qDebug() << "NNF após set:" << QString::fromStdString(nnf);
}

void NfceACBR::setCliente(QString cpf, bool ehPf){
    cpfCliente = cpf;
    ehPfCliente = ehPf;
    qDebug() << cpfCliente << ehPfCliente;
}

bool NfceACBR::isValidGTIN(const QString& gtin) {
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

void NfceACBR::setProdutosVendidos(QList<QList<QVariant>> produtosVendidos, bool emitirTodos){
    if (!db.open()) {
        qDebug() << "não abriu bd setprodutosvendidos";
        return;
    }
    emitirApenasNf = !emitirTodos;

    QList<QList<QVariant>> produtosFiltrados; // nova lista filtrada

    for (int i = 0; i < produtosVendidos.size(); ++i) {
        QList<QVariant> produto = produtosVendidos[i];

        // Verificar se o produto deve ser incluído conforme o campo 'nf'
        QSqlQuery nfQuery;
        nfQuery.prepare("SELECT nf FROM produtos WHERE id = :idprod");
        nfQuery.bindValue(":idprod", produto[0]);
        if (!nfQuery.exec() || !nfQuery.next()) {
            qWarning() << "Erro ao consultar campo 'nf' do produto ID:" << produto[0].toString()
            << nfQuery.lastError().text();
            continue; // pula esse produto
        }

        int nf = nfQuery.value("nf").toInt();
        if (!emitirTodos && nf != 1) {
            continue; // ignora se não for para emitir todos e nf != 1
        }

        // [1] quantidade, [3] valor unitário
        QString quantidadeStr = QString::number(portugues.toFloat(produto[1].toString()));
        QString valorUnitarioStr = QString::number(portugues.toFloat(produto[3].toString()));

        double quantidade = quantidadeStr.toDouble();
        double valorUnitario = valorUnitarioStr.toDouble();
        double valorTotal = quantidade * valorUnitario;

        produto[1] = quantidade;
        produto[3] = valorUnitario;
        produto.append(QVariant(valorTotal)); // [4] valor total

        // Consulta dados adicionais do produto
        QSqlQuery query;
        query.prepare("SELECT codigo_barras, un_comercial, ncm, cest, csosn, pis,"
                      " aliquota_imposto FROM produtos WHERE id = :idprod");
        query.bindValue(":idprod", produto[0]);

        if (query.exec() && query.next()) {
            QString codigoBarras = query.value("codigo_barras").toString();
            QString unComercial = query.value("un_comercial").toString();
            QString ncm = query.value("ncm").toString();
            QString cest = query.value("cest").toString();
            QString csosn = query.value("csosn").toString();
            QString pis = query.value("pis").toString();
            double aliquotaImposto   = query.value("aliquota_imposto").toDouble();
            if (!isValidGTIN(codigoBarras)) {
                codigoBarras = "SEM GTIN";
            }
            produto.append(codigoBarras);      // [5]
            produto.append(unComercial);       // [6]
            produto.append(ncm);               // [7]
            produto.append(cest);              // [8]
            produto.append(aliquotaImposto);   // [9]
            produto.append(csosn); //[10]
            produto.append(pis);//[11]
        } else {
            qWarning() << "Produto ID não encontrado ou erro ao consultar:" << produto[0].toString()
                       << query.lastError().text();

            produto.append("");      // codigo_barras
            produto.append("");      // un_comercial
            produto.append("");      // ncm
            produto.append("");      // cest
            produto.append(0.0);     // aliquota_imposto
            produto.append("");//csosn
            produto.append("");//pis
        }

        produtosFiltrados.append(produto); // adiciona produto processado
    }

    quantProds = produtosFiltrados.size();
    vTotTribProduto.resize(produtosFiltrados.size());
    listaProdutos = produtosFiltrados;
}

float NfceACBR::corrigirTaxa(float taxaAntiga, float desconto){
    float taxaConvertida = (taxaAntiga / 100) + 1;
    float taxaNova = 0.0;
    float valorTotalProdutos = 0.0;
    for (int i = 0; i < listaProdutos.size(); ++i) {
        valorTotalProdutos += listaProdutos[i][4].toDouble();
    }
    taxaNova = (valorTotalProdutos - desconto) * taxaConvertida + desconto;
    return ((taxaNova/valorTotalProdutos) - 1) * 100;
}


void NfceACBR::aplicarDescontoTotal(float descontoTotal) {
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

void NfceACBR::setPagamentoValores(QString formaPag, float desconto,float recebido, float troco, float taxa){
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

void NfceACBR::aplicarAcrescimoProporcional(float taxaPercentual)
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

void NfceACBR::carregarConfig(){

        qDebug() << "=== CARREGANDO CONFIGURAÇÕES ACBR ===";

        QDir dir;
        if(!dir.exists(caminhoXml)){
            dir.mkpath(caminhoXml);
        }

        // LIMPAR strings antes de usar
        auto cleanStr = [](const QString& str) -> std::string {
            std::string result = str.toStdString();
            result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
            return result.empty() ? "" : result;
        };

        std::string certificadoPath = cleanStr(fiscalValues.value("caminho_certificado"));
        std::string certificadoSenha = cleanStr(fiscalValues.value("senha_certificado"));
        std::string uf = cleanStr(empresaValues.value("estado_empresa"));
        std::string schemaPath = cleanStr(fiscalValues.value("caminho_schema"));
        std::string idCsc = cleanStr(fiscalValues.value("id_csc"));
        std::string csc = cleanStr(fiscalValues.value("csc"));
        tpAmb = (fiscalValues.value("tp_amb") == "0" ? "1" : "0");

        qDebug() << "Certificado:" << QString::fromStdString(certificadoPath);
        qDebug() << "UF:" << QString::fromStdString(uf);
        qDebug() << "Schema:" << QString::fromStdString(schemaPath);
        qDebug() << "Ambiente:" << QString::fromStdString(tpAmb);


        //     // LIMPAR lista
        // nfce->LimparLista();



        //     // CONFIGURAÇÕES PRINCIPAIS - DFe
        nfce->ConfigGravarValor("DFe", "ArquivoPFX", certificadoPath);
        nfce->ConfigGravarValor("DFe", "Senha", certificadoSenha);
        nfce->ConfigGravarValor("DFe", "UF", uf);
        nfce->ConfigGravarValor("DFe", "SSLHttpLib", "3");
        nfce->ConfigGravarValor("DFe", "SSLCryptLib", "1");
        nfce->ConfigGravarValor("DFe", "SSLXmlSignLib", "4");

        // //     // CONFIGURAÇÕES NFe
        nfce->ConfigGravarValor("NFe", "PathSchemas", schemaPath);
        nfce->ConfigGravarValor("NFe", "IdCSC", idCsc);
        nfce->ConfigGravarValor("NFe", "CSC", csc);
        nfce->ConfigGravarValor("NFe", "ModeloDF", "1");  // NFCe
        nfce->ConfigGravarValor("NFe", "VersaoDF", "3");
        nfce->ConfigGravarValor("NFe", "VersaoQRCode", "3");
        nfce->ConfigGravarValor("NFe", "FormaEmissao", "0");
        nfce->ConfigGravarValor("NFe", "Ambiente", tpAmb);

            // // CONFIGURAÇÕES DE ARQUIVO

        nfce->ConfigGravarValor("NFe", "PathSalvar", caminhoXml.toStdString());
        nfce->ConfigGravarValor("NFe", "AdicionarLiteral", "1");
        nfce->ConfigGravarValor("NFe", "SepararPorCNPJ", "1");
        nfce->ConfigGravarValor("NFe", "SepararPorModelo", "1");
        nfce->ConfigGravarValor("NFe", "SepararPorAno", "1");
        nfce->ConfigGravarValor("NFe", "SepararPorMes", "1");
        nfce->ConfigGravarValor("NFe", "SalvarApenasNFeProcessadas", "1");
        nfce->ConfigGravarValor("NFe", "PathNFe", caminhoXml.toStdString());

        //sistema
        nfce->ConfigGravarValor("Sistema", "Nome", "QEstoqueLoja");
        nfce->ConfigGravarValor("Sistema", "Versao", "2.1.0");

        //     // CONFIGURAÇÕES DE CONEXÃO
        //     // nfce->ConfigGravarValor("NFe", "Timeout", "30000");
        //     // nfce->ConfigGravarValor("NFe", "Tentativas", "5");
        //     // nfce->ConfigGravarValor("NFe", "IntervaloTentativas", "1000");

        //     // // CONFIGURAÇÕES DANFE NFCe
        //     // nfce->ConfigGravarValor("DANFENFCe", "TipoRelatorioBobina", "0");
        //     // nfce->ConfigGravarValor("DANFENFCe", "ImprimeItens", "1");
        //     // nfce->ConfigGravarValor("DANFENFCe", "ViaConsumidor", "1");
        //     // nfce->ConfigGravarValor("DANFENFCe", "FormatarNumeroDocumento", "1");
            nfce->ConfigGravar("");
            qDebug() << "Configurações salvas no arquivo acbrlib.ini";

}


void NfceACBR::ide()
{
    cuf = fiscalValues.value("cuf").toStdString();
    std::string data = QDateTime::currentDateTime().toString("dd/MM/yyyy").toStdString();
    std::string dataHora = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm").toStdString();
    mod = 65;
    int numeroAleatorio8dig = QRandomGenerator::global()->bounded(10000000, 99999999);
    if (nnf == "") {
        qDebug() << "Erro: NNF não foi definido!";
        return;
    }else{
        qDebug() << "NNF foi definido!";

    }
    cnf = nfce->GerarChave(
        std::stoi(cuf), numeroAleatorio8dig, mod,
        serieNf.toInt(), std::stoi(nnf), tpEmis, data, cnpjEmit
        );
    std::string tpAmbIde = (tpAmb == "1") ? "2" : "1";
    qDebug() << "TPAMBIDE:" << tpAmbIde + "tpamb:" << tpAmb;
    qDebug() << "cnf:" <<  std::to_string(numeroAleatorio8dig);

    ini << "[Identificacao]\n";
    ini << "cUF=" << cuf << "\n";
    ini << "tpAmb=" << tpAmbIde << "\n";
    ini << "mod=" << mod << "\n";
    ini << "serie=" << serieNf.toStdString() << "\n";
    ini << "nNF=" << nnf << "\n";
    ini << "cNF=" << std::to_string(numeroAleatorio8dig) << "\n";
    ini << "dhEmi=" << dataHora << "\n";
    ini << "natOp=VENDA AO CONSUMIDOR\n";
    ini << "tpNF=1\n";
    ini << "finNFe=1\n";
    ini << "indFinal=1\n";
    ini << "indPres=1\n";
    ini << "tpImp=4\n";
    ini << "procEmi=0\n";
    ini << "verProc=1.0\n\n";
}

void NfceACBR::emite()
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

void NfceACBR::dest()
{


    if (cpfCliente != "") {
        ini << "[Destinatario]\n";
        ini << "CNPJCPF=" << cpfCliente.toStdString() << "\n";
    }

    if (fiscalValues.value("tp_amb") == "0" && cpfCliente != "") {
        ini << "xNome=NF-E EMITIDA EM AMBIENTE DE HOMOLOGACAO - SEM VALOR FISCAL\n";
    }

    ini << "\n";
}


void NfceACBR::carregarProds()
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

        ini << secProduto << "\n";
        ini << "CFOP=5102\n";
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

void NfceACBR::total()
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


void NfceACBR::transp()
{
    ini << "[Transportador]\n";
    ini << "modFrete=9\n\n";
}

void NfceACBR::pag()
{
    if (emitirApenasNf) {
        vPagNf = vNf;
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

void NfceACBR::infRespTec()
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

void NfceACBR::ibscbsTotais()
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

QString NfceACBR::gerarEnviar(){

    ini.str("");  // Limpar conteúdo anterior
    ini.clear();

    ini << "[infNFe]\n";
    ini << "versao=4.00\n\n";

    ide();
    emite();
    dest();
    carregarProds();
    total();
    transp();
    pag();
    infRespTec();
    ibscbsTotais();

    try {
        nfce->CarregarINI(ini.str());
        nfce->Assinar();
        // nfce->GravarXml(0, "xml_naoaut_nota_"+ nnf + ".xml", "./xml");

        nfce->Validar();

        std::string retorno = nfce->Enviar(1, false, true, false);
        QString ret = QString::fromUtf8(retorno.c_str());
        qDebug() << "nfce getpath: " << nfce->GetPath(0);
        qDebug() << "nfce chave: " << cnf;

        qDebug() << "Retorno SEFAZ:" << ret;
        return ret;
        // if (ret.contains("Autorizado", Qt::CaseInsensitive)) {
        //     // nfce->GravarXml(0, "xml_autorizado_nota_"+ nnf + ".xml", "./xml");
        //     try {
        //         nfce->Imprimir("", 1, "", true, std::nullopt, std::nullopt, std::nullopt);
        //     } catch (std::exception &e) {
        //         qDebug() << "Erro ao imprimir:" << e.what();
        //     }
        //     return "Nota " + QString::fromStdString(nnf) + " autorizada com sucesso!\n\n" + ret;
        // }
        // else if (ret.contains("Rejeitado", Qt::CaseInsensitive)) {
        //     return "Nota " + QString::fromStdString(nnf) + " rejeitada pela SEFAZ:\n\n" + ret;
        // }
        // else {
        //     return "Retorno da SEFAZ:\n\n" + ret;
        // }
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








 #include "nfcevenda.h"
#include "../configuracao.h"
#include <QSqlQuery>
#include <QRegularExpression>

NfceVenda::NfceVenda(QObject *parent)
    : QObject{parent}, m_nfe{new CppNFe}
{
    fiscalValues = Configuracao::get_All_Fiscal_Values();
    empresaValues = Configuracao::get_All_Empresa_Values();
    nNf = getProximoNNF();

    connect(m_nfe, &CppNFe::wsChange, this, &NfceVenda::onWSChange);
    //connect(m_nfe, SIGNAL(errorOccurred(QString)), this, SIGNAL(errorOccurred(QString)));
    connect(m_nfe, &CppNFe::errorOccurred, this, &NfceVenda::errorOccurred);

    if(fiscalValues.value("emit_nf") == "1"){
        configurar();
    }
}

NfceVenda::~NfceVenda()
{
    delete m_nfe;
}


void NfceVenda::configurar()
{
    //configuração
    //geral
    m_nfe->configuracoes->set_ModeloDF(ConvNF::strToModeloDF("65"));
    m_nfe->configuracoes->set_VersaoNF(VersaoNF::v400);
    m_nfe->configuracoes->set_VersaoQrCodeNF(VersaoQrCodeNF::v200);
    m_nfe->configuracoes->set_TipoEmissao(TpEmis::Normal);
    m_nfe->configuracoes->set_IdCSC(fiscalValues.value("id_csc"));
    m_nfe->configuracoes->set_CSC(fiscalValues.value("csc"));
    m_nfe->configuracoes->set_indicadorSincrono(IndSinc::Nao);

//arquivos
#if defined(Q_OS_LINUX)
    m_nfe->configuracoes->arquivos->set_caminhoSchema(fiscalValues.value("caminho_schema"));
#elif defined(Q_OS_WIN)
    m_nfe->configuracoes->arquivos->set_caminhoSchema(fiscalValues.value("caminho_schema");
#endif

    //salvar
    m_nfe->configuracoes->arquivos->set_salvar(true);
    m_nfe->configuracoes->arquivos->set_salvarLogs(true);
    m_nfe->configuracoes->arquivos->set_salvarPorModelo(true); //55 ou 65
    m_nfe->configuracoes->arquivos->set_salvarPorCNPJ(1);
    m_nfe->configuracoes->arquivos->set_salvarPorAnoMes(1); //yyyymm
    //m_nfe->configuracoes->arquivos->set_salvarPorAnoMesDia(1); //yyymmdd
    //m_nfe->configuracoes->arquivos->set_salvarPorAno(1); //yyyy
    //m_nfe->configuracoes->arquivos->set_salvarPorMes(1); //mm
    //m_nfe->configuracoes->arquivos->set_salvarPorDia(1); //dd

//obs: se não informar o lugar para salvar, será salvo na pasta do executável.
#if defined(Q_OS_LINUX)
    QDir dir;
    if(!dir.exists(caminhoXml)){
        dir.mkpath(caminhoXml);
    }
    m_nfe->configuracoes->arquivos->set_caminhoSalvar(caminhoXml);
#elif defined(Q_OS_WIN)
      QDir dir;
      if(!dir.exists(caminhoXml)){
          dir.mkpath(caminhoXml);
      }
      m_nfe->configuracoes->arquivos->set_caminhoSalvar(caminhoXml);
#endif

    m_nfe->configuracoes->certificado->set_cryptoLib(CryptoType::OpenSSL);
    //configuração certificado
#if defined(Q_OS_LINUX)
    m_nfe->configuracoes->certificado->set_caminhoCertificado(fiscalValues.value("caminho_certificado"));
    m_nfe->configuracoes->certificado->set_senhaCertificado(fiscalValues.value("senha_certificado"));
#elif defined(Q_OS_WIN)
    m_nfe->configuracoes->certificado->set_caminhoCertificado(fiscalValues.value("caminho_certificado"));
    m_nfe->configuracoes->certificado->set_senhaCertificado(fiscalValues.value("senha_certificado"));
#endif
    //certificados AC
#if defined(Q_OS_LINUX)
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv4.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv5.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv6.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv7.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv10.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv11.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv12.crt");
#elif defined(Q_OS_WIN)
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasil.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv2.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv5.crt");
    m_nfe->configuracoes->certificado->addCaminhoCertificadoAC(fiscalValues.value("caminho_certac") + "/ICP-Brasilv10.crt");
#endif

    if (!m_nfe->configuracoes->certificado->carregarCertificado())
    {
        qInfo() << "Erro ao configurar certificado";
        QString _err =  m_nfe->configuracoes->certificado->get_error();
        emit errorOccurred(_err);
    }

    //webservices
    m_nfe->configuracoes->webservices->set_httpType(HttpType::HttpQt);
    m_nfe->configuracoes->webservices->set_tpAmb(fiscalValues.value("tp_amb").trimmed().toInt() == 1 ? TpAmb::Producao : TpAmb::Homologacao);
    m_nfe->configuracoes->webservices->set_protocoloSSL(ConvNF::strToQSslProtocol("TlsV1_2"));//QSsl::TlsV1_3
    m_nfe->configuracoes->webservices->set_timeoutAssincrono(8000);//milisegundos
    m_nfe->configuracoes->webservices->set_tentativas(3);



    std::map<QString, UF> estadosMap = {
        {"AC", UF::AC}, {"AL", UF::AL}, {"AP", UF::AP}, {"AM", UF::AM},
        {"BA", UF::BA}, {"CE", UF::CE}, {"DF", UF::DF}, {"ES", UF::ES},
        {"GO", UF::GO}, {"MA", UF::MA}, {"MT", UF::MT}, {"MS", UF::MS},
        {"MG", UF::MG}, {"PA", UF::PA}, {"PB", UF::PB}, {"PR", UF::PR},
        {"PE", UF::PE}, {"PI", UF::PI}, {"RJ", UF::RJ}, {"RN", UF::RN},
        {"RS", UF::RS}, {"RO", UF::RO}, {"RR", UF::RR}, {"SC", UF::SC},
        {"SP", UF::SP}, {"SE", UF::SE}, {"TO", UF::TO}
    };


    QString estadoStr = empresaValues.value("estado_empresa");
    auto it = estadosMap.find(estadoStr);
    if (it != estadosMap.end()) {
        m_nfe->configuracoes->webservices->set_uf(it->second);
    } else {
        m_nfe->configuracoes->webservices->set_uf(UF::None);
    }
    m_nfe->configuracoes->webservices->set_compactar(true);
    m_nfe->configuracoes->webservices->set_compactarAcimaDe(300);
    m_nfe->configuracoes->webservices->set_verificarSslSocket(true); //remove o tratamento de exception
}


void NfceVenda::nfe()
{
    infNFe();
    m_nfe->notafiscal->NFe->add();
}

void NfceVenda::infNFe()
{
    m_nfe->notafiscal->NFe->obj->infNFe->set_versao(ConvNF::versaoNFToStr(m_nfe->configuracoes->get_VersaoNF()));
    ide();
    emite();
    dest();
    retirada();
    entrega();
    autXML();
    det(quantProds);
    total();
    transp();
    cobr();
    pag();
    infIntermed();
    infAdic();
    exporta();
    compra();
    cana();
    infRespTec();
}

void NfceVenda::ide()
{
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_cUF(fiscalValues.value("cuf").toInt());
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_cNF(CppUtil::random(8));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_natOp("VENDA AO CONSUMIDOR");
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_mod(m_nfe->configuracoes->get_ModeloDF());
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_serie(serieNf);
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_nNF(nNf); //NUMERO *importante*
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_dhEmi(QDateTime::currentDateTime());
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_dhSaiEnt(QDateTime::currentDateTime());
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_tpNF(ConvNF::strToTpNF("1"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_idDest(ConvNF::strToIdDest("1"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_cMunFG(fiscalValues.value("cmun").toInt());
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_tpImp(TpImp::NFCe); //TpImp::Retrato
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_tpEmis(TpEmis::Normal);
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_tpAmb(m_nfe->configuracoes->webservices->get_tpAmb());//(ConvNF::strToTpAmb("2"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_finNFe(ConvNF::strToFinNFe("1"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_indFinal(ConvNF::strToIndFinal("1"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_indPres(ConvNF::strToIndPres("1"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_indIntermed(ConvNF::strToIndIntermed("0"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_procEmi(ConvNF::strToProcEmi("0"));
    m_nfe->notafiscal->NFe->obj->infNFe->ide->set_verProc("1.0.0.0");
    //--contingência
    //m_nfe->notafiscal->NFe->obj->infNFe->ide->set_dhCont(QDateTime::currentDateTime());
    //m_nfe->notafiscal->NFe->obj->infNFe->ide->set_xJust("Justificativa contingencia");

    //Grupo BA. Documento Fiscal Referenciado
    //---NFe referenciada --***CONTAINER*** 0-500
    /*
    //NFe/NFCe
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->set_refNFe("numero da chave de acesso");
    //CTe
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->set_refCTe("Chave de acesso do CT-e");

    //Informação da NF modelo 1/1A ou NF modelo 2 referenciada
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_cUF();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_AAMM();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_CNPJ();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_mod(); //01 ou 02
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_serie();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNF->set_nNF();

    //Informações da NF de produtorrural referenciada
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_cUF();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_AAMM();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_CNPJ();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_CPF();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_IE();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_mod(); //04=NF de Produtor
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_serie();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refNFP->set_nNF();

    //Informações do Cupom Fiscal referenciado
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refECF->set_mod(); //2B, 2C OU 2D
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refECF->set_nECF();
    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->obj->refECF->set_nCOO();

    m_nfe->notafiscal->NFe->obj->infNFe->ide->NFref->add(); //adicionando Referenciada
    */

}

void NfceVenda::emite()
{
    m_nfe->notafiscal->NFe->obj->infNFe->emite->set_CNPJ(empresaValues.value("cnpj_empresa"));
    //m_nfe->notafiscal->NFe->obj->infNFe->emite->set_CPF("");
    m_nfe->notafiscal->NFe->obj->infNFe->emite->set_xNome(empresaValues.value("nome_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->set_xFant(empresaValues.value("nfant_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->set_IE(fiscalValues.value("iest"));
    //m_nfe->notafiscal->NFe->obj->infNFe->emite->set_IEST("IE do Substituto Tributário");
    //m_nfe->notafiscal->NFe->obj->infNFe->emite->set_IM("INSCRIÇÃO MUNICIPAL");
    //m_nfe->notafiscal->NFe->obj->infNFe->emite->set_CNAE("CNAE fiscal");
    m_nfe->notafiscal->NFe->obj->infNFe->emite->set_CRT(Crt::SimplesNacional);
    //****endereço
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_xLgr(empresaValues.value("endereco_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_nro(empresaValues.value("numero_empresa").toInt()); //NUMERO INTEIRO
    //m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_xCpl("complemento");
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_xBairro(empresaValues.value("bairro_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_cMun(fiscalValues.value("cmun").toInt());//CODIGO DO MUNICIPIO NUMERO INTEIRO
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_xMun(empresaValues.value("cidade_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_UF(empresaValues.value("estado_empresa"));
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_CEP(empresaValues.value("cep_empresa").toInt()); //CEP NUMERO INTEIRO
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_cPais(1058); //1058 BRASIL
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_xPais("BRASIL");
    m_nfe->notafiscal->NFe->obj->infNFe->emite->enderEmit->set_fone(empresaValues.value("telefone_empresa"));


}

void NfceVenda::dest()
{
    if(ehPfCliente == false && cpfCliente != ""){
        m_nfe->notafiscal->NFe->obj->infNFe->dest->set_CNPJ(cpfCliente); //PARA PESSOA JURIDICA
    }

    if(ehPfCliente == 1 && cpfCliente != ""){
        m_nfe->notafiscal->NFe->obj->infNFe->dest->set_CPF(cpfCliente); //PARA PESSOA FISICA

    }
    if(fiscalValues.value("tp_amb") == "0" && cpfCliente != ""){
      m_nfe->notafiscal->NFe->obj->infNFe->dest->set_xNome("NF-E EMITIDA EM AMBIENTE DE HOMOLOGACAO - SEM VALOR FISCAL");
    }
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->set_CPF(""); //PARA PESSOA FISICA
    // //m_nfe->notafiscal->NFe->obj->infNFe->dest->set_idEstrangeiro("ID ESTRANGEIRO")
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->set_xNome("Consumidor");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->set_indIEDest(IndIEDest::NaoContribuinte);
    // //m_nfe->notafiscal->NFe->obj->infNFe->dest->set_IE("IE");
    // //m_nfe->notafiscal->NFe->obj->infNFe->dest->set_ISUF("suframa");
    // //m_nfe->notafiscal->NFe->obj->infNFe->dest->set_IM("insc municipal");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->set_email("email@.com");
    // //Endereço
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xLgr("rua");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_nro(1234); //numero interio
    // //m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xCpl("complemento");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xBairro("BAIRRO");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_cMun(4118600); //CODIGO DO MUNICÍPIO
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xMun("NOME DA CIDADE");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_UF("PR");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_CEP(84630000); //NUMERO INTEIRO
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_cPais(1058); //BRASIL
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_xPais("1058");
    // m_nfe->notafiscal->NFe->obj->infNFe->dest->enderDest->set_fone("1231231"); //STRING


}

void NfceVenda::retirada()
{
}

void NfceVenda::entrega()
{
}

void NfceVenda::autXML()
{

    // m_nfe->notafiscal->NFe->obj->infNFe->autXML->obj->set_CNPJ(empresaValues.value("cnpj_empresa"));
    // m_nfe->notafiscal->NFe->obj->infNFe->autXML->obj->get_CPF();
    // m_nfe->notafiscal->NFe->obj->infNFe->autXML->add();

}

void NfceVenda::det(int quantProdutos)
{
    //aplica no valor de cada produto da lista se houver taxa
    aplicarDescontoTotal(descontoNf);
    aplicarAcrescimoProporcional(taxaPercentual);


    for (int i = 0; i <= quantProdutos - 1; ++i) {
        det_produto(i);
        det_imposto(i);
        det_impostoDevol();
        det_obsItem();
        //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->set_infAdProd();

        m_nfe->notafiscal->NFe->obj->infNFe->det->add();

    }
}

void NfceVenda::det_produto(const int &i)
{
    // Acessa a linha i da lista
    QList<QVariant> produto = listaProdutos[i];
    QString id            = produto[0].toString();
    float quantVendida  = produto[1].toFloat();
    QString desc          = produto[2].toString();
    double valorUnitario = produto[3].toDouble();
    double valorTotalProd = produto[4].toDouble();
    QString codigoBarra = produto[5].toString();
    QString uCom = produto[6].toString();
    QString ncm = produto[7].toString();
    // QString cest = produto[8].toString();
    // double aliquotaImp = produto[9].toDouble();

    float descontoProduto = descontoProd[i];
    qDebug() << "produto(" << i <<")" << desc;

    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_nItem(i + 1);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_cProd(id);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_cEAN(codigoBarra);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_xProd(desc);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_NCM(ncm);//
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_NVE();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_CEST("1704900");
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_indEscala(ConvNF::strToIndEscala("S"));
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_CNPJFab("");
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_cBenef("");
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_EXTIPI("");
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_CFOP("5102");
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_uCom(uCom);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_qCom(quantVendida);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vUnCom(valorUnitario);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vProd(valorTotalProd);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_cEANTrib(codigoBarra);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_uTrib(uCom);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_qTrib(quantVendida);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vUnTrib(valorUnitario);
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vFrete();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vSeg();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vDesc(descontoProduto);
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vOutro();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_indTot(ConvNF::strToIndTot("1"));
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vFrete();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vSeg();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vDesc();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_vOutro();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_indTot(ConvNF::strToIndTot("1"));
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_xPed();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_nItemPed();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_nFCI();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->set_nRECOPI();

    /*
    //Grupo I01. Produtos e Serviços / Declaração de Importação
    //***CONTAINER*** 0-100
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_nDI();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_dDI();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_xLocDesemb();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_UFDesemb();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_dDesemb();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_tpViaTransp();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_vAFRMM();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_tpIntermedio();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_CNPJ();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_UFTerceiro();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->set_cExportador();
        //---Adições(Dentro de DI)--***CONTAINER*** 1-100
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->obj->set_nAdicao();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->obj->set_nSeqAdic();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->obj->set_cFabricante();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->obj->set_vDescDI();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->obj->set_nDraw();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->obj->adi->add();//adicionando adi
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->DI->add();//adicionando DI
    */

    /*
    //Grupo I03. Produtos e Serviços / Grupo de Exportação
    //-***CONTAINER*** 0-500
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->detExport->obj->set_nDraw();
        //Grupo sobre exportação indireta
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->detExport->obj->exportInd->set_nRE();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->detExport->obj->exportInd->set_chNFe();
        m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->detExport->obj->exportInd->set_qExport();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->detExport->add();//adicionando detExport
    */

    /*
    //Grupo I80. Rastreabilidade de produto
    //--***CONTAINER*** 0-500
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->obj->set_nLote();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->obj->set_qLote();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->obj->set_dFab();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->obj->set_dVal();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->obj->set_cAgreg();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->rastro->add();//adicionando rastro
    */

    /*
    //Grupo JA. Detalhamento Específico de Veículos novos
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_tpOp();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_chassi();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_cCor();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_pot();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_cilin();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_pesoL();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_pesoB();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_nSerie();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_tpComb();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_nMotor();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_CMT();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_dist();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_anoMocsc
);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_anoFab();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_tpPint();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_tpVeic();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_espVeic();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_VIN();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_condVeic();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_cMod();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_cCorDENATRAN();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_lota();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->veicProd->set_tpRest();
    */

    /*
    //Grupo K. Detalhamento Específico de Medicamento e de matérias-primas farmacêuticas
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->med->set_cProdANVISA();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->med->set_xMotivoIsencao();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->med->set_vPMC();
    */

    /*
    //Grupo L. Detalhamento Específico de Armamentos
    //--***CONTAINER*** 1-500
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->arma->obj->set_tpArma();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->arma->obj->set_nSerie();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->arma->obj->set_nCano();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->arma->obj->set_descr();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->arma->add();
    */

    /*
    //Grupo LA. Detalhamento Específico de Combustíveis
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_cProdANP();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_descANP();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_pGLP();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_pGNn();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_pGNi();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_vPart();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_CODIF();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_qTemp();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->set_UFCons();
    //LA07 - Informações da CIDE (obs:Dentro de comb)
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->CIDE->set_qBCProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->CIDE->set_vAliqProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->CIDE->set_vCIDE();
    //LA11 - Informações do grupo de “encerrante” (obs:Dentro de comb)
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->encerrante->set_nBico();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->encerrante->set_nBomba();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->encerrante->set_nTanque();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->encerrante->set_vEncIni();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->prod->comb->encerrante->set_vEncFin();
    */


}

void NfceVenda::det_imposto(const int &i)
{
    QList<QVariant> produto = listaProdutos[i];
    //QString id            = produto[0].toString();
    //float quantVendida  = produto[1].toFloat();
    //QString desc          = produto[2].toString();
    //double valorUnitario = produto[3].toDouble();
    double valorTotalProd = produto[4].toDouble();
    double aliquotaImp = produto[9].toDouble();
    double porcentagemImposto = aliquotaImp/100;
    float vTotTrib = QString::number(valorTotalProd * porcentagemImposto,'f',2).toFloat();

    //Grupo M. Tributos incidentes no Produto ou Serviço
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->set_vTotTrib(vTotTrib);

    // adiciona o imposto referente ao produto para contabilizar depois
    vTotTribProduto[i] = vTotTrib;
    //Grupo N01. ICMS Normal e ST
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_orig(ConvNF::strToOrig("0"));
    // m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_CST(ConvNF::strToCstICMS("00"));
    // m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_modBC(ConvNF::strToModBC("0"));
    // m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_vBC(1.00);
    // m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_pICMS(17.50);
    // m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_vICMS(0.17);
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_pFCP();
    //m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_vFCP();

    //simples nacional
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMS->set_CSOSN(ConvNF::strToCsosnICMS("102"));

    /*
    //Grupo NA. ICMS para a UF de destino
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_vBCUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_vBCFCPUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_pFCPUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_pICMSUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_pICMSInter();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_pICMSInterPart();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_vFCPUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_vICMSUFDest();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ICMSUFDest->set_vICMSUFRemet();
    */

    /*
    //Grupo O. Imposto sobre Produtos Industrializados
    //IPI
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_CNPJProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_cSelo();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_qSelo();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_cEnq();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_CST();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_pIPI();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_qUnid();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_vUnid();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->IPI->set_vIPI();
    */

    /*
    //Grupo P. Imposto de Importação
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->II->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->II->set_vDespAdu();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->II->set_vII();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->II->set_vIOF();
    */


    //Grupo Q. PIS
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PIS->set_CST(ConvNF::strToCstPIS("49"));
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PIS->set_vBC(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PIS->set_pPIS(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PIS->set_vPIS(0.00);


    /*
    //Grupo R. PIS ST
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_pPIS();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_qBCProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_vAliqProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_vAliqProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->PISST->set_vPIS();
    */

    //Grupo S. COFINS
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINS->set_CST(ConvNF::strToCstCOFINS("49"));
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINS->set_vBC(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINS->set_pCOFINS(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINS->set_vCOFINS(0.00);

    /*
    //Grupo T. COFINS ST
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINSST->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINSST->set_pCOFINS();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINSST->set_qBCProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINSST->set_vAliqProd();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->COFINSST->set_vCOFINS();
    */

    /*
    //Grupo U. ISSQN
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vAliq();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vISSQN();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_cMunFG();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_cListServ();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vDeducao();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vOutro();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vDescIncond();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vDescCond();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_vISSRet();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_indISS();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_cServico();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_cMun();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_cPais();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_nProcesso();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->imposto->ISSQN->set_indIncentivo();
    */

}

void NfceVenda::det_impostoDevol()
{
    /*
    //Grupo UA. Tributos Devolvidos (para o item da NF-e)
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->impostoDevol->set_pDevol();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->impostoDevol->set_vIPIDevol();
    */

}

void NfceVenda::det_obsItem()
{
    //Nota Técnica 2021.004

    /*
    //Grupo de observações de uso livre do Contribuinte
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->obsItem->obsCont->set_xCampo();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->obsItem->obsCont->set_xTexto();
    //Grupo de observações de uso livre do Fisco
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->obsItem->obsFisco->set_xCampo();
    m_nfe->notafiscal->NFe->obj->infNFe->det->obj->obsItem->obsFisco->set_xTexto();
    */

}

void NfceVenda::total()
{
    double totalGeral,vSeg,vFrete,totalTributo = 0.0;

    for (const QList<QVariant>& produto : listaProdutos) {
        if (produto.size() >= 5) { // índice 4 é o valor total
            totalGeral += produto[4].toDouble();
        }
    }

    vNf = totalGeral + vSeg + vFrete - descontoNf;


    //Grupo W. Total da NF-e
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vBC(0.00); //2.00
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vICMS(0.00); //0.34
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vICMSDeson(0.00);
    //m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vFCPUFDest();
    //m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vICMSUFDest();
    //m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vICMSUFRemet();
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vFCP(0.00);
    //m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vBCST(0.00);
    //m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vST(0.00);
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vFCPST(0.00);
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vFCPSTRet(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vProd(totalGeral);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vFrete(vFrete);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vSeg(vSeg);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vDesc(descontoNf);
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vII(0.00);
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vIPI(0.00);
    // m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vIPIDevol(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vPIS(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vCOFINS(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vOutro(0.00);
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vNF(vNf);

    for(int i = 0; i < listaProdutos.size(); i++){
        totalTributo += vTotTribProduto[i];
    }
    qDebug() << "total trib: " << totalTributo;
    m_nfe->notafiscal->NFe->obj->infNFe->total->ICMSTot->set_vTotTrib(totalTributo);//duas casas decimais

    /*
    //Grupo W01. Total da NF-e / ISSQN
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vServ();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vBC();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vISS();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vPIS();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vCOFINS();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_dCompet();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vDeducao();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vOutro();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vDescIncond();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vDescCond();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_vISSRet();
    m_nfe->notafiscal->NFe->obj->infNFe->total->ISSQNtot->set_cRegTrib();
    */

    /*
    //Grupo W02. Total da NF-e / Retenção de Tributos
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vRetPIS();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vRetCOFINS();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vRetCSLL();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vBCIRRF();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vIRRF();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vBCRetPrev();
    m_nfe->notafiscal->NFe->obj->infNFe->total->retTrib->set_vRetPrev();
    */

}

void NfceVenda::transp()
{
    //Grupo X. Informações do Transporte da NF-e
    m_nfe->notafiscal->NFe->obj->infNFe->transp->set_modFrete(ConvNF::strToModFrete("9"));
    transp_transporta();
    transp_retTransp();
    transp_veicTransp();
    transp_reboque();
    //m_nfe->notafiscal->NFe->obj->infNFe->transp->set_vagao();
    //m_nfe->notafiscal->NFe->obj->infNFe->transp->set_balsa();
    transp_vol();

}

void NfceVenda::transp_transporta()
{
    /*
    //Grupo Transportador
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_CNPJ();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_CPF();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_xNome();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_IE();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_xEnder();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_xMun();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->transporta->set_UF();
    */

}

void NfceVenda::transp_retTransp()
{
    /*
    //Grupo Retenção ICMS transporte
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_vServ();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_vBCRet();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_pICMSRet();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_vICMSRet();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_CFOP();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->retTransp->set_cMunFG();
    */
}

void NfceVenda::transp_veicTransp()
{
    /*
    //Grupo Veículo Transporte
    m_nfe->notafiscal->NFe->obj->infNFe->transp->veicTransp->set_placa();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->veicTransp->set_UF();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->veicTransp->set_RNTC();
    */

}

void NfceVenda::transp_reboque()
{
    /*
    //Grupo Reboque
    m_nfe->notafiscal->NFe->obj->infNFe->transp->reboque->obj->set_placa();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->reboque->obj->set_UF();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->reboque->obj->set_RNTC();
    */
}

void NfceVenda::transp_vol()
{
    /*
    //Grupo Volumes
    //--***CONTAINER*** 0-5000
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_qVol();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_esp();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_marca();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_nVol();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_pesoL();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->set_pesoB();
        //sub grupo de volume - Grupo Lacres
        //--***CONTAINER*** 0-5000
        m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->lacres->obj->set_nLacre();
        m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->obj->lacres->add();
    m_nfe->notafiscal->NFe->obj->infNFe->transp->vol->add();
    */

}

void NfceVenda::cobr()
{
    /*
    //Grupo Y. Dados da Cobrança

    //Grupo Fatura
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->fat->set_nFat();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->fat->set_vOrig();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->fat->set_vDesc();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->fat->set_vLiq();

    //Grupo Parcelas
    //--***CONTAINER*** 0-120
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->dup->obj->set_nDup();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->dup->obj->set_dVenc();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->dup->obj->set_vDup();
    m_nfe->notafiscal->NFe->obj->infNFe->cobr->dup->add();
    */

}

void NfceVenda::pag()
{
    // ajusta os valores de valor recebido e troco para poder emitir
    if(emitirApenasNf){
        vPagNf = vNf;
        trocoNf = 0;
    }
    //Grupo YA. Informações de Pagamento


    //Grupo Detalhamento do Pagamento
    //--***CONTAINER*** 1-100
    m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->set_indPag(ConvNF::strToIndPag(indPagNf));
    m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->set_tPag(ConvNF::strToTPag(tPagNf));
    m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->set_vPag(vPagNf);
    if(tPagNf == "03" || tPagNf == "04" || tPagNf == "17"){

    //Grupo de Cartões //setado valores genericos sem procedencia
        m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->card->set_tpIntegra(ConvNF::strToTpIntegra("2"));
        //m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->card->set_CNPJ();
        m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->card->set_tBand(ConvNF::strToTBand("99"));
        m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->obj->card->set_cAut("000000");

    }

    m_nfe->notafiscal->NFe->obj->infNFe->pag->detPag->add();

    m_nfe->notafiscal->NFe->obj->infNFe->pag->set_vTroco(trocoNf);

}

void NfceVenda::infIntermed()
{
    /*
    //Grupo YB. Informações do Intermediador da Transação
    m_nfe->notafiscal->NFe->obj->infNFe->infIntermed->set_CNPJ();
    m_nfe->notafiscal->NFe->obj->infNFe->infIntermed->set_idCadIntTran();
    */
}

void NfceVenda::infAdic()
{
    //Grupo Z. Informações Adicionais da NF-e
    //m_nfe->notafiscal->NFe->obj->infNFe->infAdic->set_infAdFisco();
    //m_nfe->notafiscal->NFe->obj->infNFe->infAdic->set_infCpl();

    /*
    //obsCont - Grupo Campo de uso livre do contribuinte
    //--***CONTAINER*** 0-10
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsCont->obj->set_xCampo();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsCont->obj->set_xTexto();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsCont->add();
    */

    /*
    //obsFisco - Grupo Campo de uso livre do Fisco(dentro de infAdic)
    //--***CONTAINER*** 0-10
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsFisco->obj->set_xCampo();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsFisco->obj->set_xTexto();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->obsFisco->add(); //adicionando obsFisco
    */

    /*
    //procRef - Grupo Processo referenciado
    //--***CONTAINER*** 0-100
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->procRef->obj->set_nProc();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->procRef->obj->set_indProc();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->procRef->obj->set_tpAto();
    m_nfe->notafiscal->NFe->obj->infNFe->infAdic->procRef->add(); //adicionando procRef
    */
}

void NfceVenda::exporta()
{
    /*
     //Grupo ZA. Informações de Comércio Exterior
     m_nfe->notafiscal->NFe->obj->infNFe->exporta->set_UFSaidaPais();
     m_nfe->notafiscal->NFe->obj->infNFe->exporta->set_xLocExporta();
     m_nfe->notafiscal->NFe->obj->infNFe->exporta->set_xLocDespacho();
     */
}

void NfceVenda::compra()
{
    /*
    //Grupo ZB. Informações de Compras
    m_nfe->notafiscal->NFe->obj->infNFe->compra->set_xNEmp();
    m_nfe->notafiscal->NFe->obj->infNFe->compra->set_xPed();
    m_nfe->notafiscal->NFe->obj->infNFe->compra->set_xCont();
    */

}

void NfceVenda::cana()
{
    /*
    //Grupo ZC. Informações do Registro de Aquisição de Cana
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_safra();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_ref();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_qTotMes();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_qTotAnt();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_qTotGer();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_vFor();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_vTotDed();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->set_vLiqFor();

    //forDia - Grupo Fornecimento diário de cana
    //--***CONTAINER*** 1-31
    m_nfe->notafiscal->NFe->obj->infNFe->cana->forDia->obj->set_dia();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->forDia->obj->set_qtde();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->forDia->add();

    //deduc - Grupo Deduções – Taxas e Contribuições
    //--***CONTAINER*** 0-10
    m_nfe->notafiscal->NFe->obj->infNFe->cana->deduc->obj->set_xDed();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->deduc->obj->set_vDed();
    m_nfe->notafiscal->NFe->obj->infNFe->cana->deduc->add();//adicionando deduc
    */

}

void NfceVenda::infRespTec()
{

    // //Grupo ZD. Informações do Responsável Técnico
    m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_CNPJ(fiscalValues.value("cnpj_rt"));
    m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_xContato(fiscalValues.value("nome_rt"));
    m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_email(fiscalValues.value("email_rt"));
    m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_fone(fiscalValues.value("fone_rt"));
    // m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_idCSRT("");
    // m_nfe->notafiscal->NFe->obj->infNFe->infRespTec->set_hashCSRT("");

}


void NfceVenda::onReqStatusServico()
{
    QString _status;

    m_nfe->eventos->statusServico->clear();

    if (m_nfe->eventos->statusServico->status())
    {
        _status = "Versao: " + m_nfe->eventos->statusServico->retorno->get_versao() + "\n";
        _status += "cStat: " + QString::number(m_nfe->eventos->statusServico->retorno->get_cStat()) + "\n";
        _status += "cUF: " + QString::number(m_nfe->eventos->statusServico->retorno->get_cUF()) + "\n";
        _status += "dhRecbto: " + CppUtil::dateTimeToStr(m_nfe->eventos->statusServico->retorno->get_dhRecbto(),
                                                         DtH::DateTimeBr) + "\n";
        _status += "tpAmb: " + ConvNF::tpAmbToStr(m_nfe->eventos->statusServico->retorno->get_tpAmb()) + "\n";
        _status += "tMed: " + QString::number(m_nfe->eventos->statusServico->retorno->get_tMed()) + "\n";
        _status += "verAplic: " + m_nfe->eventos->statusServico->retorno->get_verAplic() + "\n";
        _status += "xMotivo: " + m_nfe->eventos->statusServico->retorno->get_xMotivo() + "\n";
        _status += "xObs: " + m_nfe->eventos->statusServico->retorno->get_xObs() + "\n";
        _status += "dhRetorno: " + CppUtil::dateTimeToStr(m_nfe->eventos->statusServico->retorno->get_dhRetorno(),
                                                          DtH::DateTimeBr) + "\n\n";

        emit retStatusServico(_status);

    }
}

void NfceVenda::onReqGerarEnviar()
{
    nfe();
    m_nfe->notafiscal->gerar();
    m_nfe->notafiscal->assinar();
    m_nfe->notafiscal->validar();

    QString _retLote, _xml;
    int _lote = 4;
    if (m_nfe->notafiscal->enviar(_lote))
    {
        _retLote = "Lote \n";
        _retLote += "Versao: " + m_nfe->notafiscal->retorno->get_versao() +"\n";
        _retLote += "tpAmb: " + ConvNF::tpAmbToStr(m_nfe->notafiscal->retorno->get_tpAmb()) +"\n";
        _retLote += "verAplic: " + m_nfe->notafiscal->retorno->get_verAplic() +"\n";
        _retLote += "nRec: " + m_nfe->notafiscal->retorno->get_nRec() +"\n";
        _retLote += "cStat: " + QString::number(m_nfe->notafiscal->retorno->get_cStat()) +"\n";
        _retLote += "xMotivo: " + m_nfe->notafiscal->retorno->get_xMotivo() +"\n";
        _retLote += "cUF: " + QString::number(m_nfe->notafiscal->retorno->get_cUF()) +"\n";
        _retLote += "dhRecbto: " + CppUtil::dateTimeToStr(m_nfe->notafiscal->retorno->get_dhRecbto(), DtH::DateTimeBr) +"\n";
        _retLote += "cMsg: " + QString::number(m_nfe->notafiscal->retorno->get_cMsg()) +"\n";
        _retLote += "xMsg: " + m_nfe->notafiscal->retorno->get_xMsg() + "\n\n";

        if(m_nfe->notafiscal->retorno->protNFe->items->count() > 0)
        {
            _retLote += "Informacoes das Notas \n\n";

            for (int i = 0; i < m_nfe->notafiscal->retorno->protNFe->items->count(); ++i) {
                _retLote += "Versao: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_versao() +"\n";
                _retLote += "ID: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_Id() +"\n";
                _retLote += "tpAmb: " + ConvNF::tpAmbToStr(m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_tpAmb()) +"\n";
                _retLote += "verAplic: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_verAplic() +"\n";
                _retLote += "chNFe: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_chNFe() +"\n";
                _retLote += "dhRecbto: " +  m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_dhRecbto().toString("dd/MM/yyyy hh:mm:ss") +"\n";
                _retLote += "nProt: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_nProt() +"\n";
                _retLote += "digVal: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_digVal() +"\n";
                _retLote += "cStat: " + QString::number(m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_cStat()) +"\n";
                _retLote += "xMotivo: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_xMotivo() +"\n";
                _retLote += "cMsg: " + QString::number(m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_cMsg()) +"\n";
                _retLote += "xMsg: " + m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_xMsg() +"\n";
                _retLote += QStringLiteral("-------------------------------") + "\n\n";

                _xml +=  m_nfe->notafiscal->retorno->protNFe->items->value(i)->get_xml();

            }

            emit retXML(_xml);

        }
        emit retLote(_retLote);
    }
}

void NfceVenda::onWSChange(const WebServicesNF &webServicesNF)
{
    QString _ws;
    switch (webServicesNF)
    {
    case WebServicesNF::NFeAutorizacao : _ws = QStringLiteral("Enviando NFe/NFCe ...") + "\n";
        break;
    case WebServicesNF::NFeConsultaCadastro : _ws = QStringLiteral("Consultando Cadastro ...") + "\n";
        break;
    case WebServicesNF::NFeConsultaProtocolo : _ws = QStringLiteral("Consultando Protocolo ...") + "\n";
        break;
    case WebServicesNF::NFeInutilizacao : _ws = QStringLiteral("Enviando Inutilização ...") + "\n";
        break;
    case WebServicesNF::NFeRecepcaoEvento : _ws = QStringLiteral("Recebendo Eventos ...") + "\n";
        break;
    case WebServicesNF::NFeRetAutorizacao : _ws = QStringLiteral("Recebendo Retorno da NFe/NFCe ...") + "\n";
        break;
    case WebServicesNF::NFeStatusServico : _ws = QStringLiteral("Recebendo Status do Serviço ...") + "\n";
        break;
    case WebServicesNF::None : _ws = QStringLiteral("Finalizando Conexão...") + "\n";
        break;
    default: _ws = QStringLiteral("...") + "\n";
        break;
    }


    emit  retWSChange(_ws);


}

const CppNFe *NfceVenda::getCppNFe()
{
    return this->m_nfe;
}

void NfceVenda::setProdutosVendidos(QList<QList<QVariant>> produtosVendidos, bool emitirTodos){
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
        query.prepare("SELECT codigo_barras, un_comercial, ncm, cest, aliquota_imposto FROM produtos WHERE id = :idprod");
        query.bindValue(":idprod", produto[0]);

        if (query.exec() && query.next()) {
            QString codigoBarras     = query.value("codigo_barras").toString();
            QString unComercial      = query.value("un_comercial").toString();
            QString ncm              = query.value("ncm").toString();
            QString cest             = query.value("cest").toString();
            double aliquotaImposto   = query.value("aliquota_imposto").toDouble();
            if (!isValidGTIN(codigoBarras)) {
                codigoBarras = "SEM GTIN";
            }
            produto.append(codigoBarras);      // [5]
            produto.append(unComercial);       // [6]
            produto.append(ncm);               // [7]
            produto.append(cest);              // [8]
            produto.append(aliquotaImposto);   // [9]
        } else {
            qWarning() << "Produto ID não encontrado ou erro ao consultar:" << produto[0].toString()
                       << query.lastError().text();

            produto.append("");      // codigo_barras
            produto.append("");      // un_comercial
            produto.append("");      // ncm
            produto.append("");      // cest
            produto.append(0.0);     // aliquota_imposto
        }

        produtosFiltrados.append(produto); // adiciona produto processado
    }

    quantProds = produtosFiltrados.size();
    vTotTribProduto.resize(produtosFiltrados.size());
    listaProdutos = produtosFiltrados;
}
void NfceVenda::setPagamentoValores(QString formaPag, float desconto,float recebido, float troco, float taxa){
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
int NfceVenda::getNNF(){
    return nNf;
}
int NfceVenda::getSerie(){
    return serieNf;
}
QString NfceVenda::getXmlPath(){
    QString xml = m_nfe->configuracoes->arquivos->get_caminhoNF() +
                  m_nfe->notafiscal->NFe->items->value(0)->get_chNFe() + "-nfe.xml";
    return xml;
}
int NfceVenda::getProximoNNF(){
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
void NfceVenda::aplicarAcrescimoProporcional(float taxaPercentual)
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
void NfceVenda::aplicarDescontoTotal(float descontoTotal) {
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
float NfceVenda::corrigirDescontoParaAplicacaoPosTaxa(float descontoDesejado, float taxaPercentual) {
    float fatorTaxa = 1.0 + (taxaPercentual / 100.0);
    return descontoDesejado * fatorTaxa;
}
float NfceVenda::corrigirTaxa(float taxaAntiga, float desconto){
    float taxaConvertida = (taxaAntiga / 100) + 1;
    float taxaNova = 0.0;
    float valorTotalProdutos = 0.0;
    for (int i = 0; i < listaProdutos.size(); ++i) {
        valorTotalProdutos += listaProdutos[i][4].toDouble();
    }
    taxaNova = (valorTotalProdutos - desconto) * taxaConvertida + desconto;
    return ((taxaNova/valorTotalProdutos) - 1) * 100;
}
void NfceVenda::setCliente(QString cpf, bool ehPf){
    cpfCliente = cpf;
    ehPfCliente = ehPf;

}
bool NfceVenda::isValidGTIN(const QString& gtin) {
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

float NfceVenda::getVNF(){
    return vNf;
}








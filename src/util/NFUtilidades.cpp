#include "NfUtilidades.h"

const QString unidadesComerciais[] = {
    "UN", "PC", "CX", "KG", "G", "MG", "L", "ML", "MT", "CM", "MM", "M2", "M3",
    "PCT", "SC", "DZ", "PT", "RL", "CJ", "FD", "AM", "FR", "TB", "PA", "VD",
    "BL", "FL", "JG", "BG", "KT", "RP", "TR", "GR"
};

const int unidadesComerciaisCount = sizeof(unidadesComerciais) / sizeof(QString);


const QString EVENTO_CARTA_CORRECAO = "110110";
const QString EVENTO_CANCELAMENTO = "110111";

const QString EVENTO_CONFIRMACAO_OPERACAO = "210200";
const QString EVENTO_CIENCIA_OPERACAO = "210210";
const QString EVENTO_DESCONHECIMENTO = "210220";
const QString EVENTO_OPERACAO_NAO_REALIZADA = "210240";

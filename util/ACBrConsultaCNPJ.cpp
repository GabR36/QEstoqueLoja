#include "ACBrConsultaCNPJ.h"
#include <stdexcept>
#include <iostream>
#include <QDebug>

typedef int (*CNPJ_Inicializar)(void**, const char*, const char*);
typedef int (*CNPJ_Finalizar)(void*);
typedef int (*CNPJ_Nome)(void*, char*, long*);
typedef int (*CNPJ_Versao)(void*, char*, long*);
typedef int (*CNPJ_Consultar)(void*, const char*, char*, long*);

ACBrConsultaCNPJ::ACBrConsultaCNPJ(std::string eArqConfig, std::string eChaveCrypt) {
#if defined(ISWINDOWS)
#if defined(ENVIRONMENT32)
    nHandler = LoadLibraryW(L"ACBrConsultaCNPJ32.dll");
#else
    nHandler = LoadLibraryW(L"ACBrConsultaCNPJ64.dll");
#endif
#else
#if defined(ENVIRONMENT32)
    std::string path = "/usr/lib/libacbrlibconsultacnpj32.so";
#else
    std::string path = "/usr/lib/libacbrconsultacnpj64.so";
#endif
    nHandler = dlopen(path.c_str(), RTLD_LAZY);
#endif

    if (!nHandler)
        throw std::runtime_error("Falha ao carregar biblioteca ACBrLibConsultaCNPJ");

    CNPJ_Inicializar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<CNPJ_Inicializar>(GetProcAddress(nHandler, "CNPJ_Inicializar"));
#else
    method = reinterpret_cast<CNPJ_Inicializar>(dlsym(nHandler, "CNPJ_Inicializar"));
#endif

    if (!method)
        throw std::runtime_error("Não encontrou função CNPJ_Inicializar na biblioteca");

    int ret = method(&this->libHandler, eArqConfig.c_str(), eChaveCrypt.c_str());
    CheckResult(ret);
}

ACBrConsultaCNPJ::~ACBrConsultaCNPJ() {
    qDebug() << "~ACBrConsultaCNPJ() chamado";
    if (!nHandler) return;

    CNPJ_Finalizar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<CNPJ_Finalizar>(GetProcAddress(nHandler, "CNPJ_Finalizar"));
#else
    method = reinterpret_cast<CNPJ_Finalizar>(dlsym(nHandler, "CNPJ_Finalizar"));
#endif
    if (method)
        method(this->libHandler);

#if defined(ISWINDOWS)
    FreeLibrary(nHandler);
#else
    dlclose(nHandler);
#endif
        qDebug() << "Finalização concluída";
}

std::string ACBrConsultaCNPJ::Nome() const {
    CNPJ_Nome method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<CNPJ_Nome>(GetProcAddress(nHandler, "CNPJ_Nome"));
#else
    method = reinterpret_cast<CNPJ_Nome>(dlsym(nHandler, "CNPJ_Nome"));
#endif

    std::string buffer(BUFFER_LEN, '\0');
    long len = BUFFER_LEN;

    int ret = method(this->libHandler, buffer.data(), &len);
    CheckResult(ret);
    return ProcessResult(buffer, len);
}

std::string ACBrConsultaCNPJ::Versao() const {
    CNPJ_Versao method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<CNPJ_Versao>(GetProcAddress(nHandler, "CNPJ_Versao"));
#else
    method = reinterpret_cast<CNPJ_Versao>(dlsym(nHandler, "CNPJ_Versao"));
#endif

    std::string buffer(BUFFER_LEN, '\0');
    long len = BUFFER_LEN;

    int ret = method(this->libHandler, buffer.data(), &len);
    CheckResult(ret);
    return ProcessResult(buffer, len);
}

std::string ACBrConsultaCNPJ::Consultar(std::string eCNPJ) const {
    CNPJ_Consultar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<CNPJ_Consultar>(GetProcAddress(nHandler, "CNPJ_Consultar"));
#else
    method = reinterpret_cast<CNPJ_Consultar>(dlsym(nHandler, "CNPJ_Consultar"));
#endif

    std::string buffer(BUFFER_LEN, '\0');
    long len = BUFFER_LEN;

    int ret = method(this->libHandler, eCNPJ.c_str(), buffer.data(), &len);
    CheckResult(ret);
    return ProcessResult(buffer, len);
}

void ACBrConsultaCNPJ::CheckResult(int ret) const {
    if (ret != 0)
        throw std::runtime_error("Erro na execução da função da ACBrLibConsultaCNPJ. Código: " + std::to_string(ret));
}

std::string ACBrConsultaCNPJ::ProcessResult(const std::string& buffer, int len) const {
    if (len <= 0) return "";
    return std::string(buffer.c_str(), len);
}

#include "ACBrMail.h"
#include <stdexcept>
#include <iostream>

typedef int (*MAIL_Inicializar)(void**, const char*, const char*);
typedef int (*MAIL_Finalizar)(void*);
typedef int (*MAIL_Nome)(void*, char*, long*);
typedef int (*MAIL_Versao)(void*, char*, long*);
typedef int (*MAIL_ConfigGravarValor)(void*, const char*, const char*, const char*);
typedef int (*MAIL_Clear)(void*);
typedef int (*MAIL_SetSubject)(void*, const char*);
typedef int (*MAIL_AddAddress)(void*, const char*);
typedef int (*MAIL_AddBody)(void*, const char*);
typedef int (*MAIL_AddAltBody)(void*, const char*);
typedef int (*MAIL_Send)(void*, int);
typedef int (*MAIL_ConfigGravar)(void*, const char*);
typedef int (*MAIL_ClearAttachment)(void*);
typedef int (*MAIL_AddAttachment)(void*, const char*, const char*, int);

#define CHECK_HANDLE() \
if (!libHandler) \
        throw std::runtime_error("ACBrMail não inicializado");

ACBrMail::ACBrMail(std::string eArqConfig,
                   std::string eChaveCrypt,
                   std::string ePathLog)
{
#if defined(ISWINDOWS)
    nHandler = LoadLibraryW(L"ACBrMail64.dll");
#else
    nHandler = dlopen("/usr/lib/libacbrmail64.so", RTLD_LAZY);
#endif

    if (!nHandler)
        throw std::runtime_error("Falha ao carregar ACBrMail");

    MAIL_Inicializar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Inicializar>(
        GetProcAddress(nHandler, "MAIL_Inicializar"));
#else
    method = reinterpret_cast<MAIL_Inicializar>(
        dlsym(nHandler, "MAIL_Inicializar"));
#endif

    if (!method)
        throw std::runtime_error("Não encontrou MAIL_Inicializar");

    int ret = method(&libHandler, eArqConfig.c_str(), eChaveCrypt.c_str());
    CheckResult(ret);

    ConfigGravarValor("Principal", "TipoResposta", "2");
    ConfigGravarValor("Principal", "LogNivel", "4");
    ConfigGravarValor("Principal", "LogPath", ePathLog);
}

ACBrMail::~ACBrMail() {
    if (!nHandler) return;

    MAIL_Finalizar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Finalizar>(
        GetProcAddress(nHandler, "MAIL_Finalizar"));
#else
    method = reinterpret_cast<MAIL_Finalizar>(
        dlsym(nHandler, "MAIL_Finalizar"));
#endif

    if (method)
        method(libHandler);

#if defined(ISWINDOWS)
    FreeLibrary(nHandler);
#else
    dlclose(nHandler);
#endif
}

std::string ACBrMail::Nome() const {
    MAIL_Nome method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Nome>(
        GetProcAddress(nHandler, "MAIL_Nome"));
#else
    method = reinterpret_cast<MAIL_Nome>(
        dlsym(nHandler, "MAIL_Nome"));
#endif

    std::string buffer(BUFFER_LEN, '\0');
    long len = BUFFER_LEN;

    int ret = method(libHandler, buffer.data(), &len);
    CheckResult(ret);
    return std::string(buffer.c_str(), len);
}

std::string ACBrMail::Versao() const {
    MAIL_Versao method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Versao>(
        GetProcAddress(nHandler, "MAIL_Versao"));
#else
    method = reinterpret_cast<MAIL_Versao>(
        dlsym(nHandler, "MAIL_Versao"));
#endif

    std::string buffer(BUFFER_LEN, '\0');
    long len = BUFFER_LEN;

    int ret = method(libHandler, buffer.data(), &len);
    CheckResult(ret);
    return std::string(buffer.c_str(), len);
}

void ACBrMail::ConfigGravarValor(const std::string& eSessao,
                                 const std::string& eChave,
                                 const std::string& eValor) const
{
    MAIL_ConfigGravarValor method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_ConfigGravarValor>(
        GetProcAddress(nHandler, "MAIL_ConfigGravarValor"));
#else
    method = reinterpret_cast<MAIL_ConfigGravarValor>(
        dlsym(nHandler, "MAIL_ConfigGravarValor"));
#endif

    int ret = method(libHandler,
                     eSessao.c_str(),
                     eChave.c_str(),
                     eValor.c_str());
    CheckResult(ret);
}

void ACBrMail::Limpar() const {
    MAIL_Clear method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Clear>(
        GetProcAddress(nHandler, "MAIL_Clear"));
#else
    method = reinterpret_cast<MAIL_Clear>(
        dlsym(nHandler, "MAIL_Clear"));
#endif
    CheckResult(method(libHandler));
}

void ACBrMail::SetAssunto(const std::string& assunto) const {
    MAIL_SetSubject method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_SetSubject>(
        GetProcAddress(nHandler, "MAIL_SetSubject"));
#else
    method = reinterpret_cast<MAIL_SetSubject>(
        dlsym(nHandler, "MAIL_SetSubject"));
#endif
    CheckResult(method(libHandler, assunto.c_str()));
}

void ACBrMail::AddDestinatario(const std::string& email) const {
    MAIL_AddAddress method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_AddAddress>(
        GetProcAddress(nHandler, "MAIL_AddAddress"));
#else
    method = reinterpret_cast<MAIL_AddAddress>(
        dlsym(nHandler, "MAIL_AddAddress"));
#endif
    CheckResult(method(libHandler, email.c_str()));
}

void ACBrMail::AddCorpo(const std::string& corpo) const {
    CHECK_HANDLE();

    MAIL_AddBody method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_AddBody>(
        GetProcAddress(nHandler, "MAIL_AddBody"));
#else
    method = reinterpret_cast<MAIL_AddBody>(
        dlsym(nHandler, "MAIL_AddBody"));
#endif
    CheckResult(method(libHandler, corpo.c_str()));
}

void ACBrMail::AddCorpoAlternativo(const std::string& corpo) const {
    MAIL_AddAltBody method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_AddAltBody>(
        GetProcAddress(nHandler, "MAIL_AddAltBody"));
#else
    method = reinterpret_cast<MAIL_AddAltBody>(
        dlsym(nHandler, "MAIL_AddAltBody"));
#endif
    CheckResult(method(libHandler, corpo.c_str()));
}

void ACBrMail::Enviar() const {
    MAIL_Send method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_Send>(
        GetProcAddress(nHandler, "MAIL_Send"));
#else
    method = reinterpret_cast<MAIL_Send>(
        dlsym(nHandler, "MAIL_Send"));
#endif
    CheckResult(method(libHandler, 1));
}

void ACBrMail::CheckResult(int ret) const {
    if (ret != 0)
        throw std::runtime_error("Erro ACBrMail. Código: " + std::to_string(ret));
}

void ACBrMail::ConfigGravar(const std::string& eArqConfig) const
{
    MAIL_ConfigGravar method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_ConfigGravar>(
        GetProcAddress(nHandler, "MAIL_ConfigGravar"));
#else
    method = reinterpret_cast<MAIL_ConfigGravar>(
        dlsym(nHandler, "MAIL_ConfigGravar"));
#endif

    if (!method)
        throw std::runtime_error("Não encontrou MAIL_ConfigGravar");

    int ret = method(libHandler, eArqConfig.c_str());
    CheckResult(ret);
}

void ACBrMail::LimparAnexos() const
{
    CHECK_HANDLE();

    MAIL_ClearAttachment method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_ClearAttachment>(
        GetProcAddress(nHandler, "MAIL_ClearAttachment"));
#else
    method = reinterpret_cast<MAIL_ClearAttachment>(
        dlsym(nHandler, "MAIL_ClearAttachment"));
#endif

    if (!method)
        throw std::runtime_error("Não encontrou MAIL_ClearAttachment");

    CheckResult(method(libHandler));
}

void ACBrMail::AddAnexo(const std::string& fileName,
                        const std::string& descricao,
                        int disposition) const
{
    CHECK_HANDLE();

    MAIL_AddAttachment method;
#if defined(ISWINDOWS)
    method = reinterpret_cast<MAIL_AddAttachment>(
        GetProcAddress(nHandler, "MAIL_AddAttachment"));
#else
    method = reinterpret_cast<MAIL_AddAttachment>(
        dlsym(nHandler, "MAIL_AddAttachment"));
#endif

    if (!method)
        throw std::runtime_error("Não encontrou MAIL_AddAttachment");

    const char* desc = descricao.empty() ? nullptr : descricao.c_str();

    CheckResult(method(libHandler,
                       fileName.c_str(),
                       desc,
                       disposition));
}

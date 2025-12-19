#pragma once
#include <string>

#if _WIN32 || _WIN64
#define ISWINDOWS
#elif __GNUC__
#define ISUNIX
#endif

#if defined(ISWINDOWS)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define BUFFER_LEN 8192

class ACBrMail {
public:
    ACBrMail(std::string eArqConfig = "",
             std::string eChaveCrypt = "",
             std::string ePathLog = "");
    ~ACBrMail();

    std::string Nome() const;
    std::string Versao() const;

    void ConfigGravarValor(const std::string& eSessao,
                           const std::string& eChave,
                           const std::string& eValor) const;

    void Limpar() const;

    void SetAssunto(const std::string& assunto) const;
    void AddDestinatario(const std::string& email) const;
    void AddCorpo(const std::string& corpo) const;
    void AddCorpoAlternativo(const std::string& corpo) const;
    void ConfigGravar(const std::string& eArqConfig) const;

    void Enviar() const;

private:
#if defined(ISWINDOWS)
    HMODULE nHandler;
#else
    void* nHandler;
#endif
    void* libHandler;

    void CheckResult(int ret) const;
};

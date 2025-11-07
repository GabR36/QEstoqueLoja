#pragma once
#include <string>
#include <cstdint>

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

class ACBrConsultaCNPJ {
public:
    ACBrConsultaCNPJ(std::string eArqConfig = "", std::string eChaveCrypt = "");
    ~ACBrConsultaCNPJ();

    std::string Nome() const;
    std::string Versao() const;
    std::string Consultar(std::string eCNPJ) const;
    void ConfigGravarValor(const std::string& eSessao,
                           const std::string& eChave,
                           const std::string& eValor) const;

    void ConfigGravar(const std::string& eArqConfig) const;


private:
#if defined(ISWINDOWS)
    HMODULE nHandler;
#else
    void* nHandler;
#endif
    void* libHandler;

    void CheckResult(int ret) const;
    std::string ProcessResult(const std::string& buffer, int len) const;
};

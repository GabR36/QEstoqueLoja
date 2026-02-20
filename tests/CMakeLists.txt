cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Test
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Sql Gui Widgets Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Sql Gui Widgets Xml)

if(CMAKE_SYSTEM MATCHES "Windows")
    include(FetchContent)
    FetchContent_Declare(
    zint
    GIT_REPOSITORY https://github.com/zint/zint.git
    GIT_TAG master # Você pode substituir por uma versão específica, se desejar
    )
    FetchContent_MakeAvailable(zint)
elseif(CMAKE_SYSTEM MATCHES "Linux")
    find_package(PNG)
    find_library(ZINT_LIBRARIES NAMES zint)

endif()


# ==============================
# Fontes de teste
# ==============================

set(TEST_SOURCES
    test_main.cpp

    # util
    util/test_dbutil.cpp

    # infra de teste
    db/test_db_factory.cpp

    # services
    services/test_produto_service.cpp
    services/test_barcode_service.cpp
)

# ==============================
# Executável de testes
# ==============================
add_executable(QEstoqueLojaTests
    ${TEST_SOURCES}

    # Código real do projeto usado nos testes
    ${CMAKE_SOURCE_DIR}/src/util/dbutil.cpp
    ${CMAKE_SOURCE_DIR}/src/services/Produto_service.cpp
    ${CMAKE_SOURCE_DIR}/src/util/ibptutil.cpp
    services/test_produto_service.h
    ${CMAKE_SOURCE_DIR}/src/repository/Produto_repository.cpp
    services/test_barcode_service.h services/test_barcode_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/barcode_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/schemamigration_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/schemamigration_service.h
    ${CMAKE_SOURCE_DIR}/src/infra/databaseconnection_service.h
    ${CMAKE_SOURCE_DIR}/src/infra/databaseconnection_service.cpp
    ${CMAKE_SOURCE_DIR}/src/infra/apppath_service.h
    ${CMAKE_SOURCE_DIR}/src/infra/apppath_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/config_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/config_service.h
    ${CMAKE_SOURCE_DIR}/src/repository/config_repository.h
    ${CMAKE_SOURCE_DIR}/src/repository/config_repository.cpp
    services/test_manifestadordfe.h services/test_manifestadordfe.cpp
    ${CMAKE_SOURCE_DIR}/src/util/manifestadordfe.h
    ${CMAKE_SOURCE_DIR}/src/util/manifestadordfe.cpp
    ${CMAKE_SOURCE_DIR}/src/repository/cliente_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/repository/dfeinfo_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/repository/eventofiscal_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/repository/notafiscal_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/repository/produtonota_repository.cpp
    ${CMAKE_SOURCE_DIR}/src/services/cliente_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/dfe_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/eventofiscal_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/notafiscal_service.cpp
    ${CMAKE_SOURCE_DIR}/src/services/produtonota_service.cpp
    ${CMAKE_SOURCE_DIR}/src/util/nfxmlutil.cpp
    ${CMAKE_SOURCE_DIR}/src/util/NFUtilidades.cpp
    ${CMAKE_SOURCE_DIR}/src/util/datautil.cpp
    ${CMAKE_SOURCE_DIR}/src/services/acbr_service.cpp
    ${CMAKE_SOURCE_DIR}/src/util/ACBrConsultaCNPJ.cpp
    ${CMAKE_SOURCE_DIR}/src/util/ACBrMail.cpp
    ${CMAKE_SOURCE_DIR}/src/util/mailmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/util/datautil.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/ACBrNFe.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/cancelnf.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/eventocienciaop.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/nfceacbr.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/nfeacbr.cpp
    ${CMAKE_SOURCE_DIR}/src/nota/acbrmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/util/consultacnpjmanager.cpp
    recursosTest.qrc








)
target_compile_definitions(QEstoqueLojaTests PRIVATE TEST_ENV)

# ==============================
# Includes
# ==============================

target_include_directories(QEstoqueLojaTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/util
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/repository
    ${CMAKE_SOURCE_DIR}/src/nota
    ${CMAKE_SOURCE_DIR}/src/dto
    ${CMAKE_SOURCE_DIR}/tests
)

# ==============================
# Linkagem
# ==============================

target_link_libraries(QEstoqueLojaTests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Gui
    zint
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
)


install(DIRECTORY ${CMAKE_SOURCE_DIR}/recursos/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLojaTests/recursos
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/reports/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLojaTests/reports
)

# --- Definir APPDATA_DIR corretamente ---
if (WIN32)
    # Roaming AppData
    file(TO_CMAKE_PATH "$ENV{APPDATA}" APPDATA_PATH)
    set(APPDATA_DIR_TEST "${APPDATA_PATH}/QEstoqueLojaTests")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (DEFINED ENV{XDG_DATA_HOME} AND NOT "$ENV{XDG_DATA_HOME}" STREQUAL "")
        set(APPDATA_DIR_TEST "$ENV{XDG_DATA_HOME}/QEstoqueLojaTests")
    else()
        set(APPDATA_DIR_TEST "$ENV{HOME}/.local/share/QEstoqueLojaTests")
    endif()
endif()

# --- Copiar diretórios fixos ---
add_custom_command(TARGET QEstoqueLojaTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR_TEST}/reports"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/reports" "${APPDATA_DIR_TEST}/reports"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR_TEST}/recursos"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/recursos" "${APPDATA_DIR_TEST}/recursos"
    COMMENT "Copiando reports e recursos para ${APPDATA_DIR_TEST}"
)
# ==============================
# Registrar no CTest
# ==============================

add_test(
    NAME QEstoqueLojaTests
    COMMAND QEstoqueLojaTests
)

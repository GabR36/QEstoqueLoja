cmake_minimum_required(VERSION 3.5)

project(QEstoqueLoja VERSION 0.1 LANGUAGES CXX)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.6.2/mingw_64/lib/cmake")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml Core Network Core5Compat)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml Core Network Core5Compat)
if (CMAKE_SYSTEM MATCHES "Linux")
    find_package(QuaZip-Qt6)
endif()

if (CMAKE_SYSTEM MATCHES "Windows")
    # find_package(ZLIB REQUIRED)
endif()

if (CMAKE_SYSTEM MATCHES "Linux")
    include_directories(
        /usr/include/QtRPT

    )
elseif (CMAKE_SYSTEM MATCHES "Windows")
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/OpenSSL3.1.3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libiconv-for-Windows/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libxml2-v2.11.5/include/libxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/QtRptProject-Source/QtRptProject/QtRPT
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/zlib-1.3/include
        # ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/quazip/quazip-headers

    )
endif()


set(TS_FILES QEstoqueLoja_pt_BR.ts)
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
set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        ${TS_FILES}
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Imagens/QEstoqueLOja/AppIcon.rc")
       list(APPEND PROJECT_SOURCES ${APP_ICON_RESOURCE_WINDOWS})
endif()
include_directories(${CMAKE_SOURCE_DIR}/src/subclass)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    #set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Imagens/QEstoqueLOja/AppIcon.rc")


    qt_add_executable(QEstoqueLoja
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        #${APP_ICON_RESOURCE_WINDOWS}
        src/alterarproduto.h src/alterarproduto.cpp src/alterarproduto.ui

        src/vendas.h src/vendas.cpp src/vendas.ui
        src/venda.h src/venda.cpp src/venda.ui
        Imagens/recursosImg.qrc
        src/relatorios.h src/relatorios.cpp src/relatorios.ui
        src/pagamento.h src/pagamento.cpp src/pagamento.ui

        src/configuracao.h src/configuracao.cpp src/configuracao.ui
        src/entradasvendasprazo.h src/entradasvendasprazo.cpp src/entradasvendasprazo.ui
        src/pagamentoaprazo.h src/pagamentoaprazo.cpp
        src/delegateprecof2.h src/delegateprecof2.cpp
        src/customdelegate.h src/customdelegate.cpp
        src/delegatehora.h src/delegatehora.cpp
        src/pagamentovenda.cpp
        src/pagamentovenda.h
        src/delegateprecof2.cpp src/delegateprecof2.h
        src/delegatepago.h src/delegatepago.cpp
        src/util/pdfexporter.h src/util/pdfexporter.cpp
        src/subclass/customlineedit.h src/subclass/customlineedit.cpp
        src/delegateprecovalidate.h src/delegateprecovalidate.cpp
        src/delegatelockcol.h src/delegatelockcol.cpp
        src/delegatequant.h src/delegatequant.cpp
        src/clientes.h src/clientes.cpp src/clientes.ui
        src/alterarcliente.h src/alterarcliente.cpp src/alterarcliente.ui
        src/inserircliente.h src/inserircliente.cpp src/inserircliente.ui
        src/subclass/produtotableview.h src/subclass/produtotableview.cpp
        src/subclass/waitdialog.h src/subclass/waitdialog.cpp

        src/inserirproduto.h src/inserirproduto.cpp src/inserirproduto.ui
        src/util/NfUtilidades.h
        src/util/ibptutil.h src/util/ibptutil.cpp
        src/nota/DanfeUtil.h src/nota/DanfeUtil.cpp
        src/util/NFUtilidades.cpp
        src/subclass/leditdialog.h src/subclass/leditdialog.cpp
        src/infojanelaprod.h src/infojanelaprod.cpp src/infojanelaprod.ui

        src/util/helppage.h src/util/helppage.cpp src/util/helppage.ui
        src/nota/acbrmanager.h src/nota/acbrmanager.cpp
        src/nota/ACBrNFe.h src/nota/ACBrNFe.cpp
        src/nota/ACBrNFeImport.h
        src/nota/nfceacbr.h src/nota/nfceacbr.cpp
        src/nota/nfeacbr.h src/nota/nfeacbr.cpp
        src/schemamanager.h src/schemamanager.cpp
        src/nota/cancelnf.h src/nota/cancelnf.cpp
        src/util/ACBrConsultaCNPJ.cpp src/util/ACBrConsultaCNPJ.h
        src/util/consultacnpjmanager.h src/util/consultacnpjmanager.cpp
        src/util/buscarcodigomunicipio.h src/util/buscarcodigomunicipio.cpp
        src/entradas.h src/entradas.cpp src/entradas.ui


        src/nota/eventocienciaop.h src/nota/eventocienciaop.cpp
        src/util/manifestadordfe.h src/util/manifestadordfe.cpp
        src/util/nfxmlutil.h src/util/nfxmlutil.cpp
        src/util/dbutil.h src/util/dbutil.cpp
        src/mergeprodutos.h src/mergeprodutos.cpp src/mergeprodutos.ui
        src/delegatesugerido.h src/delegatesugerido.cpp
        src/util/ACBrMail.cpp src/util/ACBrMail.h
        src/util/mailmanager.h src/util/mailmanager.cpp
        src/janelaemailcontador.h src/janelaemailcontador.cpp src/janelaemailcontador.ui
        src/util/ziputil.h src/util/ziputil.cpp
        src/sobre.h src/sobre.cpp src/sobre.ui
        src/monitorfiscal.h src/monitorfiscal.cpp src/monitorfiscal.ui
        src/subclass/menuitem.h src/subclass/menuitem.cpp

    )



# Define target properties for Android with Qt 6 as:
#    set_property(TARGET QEstoqueLoja APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()

    if(ANDROID)
        add_library(QEstoqueLoja SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

    else()
        #set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Imagens/QEstoqueLOja/AppIcon.rc")
        add_executable(QEstoqueLoja
            ${PROJECT_SOURCES}
            #${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if (WIN32)
    add_library(quazip1-qt6 UNKNOWN IMPORTED)

    set_target_properties(quazip1-qt6 PROPERTIES
        IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/quazip/quazip-lib/quazip1-qt6.lib
        INTERFACE_INCLUDE_DIRECTORIES
            ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/quazip/quazip-headers/
    )
endif()

target_link_libraries(QEstoqueLoja PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Sql Qt6::PrintSupport
    zint Qt6::Charts QtRPT Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Xml
     Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Network
)

#target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lxml2)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lz QuaZip::QuaZip)
elseif (CMAKE_SYSTEM MATCHES "Windows")
    target_link_directories(QEstoqueLoja PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/OpenSSL3.1.3/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libiconv-for-Windows/lib64/Release
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libxml2-v2.11.5/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/QtRptProject-Source/QtRptProject/bin/release/lib
        # ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/quazip/quazip-lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/zlib-1.3/lib
    )
    target_link_libraries(QEstoqueLoja  PRIVATE QtRPT quazip1-qt6 Qt6::Core5Compat zdll)

endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QEstoqueLoja)
endif()
set_target_properties(QEstoqueLoja PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QEstoqueLoja
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# Instalar ícone
install(FILES Imagens/QEstoqueLOja/QeLogo256.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    RENAME qestoqueloja.png
)

# Instalar arquivo .desktop
install(FILES qestoqueloja.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)
# Instalar schemas XML e CSVs no caminho de dados compartilhados
install(DIRECTORY ${CMAKE_SOURCE_DIR}/recursos/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLoja/recursos
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/reports/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLoja/reports
)

# --- Definir APPDATA_DIR corretamente ---
if (WIN32)
    # Roaming AppData
    file(TO_CMAKE_PATH "$ENV{APPDATA}" APPDATA_PATH)
    set(APPDATA_DIR "${APPDATA_PATH}/QEstoqueLoja")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (DEFINED ENV{XDG_DATA_HOME} AND NOT "$ENV{XDG_DATA_HOME}" STREQUAL "")
        set(APPDATA_DIR "$ENV{XDG_DATA_HOME}/QEstoqueLoja")
    else()
        set(APPDATA_DIR "$ENV{HOME}/.local/share/QEstoqueLoja")
    endif()
endif()

# --- Copiar diretórios fixos ---
add_custom_command(TARGET QEstoqueLoja POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR}/reports"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/reports" "${APPDATA_DIR}/reports"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR}/recursos"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/recursos" "${APPDATA_DIR}/recursos"
    COMMENT "Copiando reports e recursos para ${APPDATA_DIR}"
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QEstoqueLoja)
endif()



if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QEstoqueLoja)
endif()


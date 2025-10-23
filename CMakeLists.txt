cmake_minimum_required(VERSION 3.5)

project(QEstoqueLoja VERSION 0.1 LANGUAGES CXX)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.6.2/mingw_64/lib/cmake")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml Core)

if (CMAKE_SYSTEM MATCHES "Linux")
    include_directories(
        /usr/include/QtRPT

    )
elseif (CMAKE_SYSTEM MATCHES "Windows")
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/build/Release/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/zlib-1.3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/OpenSSL3.1.3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libiconv-for-Windows/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libxml2-v2.11.5/include/libxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/QtRptProject-Source/QtRptProject/QtRPT

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
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Imagens/QEstoqueLOja/AppIcon.rc")
       list(APPEND PROJECT_SOURCES ${APP_ICON_RESOURCE_WINDOWS})
endif()
include_directories(${CMAKE_SOURCE_DIR}/subclass)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    #set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Imagens/QEstoqueLOja/AppIcon.rc")


    qt_add_executable(QEstoqueLoja
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        #${APP_ICON_RESOURCE_WINDOWS}
        alterarproduto.h alterarproduto.cpp alterarproduto.ui

        vendas.h vendas.cpp vendas.ui
        venda.h venda.cpp venda.ui
        Imagens/recursosImg.qrc
        relatorios.h relatorios.cpp relatorios.ui
        pagamento.h pagamento.cpp pagamento.ui

        configuracao.h configuracao.cpp configuracao.ui
        entradasvendasprazo.h entradasvendasprazo.cpp entradasvendasprazo.ui
        pagamentoaprazo.h pagamentoaprazo.cpp
        delegateprecof2.h delegateprecof2.cpp
        customdelegate.h customdelegate.cpp
        delegatehora.h delegatehora.cpp
        pagamentovenda.cpp
        pagamentovenda.h
        delegateprecof2.cpp delegateprecof2.h
        delegateprecof2.cpp delegateprecof2.h
        delegateprecof2.cpp delegateprecof2.h
        delegateprecof2.cpp delegateprecof2.h
        delegatepago.h delegatepago.cpp
        util/pdfexporter.h util/pdfexporter.cpp
        subclass/customlineedit.h subclass/customlineedit.cpp
        delegateprecovalidate.h delegateprecovalidate.cpp
        delegatelockcol.h delegatelockcol.cpp
        delegatequant.h delegatequant.cpp
        clientes.h clientes.cpp clientes.ui
        alterarcliente.h alterarcliente.cpp alterarcliente.ui
        inserircliente.h inserircliente.cpp inserircliente.ui
        subclass/produtotableview.h subclass/produtotableview.cpp
        subclass/waitdialog.h subclass/waitdialog.cpp

        inserirproduto.h inserirproduto.cpp inserirproduto.ui
        util/NfUtilidades.h
        util/ibptutil.h util/ibptutil.cpp
        nota/DanfeUtil.h nota/DanfeUtil.cpp
        util/NFUtilidades.cpp
        subclass/leditdialog.h subclass/leditdialog.cpp
        infojanelaprod.h infojanelaprod.cpp infojanelaprod.ui

        util/helppage.h util/helppage.cpp util/helppage.ui
        nota/acbrmanager.h nota/acbrmanager.cpp
        nota/ACBrNFe.h nota/ACBrNFe.cpp
        nota/ACBrNFeImport.h
        nota/nfceacbr.h nota/nfceacbr.cpp
        nota/nfeacbr.h nota/nfeacbr.cpp
        schemamanager.h schemamanager.cpp
        nota/cancelnf.h nota/cancelnf.cpp





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
target_link_libraries(QEstoqueLoja PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Sql Qt6::PrintSupport
    zint Qt6::Charts QtRPT Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Xml
     Qt${QT_VERSION_MAJOR}::Qml
)

#target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lxml2)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lz)
elseif (CMAKE_SYSTEM MATCHES "Windows")
    target_link_directories(QEstoqueLoja PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/zlib-1.3/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/OpenSSL3.1.3/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libiconv-for-Windows/lib64/Release
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/build/Release/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/libxml2-v2.11.5/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/QtRptProject-Source/QtRptProject/bin/release/lib
        #${CMAKE_CURRENT_SOURCE_DIR}/libs-externas/cppbrasil/third-party/windows/QtRptProject3.0/lib/debug/lib
    )
    target_link_libraries(QEstoqueLoja  PRIVATE QtRPT zdll)

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


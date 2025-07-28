cmake_minimum_required(VERSION 3.5)

project(QEstoqueLoja VERSION 0.1 LANGUAGES CXX)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.6.2/mingw_64/lib/cmake")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Qml Xml Svg Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Qml Xml Svg Network)

if (CMAKE_SYSTEM MATCHES "Linux")
    include_directories(
        /usr/include/libxml2
        /usr/include/QtRPT
        # /usr/include/CppBrasil
        # /usr/include/openssl

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

include(FetchContent)
FetchContent_Declare(
  zint
  GIT_REPOSITORY https://github.com/zint/zint.git
  GIT_TAG master # Você pode substituir por uma versão específica, se desejar
)
FetchContent_MakeAvailable(zint)
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
        nota/nfcevenda.h nota/nfcevenda.cpp
        inserirproduto.h inserirproduto.cpp inserirproduto.ui
        util/NfUtilidades.h
        util/ibptutil.h util/ibptutil.cpp
        nota/DanfeUtil.h nota/DanfeUtil.cpp
        util/NFUtilidades.cpp
        subclass/leditdialog.h subclass/leditdialog.cpp
        infojanelaprod.h infojanelaprod.cpp infojanelaprod.ui
        nota/nfevenda.h nota/nfevenda.cpp




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
    zint Qt6::Charts QtRPT Qt6::Xml Qt6::Qml Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg
)

#target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lxml2)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lCppDanfeQtRPT -lxml2 -lCppNFe -lcrypto -lssl -lz)
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
    target_link_libraries(QEstoqueLoja  PRIVATE CppNFe QtRPT CppDanfeQtRPT libcrypto libssl libiconv libxml2 zdll)

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
if (CMAKE_SYSTEM MATCHES "Linux")
    # Caminho do arquivo de origem
    set(ARQUIVO_ORIGEM "${CMAKE_SOURCE_DIR}/reports/*")

    # Caminho do diretório de destino (ao lado do executável no diretório de build)
    set(DIRETORIO_DESTINO "${CMAKE_CURRENT_BINARY_DIR}/reports")

    # Copiar o arquivo após a compilação
    add_custom_command(TARGET QEstoqueLoja POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIRETORIO_DESTINO}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ARQUIVO_ORIGEM}" "${DIRETORIO_DESTINO}"
        COMMENT "Copiando orcamentoReport.xml para a pasta do executável"
    )

    # Caminho do arquivo de origem
    set(ARQUIVOS_RECURSOS "${CMAKE_SOURCE_DIR}/recursos/*")

    # Caminho do diretório de destino (ao lado do executável no diretório de build)
    set(DIRETORIO_DESTINO_RECURSOS "${CMAKE_CURRENT_BINARY_DIR}/recursos")

    # Copiar o arquivo após a compilação
    add_custom_command(TARGET QEstoqueLoja POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIRETORIO_DESTINO_RECURSOS}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ARQUIVOS_RECURSOS}" "${DIRETORIO_DESTINO_RECURSOS}"
        COMMENT "Copiando para a pasta do executável"
    )
endif()
if (CMAKE_SYSTEM MATCHES "Windows")
    # Pega todos os arquivos da pasta reports
    file(GLOB REPORT_FILES "${CMAKE_SOURCE_DIR}/reports/*")

    # Cria o diretório de destino
    add_custom_command(TARGET QEstoqueLoja POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/reports"
    )

    # Loop para adicionar cópia de cada arquivo
    foreach(REPORT_FILE ${REPORT_FILES})
        get_filename_component(REPORT_FILENAME ${REPORT_FILE} NAME)
        add_custom_command(TARGET QEstoqueLoja POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${REPORT_FILE}"
                    "${CMAKE_CURRENT_BINARY_DIR}/reports/${REPORT_FILENAME}"
        )
    endforeach()

    # Pega todos os arquivos da pasta recursos
    file(GLOB RECURSOS_FILES "${CMAKE_SOURCE_DIR}/recursos/*")

    # Cria o diretório de destino
    add_custom_command(TARGET QEstoqueLoja POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/recursos"
    )

    # Loop para adicionar cópia de cada arquivo
    foreach(RECURSOS_FILE ${RECURSOS_FILES})
        get_filename_component(RECURSOS_FILENAME ${RECURSOS_FILE} NAME)
        add_custom_command(TARGET QEstoqueLoja POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RECURSOS_FILE}"
                    "${CMAKE_CURRENT_BINARY_DIR}/recursos/${RECURSOS_FILENAME}"
        )
    endforeach()
endif()


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QEstoqueLoja)
endif()


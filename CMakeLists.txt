cmake_minimum_required(VERSION 3.5)

project(QEstoqueLoja VERSION 0.1 LANGUAGES CXX)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.6.2/mingw_64/lib/cmake")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql PrintSupport Charts Xml Qml)

if (CMAKE_SYSTEM MATCHES "Linux")
    include_directories(
        /usr/include/libxml2
        /usr/include/QtRPT
    )
elseif (CMAKE_SYSTEM MATCHES "Windows")
    include_directories(
        ../../include
        ../../third-party/windows/OpenSSL1.1.1q/include
        ../../third-party/windows/libxml2-v2.9.13/include/libxml2
        ../../third-party/windows/libiconv-for-Windows/include
        ../../third-party/windows/zlib-1.2.12/include
        ../../third-party/windows/QtRptProject3.0/include/QtRPT
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

        config.h config.cpp config.ui
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
    zint Qt6::Charts QtRPT Qt6::Xml Qt6::Qml)
target_link_libraries(QEstoqueLoja  PRIVATE -lQtRPT -lxml2)


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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QEstoqueLoja)
endif()


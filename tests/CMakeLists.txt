    cmake_minimum_required(VERSION 3.5)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    project(QEstoqueLojaTests)
    # Qt Test
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Sql Gui Widgets Xml PrintSupport Network Charts Core5Compat)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Sql Gui Widgets Xml PrintSupport Network Charts Core5Compat)

    if(CMAKE_SYSTEM MATCHES "Linux")
        # find_package(PNG)
        # find_library(ZINT_LIBRARIES NAMES zint)
        # find_package(QuaZip-Qt6)
    endif()

    option(TEST_WITH_POSTGRES "Executar testes usando PostgreSQL" OFF)


    set(TEST_SOURCES
        test_main.cpp

        # util
        util/test_dbutil.cpp

        # infra de teste
        db/test_db_factory.cpp

        # services
        services/test_produto_service.cpp
        services/test_barcode_service.cpp
        services/test_vendas_service.cpp
        services/test_produtovenda_service.cpp
        services/test_fiscalemitter_service.cpp
    )


    add_executable(QEstoqueLojaTests
        ${TEST_SOURCES}

        # Código real do projeto usado nos testes
        services/test_produto_service.h
        services/test_barcode_service.h services/test_barcode_service.cpp
        services/test_manifestadordfe.h services/test_manifestadordfe.cpp
        recursosTest.qrc
        services/test_cliente_service.h services/test_cliente_service.cpp
        services/test_vendas_service.h services/test_vendas_service.cpp
        services/test_produtovenda_service.h services/test_produtovenda_service.cpp
        services/test_fiscalemitter_service.h services/test_fiscalemitter_service.cpp
        services/test_eventofiscal_service.h services/test_eventofiscal_service.cpp


    )
    target_compile_definitions(QEstoqueLojaTests PRIVATE TEST_ENV)


    if(TEST_WITH_POSTGRES)
        target_compile_definitions(QEstoqueLojaTests PRIVATE TEST_POSTGRES)
    endif()

    target_include_directories(QEstoqueLojaTests PRIVATE
        # ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    if (WIN32)
        target_include_directories(QEstoqueLojaTests PRIVATE
            ${CMAKE_SOURCE_DIR}/../libs-externas/quazip/quazip-headers
            ${CMAKE_SOURCE_DIR}/../libs-externas/cppbrasil/third-party/windows/zlib-1.3/include
        )
        target_compile_definitions(QEstoqueLojaTests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    target_link_libraries(QEstoqueLojaTests PRIVATE
        QEstoqueLib
        Qt${QT_VERSION_MAJOR}::Test
    )

    if (CMAKE_SYSTEM MATCHES "Linux")
        target_link_libraries(QEstoqueLojaTests PRIVATE QuaZip::QuaZip)
    elseif (CMAKE_SYSTEM MATCHES "Windows")
        target_link_libraries(QEstoqueLojaTests PRIVATE
            ${CMAKE_SOURCE_DIR}/libs-externas/quazip/quazip-lib/quazip1-qt6.lib
        )
    endif()


    # install(DIRECTORY ${CMAKE_SOURCE_DIR}/recursos/
    #     DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLojaTests/recursos
    # )

    # install(DIRECTORY ${CMAKE_SOURCE_DIR}/reports/
    #     DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/QEstoqueLojaTests/reports
    # )

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

    add_custom_command(TARGET QEstoqueLojaTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR_TEST}/reports"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/reports" "${APPDATA_DIR_TEST}/reports"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDATA_DIR_TEST}/recursos"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/recursos" "${APPDATA_DIR_TEST}/recursos"
        COMMENT "Copiando reports e recursos para ${APPDATA_DIR_TEST}"
    )


    add_test(
        NAME QEstoqueLojaTests
        COMMAND QEstoqueLojaTests
    )

include(GNUInstallDirs)

if (NOT TLCT_HEADER_ONLY)
    install(TARGETS ${TLCT_LIB_NAME}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif ()
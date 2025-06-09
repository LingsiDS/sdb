add_executable(sdb sdb.cpp)

target_link_libraries(sdb PRIVATE sdb::libsdb PkgConfig::readline)

include(GNUInstallDirs)
install(
    TARGETS sdb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # 可执行文件安装到 /bin 或 /usr/bin
)
add_library(libsdb process.cpp)
add_library(sdb::libsdb ALIAS libsdb)

set_target_properties(libsdb PROPERTIES OUTPUT_NAME sdb)

# use c++17, public for other projects that link to this library
target_compile_features(libsdb PUBLIC cxx_std_17) 

target_include_directories(
    libsdb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  # 开发阶段，使用源码路径
        $<INSTALL_INTERFACE:include>                    # 安装后，使用标准路径
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include                 # 私有头文件路径，不暴露给外部
)

include(GNUInstallDirs)  # 引入标准安装目录变量（如 ${CMAKE_INSTALL_LIBDIR} 对应 /lib 或 /lib64）
install(TARGETS libsdb
    EXPORT sdb-targets           # 导出目标信息，供其他项目导入
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # 库文件安装路径（如 /lib/libsdb.so）
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  # 静态库安装路径（如 /lib/libsdb.a）
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # 可执行文件安装路径（若有）
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  # 头文件安装路径（如 /include/）
)


install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  # 将 include/libsdb 目录安装到 /include/ 下
)


install(
    EXPORT sdb-targets              # 导出之前定义的目标信息
    FILE sdb-config.cmake           # 生成配置文件 sdb-config.cmake
    NAMESPACE sdb::                 # 为目标添加命名空间（避免与其他库冲突）
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdb  # 配置文件安装路径
)
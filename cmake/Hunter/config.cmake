hunter_config(ZLIB VERSION 1.2.8-p3 CONFIGURATION_TYPES Release)
hunter_config(Protobuf VERSION 3.0.0-p1 CONFIGURATION_TYPES Release)
hunter_config(MySQL-client VERSION 6.1.9 CONFIGURATION_TYPES Release
        CMAKE_ARGS
        INSTALL_INCLUDEDIR=include/mysql)
hunter_config(Boost VERSION 1.63.0 CONFIGURATION_TYPES Release
        CMAKE_ARGS CMAKE_CXX_STANDARD=14)
hunter_config(OpenSSL VERSION 1.0.2j CONFIGURATION_TYPES Release)
hunter_config(rabbitmq-c VERSION 0.7.0-p1 CONFIGURATION_TYPES Release
        CMAKE_ARGS BUILD_SHARED_LIBS=OFF
        )
hunter_config(
  OpenSSL
  VERSION 1.1.1j
)

hunter_config(
  gumbo
  VERSION 0.10.1
)

hunter_config(
  jsoncpp
  VERSION 1.8.0
)

hunter_config(
  sqlite3
  VERSION 3.30.1-p0
)

hunter_config(
  CURL
  VERSION 7.60.0-p2
  CMAKE_ARGS
    CMAKE_USE_SCHANNEL=OFF
    BUILD_CURL_TESTS=OFF
    BUILD_CURL_EXE=OFF
    CMAKE_USE_OPENSSL=ON
    CMAKE_USE_LIBSSH2=OFF
    BUILD_TESTING=OFF
)
#include "SslContext.h"

SslContext::SslContext(const SslConfig& config)
    : ctx_(nullptr), config_(config) {}

SslContext::~SslContext() {
  try {
    if (ctx_) {
      // 调用SSL_CTX_free释放SSL_CTX对象
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  } catch (const std::exception& e) {
    handle_ssl_error("Failed to free SSL context");
  }
}

bool SslContext::initialize() {
  try {
    // 初始化OpenSSL，加载 SSL/TLS 相关的错误字符串，加载加密库的错误字符串
    OPENSSL_init_ssl(
        OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS,
        nullptr);

    // 创建SSL上下文
    const SSL_METHOD* method = TLS_server_method();
    // SSL_CTX_new创建SSL_CTX对象
    ctx_ = SSL_CTX_new(method);
    if (!ctx_) {
      handle_ssl_error("Failed to create SSL context");
      return false;
    }

    // 设置SSL选项
    long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION |
                   SSL_OP_CIPHER_SERVER_PREFERENCE;

    SSL_CTX_set_options(ctx_, options);

    // 加载证书和私钥
    if (!load_certificates()) {
      return false;
    }

    // 设置协议版本
    if (!setup_protocol()) {
      return false;
    }

    // 设置会话缓存
    setup_session_cache();

    LOG_INFO << "SSL context initialized successfully";
    return true;
  } catch (const std::exception& e) {
    handle_ssl_error("Failed to initialize SSL context");
    return false;
  }
}

SSL_CTX* SslContext::get_native_handle() const { return ctx_; }

bool SslContext::load_certificates() {
  try {
    // 加载证书
    if (SSL_CTX_use_certificate_file(ctx_, config_.certificate_file().c_str(),
                                     SSL_FILETYPE_PEM) <= 0) {
      handle_ssl_error("Failed to load certificate file");
      return false;
    }

    // 加载私钥
    if (SSL_CTX_use_PrivateKey_file(ctx_, config_.private_key_file().c_str(),
                                    SSL_FILETYPE_PEM) <= 0) {
      handle_ssl_error("Failed to load private key file");
      return false;
    }

    // 验证私钥
    if (!SSL_CTX_check_private_key(ctx_)) {
      handle_ssl_error("Failed to verify private key");
      return false;
    }

    // 加载证书链
    if (!config_.certificate_chain_file().empty()) {
      if (SSL_CTX_use_certificate_chain_file(
              ctx_, config_.certificate_chain_file().c_str()) <= 0) {
        handle_ssl_error("Failed to load certificate chain file");
        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    handle_ssl_error("Failed to load certificates");
    return false;
  }
}

bool SslContext::setup_protocol() {
  try {
    // 设置SSL/TLS协议版本
    long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
    switch (config_.protocol_version()) {
      case SSLVersion::TLS_1_0:
        options |= SSL_OP_NO_TLSv1;
        break;
      case SSLVersion::TLS_1_1:
        options |= SSL_OP_NO_TLSv1_1;
        break;
      case SSLVersion::TLS_1_2:
        options |= SSL_OP_NO_TLSv1_2;
        break;
      case SSLVersion::TLS_1_3:
        options |= SSL_OP_NO_TLSv1_3;
        break;
      default:
        break;
    }
    SSL_CTX_set_options(ctx_, options);

    // 设置加密套件
    if (!config_.cipher_list().empty()) {
      if (SSL_CTX_set_cipher_list(ctx_, config_.cipher_list().c_str()) <= 0) {
        handle_ssl_error("Failed to set cipher list");
        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    handle_ssl_error("Failed to setup protocol");
    return false;
  }
}

void SslContext::setup_session_cache() {
  try {
    SSL_CTX_set_session_cache_mode(ctx_, SSL_SESS_CACHE_SERVER);
    SSL_CTX_sess_set_cache_size(ctx_, config_.session_cache_size());
    SSL_CTX_set_timeout(ctx_, config_.session_timeout());
  } catch (const std::exception& e) {
    handle_ssl_error("Failed to setup session cache");
  }
}

void SslContext::handle_ssl_error(const char* message) {
  char error_buffer[256];
  ERR_error_string_n(ERR_get_error(), error_buffer, sizeof(error_buffer));
  LOG_ERROR << message << ": " << error_buffer;
}
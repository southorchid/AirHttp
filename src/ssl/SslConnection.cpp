#include "SslConnection.h"

SslConnection::SslConnection(const muduo::net::TcpConnectionPtr& conn,
                             SslContext* ctx)
    : ssl_(nullptr),
      ctx_(ctx),
      conn_(conn),
      state_(SSLState::HANDSHAKE),
      read_bio_(nullptr),
      write_bio_(nullptr),
      message_callback_(nullptr) {
  try {
    // 创建一个新的 SSL对象。该对象表示一个SSL/TLS会话，可用于管理安全通信。
    ssl_ = SSL_new(ctx_->get_native_handle());
    if (!ssl_) {
      LOG_ERROR << "Failed to create SSL object:"
                << ERR_error_string(ERR_get_error(), nullptr);
      return;
    }

    // 创建新的 BIO（Basic Input/Output）对象，用于处理输入/输出操作。
    read_bio_ = BIO_new(BIO_s_mem());
    write_bio_ = BIO_new(BIO_s_mem());

    if (!read_bio_ || !write_bio_) {
      LOG_ERROR << "Failed to create BIO object";
      // 如果创建 BIO 对象失败，则释放 SSL 对象
      SSL_free(ssl_);
      ssl_ = nullptr;
      return;
    }

    // 将 SSL 对象与 BIO 对象关联起来。
    SSL_set_bio(ssl_, read_bio_, write_bio_);

    // 将 SSL 对象设置为服务器模式，SSL 对象将准备接受客户端的连接请求。
    SSL_set_accept_state(ssl_);

    // 设置SSL选项
    SSL_set_mode(ssl_, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    SSL_set_mode(ssl_, SSL_MODE_ENABLE_PARTIAL_WRITE);

    // 设置 TCP 连接的消息回调函数，用于处理 TCP 连接收到的消息。
    conn_->setMessageCallback(
        std::bind(&SslConnection::on_read, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
  } catch (const std::exception& e) {
    LOG_ERROR << "Failed to create SSL connection: " << e.what();
  }
}

SslConnection::~SslConnection() {
  if (ssl_ != nullptr) {
    SSL_free(ssl_);  // 会同时释放关联的 BIO 对象
    ssl_ = nullptr;
  }
}

void SslConnection::start_handshake() {
  // 开始 SSL 握手过程。
  SSL_set_accept_state(ssl_);
  handle_handshake();
}

void SslConnection::send(void* data, size_t len) {
  if (state_ != SSLState::ESTABLISHED) {
    LOG_ERROR << "SSL connection is not established yet";
    return;
  }

  // SSL_write 返回成功发送的字节数或错误代码
  int write_bytes = SSL_write(ssl_, data, len);
  if (write_bytes <= 0) {
    // SSL_get_error 根据给定的 SSL 对象 s 和返回码 ret_code 确定最近一次 SSL
    // 操作的错误类型。它返回一个整数值，表示具体的错误代码
    int err = SSL_get_error(ssl_, write_bytes);
    LOG_ERROR << "Failed to write data to SSL connection: "
              << ERR_error_string(err, nullptr);
    return;
  }

  char buf[4096];
  memset(buf, 0, sizeof(buf));

  int pending = 0;
  // BIO_pending 返回挂起数据的字节数，并将结果强制转换为 int 类型
  while ((pending = BIO_pending(write_bio_)) > 0) {
    // BIO_read 是一个用于从指定的 BIO 对象 b 中读取数据的函数。
    // 它将最多 dlen 字节的数据读取到缓冲区 data 中，并返回实际读取的字节数。
    int bytes = BIO_read(write_bio_, buf,
                         std::min(pending, static_cast<int>(sizeof(buf))));
    if (bytes > 0) {
      // 将数据发送到 TCP 连接
      conn_->send(buf, bytes);
    }
  }
}

void SslConnection::on_read(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buf,
                            muduo::Timestamp receiveTime) {
  if (state_ == SSLState::HANDSHAKE) {
    // 如果处于握手阶段，则将数据写入BIO
    BIO_write(read_bio_, buf->peek(), buf->readableBytes());
    buf->retrieve(buf->readableBytes());
    handle_handshake();
    return;
  }

  else if (state_ == SSLState::ESTABLISHED) {
    // 如果处于已建立连接状态，则将数据解密后写入缓冲区
    char decrypted_data[4096];
    memset(decrypted_data, 0, sizeof(decrypted_data));
    int decrypted_bytes =
        SSL_read(ssl_, decrypted_data, sizeof(decrypted_data));
    if (decrypted_bytes > 0) {
      // 创建新的缓冲区，存储解密后的数据
      muduo::net::Buffer decrypted_buf;
      decrypted_buf.append(decrypted_data, decrypted_bytes);
      // 调用消息回调函数处理解密后的数据
      if (message_callback_) {
        message_callback_(conn, &decrypted_buf, receiveTime);
      }
    }
  }
}

bool SslConnection::is_hand_shake_done() const {
  return state_ == SSLState::ESTABLISHED;
}

muduo::net::Buffer* SslConnection::decrypted_buffer() {
  return &decrypted_buffer_;
}

int SslConnection::bio_read(BIO* bio, char* data, int len) {
  SslConnection* conn = static_cast<SslConnection*>(BIO_get_data(bio));
  if (!conn) {
    return -1;
  }

  size_t readable = conn->read_buffer_.readableBytes();
  if (readable == 0) {
    return -1;
  }

  size_t to_read = std::min(static_cast<size_t>(len), readable);
  memcpy(data, conn->read_buffer_.peek(), to_read);
  conn->read_buffer_.retrieve(to_read);
  return static_cast<int>(to_read);
}

int SslConnection::bio_write(BIO* bio, const char* data, int len) {
  SslConnection* conn = static_cast<SslConnection*>(BIO_get_data(bio));
  if (!conn) {
    return -1;
  }

  conn->conn_->send(data, len);
  return len;
}

long SslConnection::bio_ctrl(BIO* bio, int cmd, long num, void* ptr) {
  switch (cmd) {
    case BIO_CTRL_FLUSH:
      return 1;
    default:
      return 0;
  }
}

void SslConnection::message_callback(const message_callback_t& cb) {
  message_callback_ = cb;
}

void SslConnection::handle_handshake() {
  int ret = SSL_do_handshake(ssl_);
  if (ret == 1) {
    // 握手成功，切换到已建立连接状态
    state_ = SSLState::ESTABLISHED;
    LOG_INFO << "SSL handshake completed successfully";
    LOG_INFO << "Using cipher suite: " << SSL_get_cipher(ssl_);
    LOG_INFO << "Protocol version: " << SSL_get_version(ssl_);

    // 握手成功后，确保设置了正确的消息回调函数
    if (!message_callback_) {
      LOG_ERROR << "SSL connection has no message callback";
    }
    return;
  }

  int err = SSL_get_error(ssl_, ret);
  switch (err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      // 正常握手过程，需要继续
      break;
    default: {
      // 其他错误，获取错误的详细信息
      char err_buf[1024];
      unsigned long err_code = ERR_get_error();
      ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
      LOG_ERROR << "SSL handshake failed: " << err_buf;
      conn_->shutdown();
      break;
    }
  }
}

void SslConnection::on_encrypted(const char* data, size_t len) {
  write_buffer_.append(data, len);
  conn_->send(&write_buffer_);
}

void SslConnection::on_decrypted(const char* data, size_t len) {
  decrypted_buffer_.append(data, len);
}

SSLError SslConnection::get_last_error(int ret) {
  int err = SSL_get_error(ssl_, ret);
  switch (err) {
    case SSL_ERROR_NONE:
      return SSLError::NONE;
    case SSL_ERROR_WANT_READ:
      return SSLError::WANT_READ;
    case SSL_ERROR_WANT_WRITE:
      return SSLError::WANT_WRITE;
    case SSL_ERROR_SYSCALL:
      return SSLError::SYSALL;
    case SSL_ERROR_SSL:
      return SSLError::SSL;
    default:
      return SSLError::UNKNOWN;
  }
}

void SslConnection::handle_error(SSLError err) {
  switch (err) {
    case SSLError::WANT_READ:
    case SSLError::WANT_WRITE:
      // 需要等待更多数据或写入缓冲区可用
      break;
    case SSLError::SSL:
    case SSLError::SYSALL:
    case SSLError::UNKNOWN: {
      LOG_ERROR << "SSL error occurred: "
                << ERR_error_string(ERR_get_error(), nullptr);
      state_ = SSLState::ERROR;
      conn_->shutdown();
      break;
    }
    default:
      break;
  }
}

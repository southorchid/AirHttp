#pragma once

#include <muduo/base/noncopyable.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/TcpConnection.h>

#include <memory>

#include "SslContext.h"

class SslConnection : private muduo::noncopyable {
 public:
  using message_callback_t =
      std::function<void(const std::shared_ptr<muduo::net::TcpConnection>&,
                         muduo::net::Buffer*, muduo::Timestamp)>;

  SslConnection(const muduo::net::TcpConnectionPtr& conn, SslContext* ctx);
  ~SslConnection();

  // 开始SSL握手
  void start_handshake();

  // 发送数据
  void send(void* data, size_t len);

  // 处理 SSL 连接中的读取事件
  void on_read(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf, muduo::Timestamp receiveTime);

  // 判断是否握手完成
  bool is_hand_shake_done() const;

  // 获取解密后的数据
  muduo::net::Buffer* decrypted_buffer();

  // SSL BIO操作回调
  static int bio_read(BIO* bio, char* data, int len);
  static int bio_write(BIO* bio, const char* data, int len);

  static long bio_ctrl(BIO* bio, int cmd, long num, void* ptr);

  // 设置消息回调
  void message_callback(const message_callback_t& cb);

 private:
  void handle_handshake();
  void on_encrypted(const char* data, size_t len);
  void on_decrypted(const char* data, size_t len);
  SSLError get_last_error(int ret);
  void handle_error(SSLError err);

 private:
  SSL* ssl_;                             // SSL连接
  SslContext* ctx_;                      // SSL上下文
  muduo::net::TcpConnectionPtr conn_;    // TCP连接
  SSLState state_;                       // SSL状态
  BIO* read_bio_;                        // 网络数据->SSL数据
  BIO* write_bio_;                       // SSL数据->网络数据
  muduo::net::Buffer read_buffer_;       // 读取缓冲区
  muduo::net::Buffer write_buffer_;      // 写入缓冲区
  muduo::net::Buffer decrypted_buffer_;  // 解密后数据
  message_callback_t message_callback_;  // 消息回调
};
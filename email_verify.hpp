#pragma once

#include "common.hpp"

namespace purecpp {
// 邮箱验证工具类
class email_verify_t {
public:
  // 生成随机token
  static std::string generate_token() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::string token;
    token.reserve(32);

    for (int i = 0; i < 32; i++) {
      token += "0123456789abcdef"[dis(gen)];
    }

    return token;
  }

  // 获取token过期时间（24小时后）
  static uint64_t get_token_expires_at() {
    auto now = std::chrono::system_clock::now();
    auto expires = now + std::chrono::hours(24);
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               expires.time_since_epoch())
        .count();
  }

  // 创建邮箱验证token并存储到数据库
  static std::pair<bool, std::string>
  create_verify_token(uint64_t user_id, const std::string &email) {
    auto &db_pool = connection_pool<dbng<mysql>>::instance();
    auto conn = db_pool.get();
    if (conn == nullptr) {
      CINATRA_LOG_ERROR << "获取数据库连接失败";
      return std::make_pair(false, "获取数据库连接失败");
    }

    // 先删除该用户已有的邮箱验证token
    conn->delete_records_s<users_token_t>("user_id = ? and token_type = ?",
                                          user_id, TokenType::VERIFY_EMAIL);

    // 插入新的token记录
    std::string token = generate_token();
    users_token_t token_record{
        .id = 0,
        .user_id = user_id,
        .token_type = TokenType::VERIFY_EMAIL,
        .created_at = get_timestamp_milliseconds(),
        .expires_at = get_token_expires_at(),
    };
    std::copy(token.begin(), token.end(), token_record.token.data());

    auto result = conn->get_insert_id_after_insert(token_record);
    if (result == 0) {
      CINATRA_LOG_ERROR << "存储邮箱验证token失败";
      return std::make_pair(false, "存储邮箱验证token失败");
    }

    return std::make_pair(true, token);
  }

  // 验证token有效性
  static bool verify_email_token(const std::string &token) {
    auto &db_pool = connection_pool<dbng<mysql>>::instance();
    auto conn = db_pool.get();
    if (conn == nullptr) {
      CINATRA_LOG_ERROR << "获取数据库连接失败";
      return false;
    }

    // 验证token是否存在
    if (token.empty()) {
      CINATRA_LOG_ERROR << "token为空";
      return false;
    }

    // 查询数据库token是否存在
    auto users_token = conn->select(ormpp::all)
                           .from<users_token_t>()
                           .where(col(&users_token_t::token).param() &&
                                  col(&users_token_t::token_type).param())
                           .collect(token, TokenType::VERIFY_EMAIL);

    if (users_token.empty()) {
      CINATRA_LOG_ERROR << "token不存在";
      return false;
    }

    auto user_token = users_token.front();
    // 验证token是否过期
    if (user_token.expires_at < get_timestamp_milliseconds()) {
      CINATRA_LOG_ERROR << "token已过期";
      return false;
    }
    return true;
  }

  // 发送邮箱验证邮件 - 使用common.hpp中的通用函数
  static async_simple::coro::Lazy<bool>
  send_verify_email(const std::string &email, const std::string &token) {
    // 直接使用common.hpp中定义的send_verify_email函数
    return purecpp::send_verify_email(email, token);
  }
};

} // namespace purecpp
#pragma once
#include <cinttypes>
#include <string>

#include <ormpp/connection_pool.hpp>
#include <ormpp/dbng.hpp>
#include <ormpp/mysql.hpp>
using namespace ormpp;

namespace purecpp {
// database config
struct db_config {
  std::string db_ip;
  int db_port;
  std::string db_name;
  std::string db_user_name;
  std::string db_pwd;

  int db_conn_num;
  int db_conn_timeout; // seconds
  
  // SMTP配置
  std::string smtp_host;
  int smtp_port;
  std::string smtp_user;
  std::string smtp_password;
  std::string smtp_from_email;
  std::string smtp_from_name;
};

struct users_t {
  uint64_t id;
  std::string user_name; // unique, not null
  std::string email;    // unique, not null
  std::string pwd_hash;           // not null - 从std::string_view改为std::string以解决编译问题
  int is_verifyed;                // 邮箱是否已验证
  uint64_t created_at;
  uint64_t last_active_at; // 最后活跃时间
};

inline constexpr std::string_view get_alias_struct_name(users_t *) {
  return "users"; // 表名默认结构体名字(users_t), 这里可以修改表名
}

template <typename T> struct rest_response {
  bool success = true;
  std::string message;
  std::optional<std::vector<std::string>> errors;
  std::optional<T> data;
  std::string timestamp;
  int code = 200;
};

// 修改密码相关结构体
struct change_password_info {
  uint64_t user_id;
  std::string old_password;
  std::string new_password;
};

struct change_password_resp_data {
  bool success;
  std::string message;
};

// 忘记密码相关结构体
struct forgot_password_info {
  std::string email;
};

struct reset_password_info {
  std::string token;
  std::string new_password;
};

// 密码重置token表
struct password_reset_tokens_t {
  uint64_t id;
  uint64_t user_id;
  std::string token;
  uint64_t created_at;
  uint64_t expires_at;
};

inline constexpr std::string_view get_alias_struct_name(password_reset_tokens_t *) {
  return "password_reset_tokens"; // 表名
}
} // namespace purecpp
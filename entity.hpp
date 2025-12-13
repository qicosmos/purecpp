#pragma once
#include <optional>
#include <ormpp/connection_pool.hpp>
#include <ormpp/dbng.hpp>
#include <ormpp/mysql.hpp>
#include <string>
using namespace ormpp;

namespace purecpp {
// 用户等级枚举
enum class UserLevel : uint32_t {
    LEVEL_1 = 1,   // 等级1 - 新手
    LEVEL_2 = 2,   // 等级2 - 入门
    LEVEL_3 = 3,   // 等级3 - 进阶
    LEVEL_4 = 4,   // 等级4 - 熟练
    LEVEL_5 = 5,   // 等级5 - 专家
    LEVEL_6 = 6,   // 等级6 - 大师
    LEVEL_7 = 7,   // 等级7 - 宗师
    LEVEL_8 = 8,   // 等级8 - 传奇
    LEVEL_9 = 9,   // 等级9 - 神话
    LEVEL_10 = 10  // 等级10 - 不朽
};

// 用户头衔枚举
enum class UserTitle : uint32_t {
    NEWBIE = 0,           // 新手
    DEVELOPER = 1,        // 开发者
    SENIOR_DEVELOPER = 2, // 高级开发者
    ENGINEER = 3,         // 工程师
    SENIOR_ENGINEER = 4,  // 高级工程师
    ARCHITECT = 5,        // 架构师
    TECH_LEAD = 6,        // 技术负责人
    EXPERT = 7,           // 专家
    MASTER = 8,           // 大师
    LEGEND = 9            // 传奇
};

// database config
struct db_config {
  std::string db_ip;
  int db_port;
  std::string db_name;
  std::string db_user_name;
  std::string db_pwd;

  int db_conn_num;
  int db_conn_timeout;  // seconds
};

// smtp config
struct smtp_config {
  std::string smtp_host;
  int smtp_port;
  std::string smtp_user;
  std::string smtp_password;
  std::string smtp_from_email;
  std::string smtp_from_name;
  std::string reset_password_url;
};

struct users_t {
  uint64_t id;
  std::array<char, 21> user_name;  // unique, not null
  std::array<char, 254> email;      // unique, not null
  std::string
      pwd_hash;  // not null - 从std::string_view改为std::string以解决编译问题
  int is_verifyed;  // 邮箱是否已验证
  uint64_t created_at;
  uint64_t last_active_at;  // 最后活跃时间
  
  // 用户身份信息
  UserTitle title;        // 头衔枚举
  std::string role;       // 角色，如"user"、"admin"、"moderator"
  uint64_t experience;    // 经验值
  UserLevel level;        // 用户等级枚举
  
  // 个人资料信息
  std::optional<std::string> bio;  // 个人简介
  std::optional<std::string> avatar;  // 头像URL
};
// 注册users_t的主键
REGISTER_AUTO_KEY(users_t, id);

inline constexpr std::string_view get_alias_struct_name(users_t *) {
  return "users";  // 表名默认结构体名字(users_t), 这里可以修改表名
}

// 密码重置token表
struct password_reset_tokens_t {
  uint64_t id;
  uint64_t user_id;
  std::array<char, 129> token;
  uint64_t created_at;
  uint64_t expires_at;
};
// 注册password_reset_tokens_t的主键
REGISTER_AUTO_KEY(password_reset_tokens_t, id);

inline constexpr std::string_view get_alias_struct_name(
    password_reset_tokens_t *) {
  return "password_reset_tokens";  // 表名
}

}  // namespace purecpp


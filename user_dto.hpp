#pragma once
#include <cinttypes>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "entity.hpp" // 包含必要的枚举类型

namespace purecpp {
// 注册相关结构体
struct register_info {
  std::string username;
  std::string email;
  std::string password;
  std::string cpp_answer;
  size_t question_index;
};

// 用户响应数据结构
struct user_resp_data {
  uint64_t user_id;
  std::string username;
  std::string email;
  bool is_verifyed;
  UserTitle title;
  std::string role;
  uint64_t experience;
  UserLevel level;
};

// 登录相关结构体
struct login_info {
  std::string username;
  std::string password;
};

struct login_resp_data {
  uint64_t user_id;
  std::string username;
  std::string email;
  std::string token;
  UserTitle title;
  std::string role;
  uint64_t experience;
  UserLevel level;
};
} // namespace purecpp
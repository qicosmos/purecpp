#pragma once

#include "entity.hpp"
#include "jwt_token.hpp"
#include "user_aspects.hpp"

using namespace cinatra;

namespace purecpp {
// 前向声明（已经在jwt_token.hpp中定义）
class token_blacklist;

class user_login_t {
 public:
  /**
   * @brief 处理用户登录请求
   *
   * @param req HTTP请求对象
   * @param resp HTTP响应对象
   */
  void handle_login(coro_http_request& req, coro_http_response& resp) {
    // 移除可能导致崩溃的全局语言环境设置
    login_info info = std::any_cast<login_info>(req.get_user_data());

    // 查询数据库
    auto conn = connection_pool<dbng<mysql>>::instance().get();
    conn_guard guard(conn);

    // 先尝试通过用户名查找
    auto users_by_name = conn->query_s<users_t>("user_name = ?", info.username);

    users_t user{};
    bool found = false;

    // 如果用户名存在
    if (!users_by_name.empty()) {
      user = users_by_name[0];
      found = true;
    }
    else {
      // 尝试通过邮箱查找
      auto users_by_email = conn->query_s<users_t>("email = ?", info.username);
      if (!users_by_email.empty()) {
        user = users_by_email[0];
        found = true;
      }
    }

    if (!found) {
      // 用户不存在
      rest_response<std::string_view> data{false, std::string(PURECPP_ERROR_LOGIN_FAILED)};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::bad_request, std::move(json));
      return;
    }

    // 验证密码
    if (user.pwd_hash != info.password) {
      rest_response<std::string_view> data{false, std::string(PURECPP_ERROR_LOGIN_FAILED)};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::bad_request, std::move(json));
      return;
    }

    // 将std::array转换为std::string
    std::string user_name_str(user.user_name.data());
    std::string email_str(user.email.data());

    // 生成JWT token
    std::string token = generate_jwt_token(user.id, user_name_str, email_str);

    // 更新最后活跃时间
    user.last_active_at = get_timestamp_milliseconds();
    conn->update<users_t>(user, "id = ?", user.id);

    // 返回登录成功响应
    login_resp_data login_data{user.id, user_name_str, email_str, token, user.title, user.role, user.experience, user.level};
    rest_response<login_resp_data> data{};
    data.success = true;
    data.message = "登录成功";
    data.data = login_data;

    std::string json;
    iguana::to_json(data, json);

    resp.set_status_and_content(status_type::ok, std::move(json));
  }

  /**
   * @brief 处理用户退出登录请求
   *
   * @param req HTTP请求对象
   * @param resp HTTP响应对象
   */
  void handle_logout(cinatra::coro_http_request& req,
                     cinatra::coro_http_response& resp) {
    // 从请求头获取令牌
    std::string token;
    auto headers = req.get_headers();
    for (auto& header : headers) {
      if (cinatra::iequal0(header.name, "Authorization")) {
        // 提取Bearer令牌
        std::string_view auth_header = header.value;
        if (auth_header.size() > 7 && auth_header.substr(0, 7) == "Bearer ") {
          token = std::string(auth_header.substr(7));
          break;
        }
      }
    }

    // 如果请求头中没有令牌，尝试从查询参数获取
    if (token.empty()) {
      auto token_param = req.get_query_value("token");
      if (!token_param.empty()) {
        token = std::string(token_param);
      }
    }

    // 如果没有令牌，直接返回成功
    if (token.empty()) {
      rest_response<std::string> data{true, "退出登录成功"};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(cinatra::status_type::ok, std::move(json));
      return;
    }

    // 将令牌添加到黑名单
    token_blacklist::instance().add(token);

    // 返回成功响应
    rest_response<std::string> data{true, "退出登录成功"};
    std::string json;
    iguana::to_json(data, json);
    resp.set_status_and_content(cinatra::status_type::ok, std::move(json));
  }
};
}  // namespace purecpp
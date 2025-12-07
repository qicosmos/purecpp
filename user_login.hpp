#pragma once

#include "entity.hpp"
#include "user_aspects.hpp"
#include "jwt_token.hpp"

using namespace cinatra;

namespace purecpp {
class user_login_t {
public:
  void handle_login(coro_http_request &req, coro_http_response &resp) {
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
    } else {
      // 尝试通过邮箱查找
      auto users_by_email = conn->query_s<users_t>("email = ?", info.username);
      if (!users_by_email.empty()) {
        user = users_by_email[0];
        found = true;
      }
    }
    
    if (!found) {
      // 用户不存在
      rest_response<std::string_view> data{false, "用户名或密码错误"};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::bad_request, std::move(json));
      return;
    }
    
    // 验证密码
    if (user.pwd_hash != info.password) {
      rest_response<std::string_view> data{false, "用户名或密码错误"};
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
    login_resp_data login_data{user.id, user_name_str, email_str, token};
    rest_response<login_resp_data> data{};
    data.success = true;
    data.message = "登录成功";
    data.data = login_data;
    
    std::string json;
    iguana::to_json(data, json);
    
    resp.set_status_and_content(status_type::ok, std::move(json));
  }
  

};
} // namespace purecpp
#pragma once

#include "entity.hpp"
#include "user_aspects.hpp"

using namespace cinatra;

namespace purecpp {
class user_password_t {
public:
  void handle_change_password(coro_http_request &req, coro_http_response &resp) {
    change_password_info info = std::any_cast<change_password_info>(req.get_user_data());

    // 查询数据库
    auto conn = connection_pool<dbng<mysql>>::instance().get();
    conn_guard guard(conn);
    
    // 根据用户ID查找用户
    auto users = conn->query_s<users_t>("id = ?", info.user_id);
    
    if (users.empty()) {
      // 用户不存在
      rest_response<std::string_view> data{false, "用户不存在"};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::bad_request, std::move(json));
      return;
    }
    
    users_t user = users[0];
    
    // 验证旧密码
    if (user.pwd_hash != info.old_password) {
      rest_response<std::string_view> data{false, "旧密码错误"};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::bad_request, std::move(json));
      return;
    }
    
    // 使用直接的SQL语句更新密码
    std::string update_sql = "UPDATE users SET pwd_hash = '" + info.new_password + "' WHERE id = " + std::to_string(info.user_id);
    auto result = conn->execute(update_sql);
    
    if (result != 1) {
      rest_response<std::string_view> data{false, "修改密码失败"};
      std::string json;
      iguana::to_json(data, json);
      resp.set_status_and_content(status_type::internal_server_error, std::move(json));
      return;
    }
    
    // 返回修改成功响应
    rest_response<std::string_view> data{};
    data.success = true;
    data.message = "密码修改成功";
    data.data = "密码修改成功";
    
    std::string json;
    iguana::to_json(data, json);
    
    resp.set_status_and_content(status_type::ok, std::move(json));
  }
};
} // namespace purecpp
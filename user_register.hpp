#pragma once

#include "entity.hpp"
#include "user_aspects.hpp"

using namespace cinatra;

namespace purecpp {

class user_register_t {
public:
  void handle_register(coro_http_request &req, coro_http_response &resp) {
    register_info info = std::any_cast<register_info>(req.get_user_data());

    // save to database
    auto conn = connection_pool<dbng<mysql>>::instance().get();
    conn_guard guard(conn);
    users_t user{.id = 0,
                 .user_name = info.username,
                 .email = info.email,
                 .pwd_hash = info.password,
                 .is_verifyed = false,
                 .created_at = get_timestamp_milliseconds(),
                 .last_active_at = 0};

    uint64_t id = conn->get_insert_id_after_insert(user);
    if (id == 0) {
      auto err = conn->get_last_error();
      std::cout << err << "\n";
      resp.set_status_and_content(status_type::bad_request, make_error(err));
      return;
    }

    rest_response<user_resp_data> data{};
    data.success = true;
    data.message = "注册成功";
    data.data =
        user_resp_data{id, info.username, info.email, bool(user.is_verifyed)};

    std::string json;
    iguana::to_json(data, json);

    resp.set_status_and_content(status_type::ok, std::move(json));
  }
};
} // namespace purecpp
#pragma once

#include <string>
#include <unordered_set>
#include <mutex>
#include <any>
#include <string_view>
#include <cinatra/coro_http_request.hpp>
#include <cinatra/coro_http_response.hpp>
#include "entity.hpp"
#include "jwt_token.hpp"

namespace purecpp {

// 前向声明（已经在jwt_token.hpp中定义）
class token_blacklist;

class user_logout_t {
public:
    void handle_logout(cinatra::coro_http_request& req, cinatra::coro_http_response& resp) {
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

} // namespace purecpp
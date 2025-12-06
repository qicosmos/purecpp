#pragma once

#include "entity.hpp"
#include "user_aspects.hpp"
#include "jwt_token.hpp"

#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace cinatra;

namespace purecpp {
    // 生成随机token的函数
    inline std::string generate_reset_token() {
        // 使用当前时间和随机数生成token
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto value = now_ms.time_since_epoch().count();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 999999);
        
        std::stringstream ss;
        ss << std::hex << value << std::hex << dis(gen);
        return ss.str();
    }
    
    // 模拟发送邮件的函数
    inline void send_reset_email(const std::string& email, const std::string& token) {
        // 在实际项目中，这里应该使用SMTP服务发送真实邮件
        // 这里只是模拟发送邮件的过程
        std::string reset_link = "http://localhost:3389/html/reset-password.html?token=" + token;
        std::cout << "[模拟发送邮件] 收件人: " << email << std::endl;
        std::cout << "[模拟发送邮件] 主题: 重置密码" << std::endl;
        std::cout << "[模拟发送邮件] 内容: 请点击以下链接重置密码: " << reset_link << std::endl;
        std::cout << "[模拟发送邮件] 链接有效期: 1小时" << std::endl;
    }
    
    class user_forgot_password_t {
    public:
        // 处理忘记密码请求
        void handle_forgot_password(coro_http_request& req, coro_http_response& resp) {
            std::locale::global(std::locale("zh_CN.UTF-8"));
            forgot_password_info info = std::any_cast<forgot_password_info>(req.get_user_data());
            
            // 查询数据库
            auto conn = connection_pool<dbng<mysql>>::instance().get();
            conn_guard guard(conn);
            
            // 查找用户
            auto users = conn->query_s<users_t>("email = ?", info.email);
            if (users.empty()) {
                // 用户不存在，但为了安全，不告诉用户邮箱是否存在
                rest_response<std::string_view> data{true, "如果邮箱存在，重置链接已发送"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::ok, std::move(json));
                return;
            }
            
            users_t user = users[0];
            
            // 生成重置token
            std::string token = generate_reset_token();
            
            // 设置token有效期为1小时
            uint64_t now = get_timestamp_milliseconds();
            uint64_t expires_at = now + 3600000; // 1小时 = 3600000毫秒
            
            // 保存token到数据库
            password_reset_tokens_t reset_token{
                .id = 0,
                .user_id = user.id,
                .token = token,
                .created_at = now,
                .expires_at = expires_at
            };
            
            // 删除该用户之前的所有重置token
            std::string delete_sql = "DELETE FROM password_reset_tokens WHERE user_id = " + std::to_string(user.id);
            conn->execute(delete_sql);
            
            // 插入新的token
            uint64_t insert_id = conn->get_insert_id_after_insert(reset_token);
            if (insert_id == 0) {
                auto err = conn->get_last_error();
                std::cout << err << "\n";
                rest_response<std::string_view> data{false, "生成重置链接失败，请稍后重试"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::internal_server_error, std::move(json));
                return;
            }
            
            // 发送重置邮件
            send_reset_email(info.email, token);
            
            // 返回成功响应
            rest_response<std::string_view> data{true, "如果邮箱存在，重置链接已发送"};
            std::string json;
            iguana::to_json(data, json);
            resp.set_status_and_content(status_type::ok, std::move(json));
        }
        
        // 处理密码重置请求
        void handle_reset_password(coro_http_request& req, coro_http_response& resp) {
            std::locale::global(std::locale("zh_CN.UTF-8"));
            reset_password_info info = std::any_cast<reset_password_info>(req.get_user_data());
            
            // 查询数据库
            auto conn = connection_pool<dbng<mysql>>::instance().get();
            conn_guard guard(conn);
            
            // 查找token
            auto tokens = conn->query_s<password_reset_tokens_t>("token = ?", info.token);
            if (tokens.empty()) {
                // token不存在
                rest_response<std::string_view> data{false, "重置密码链接无效或已过期"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::bad_request, std::move(json));
                return;
            }
            
            password_reset_tokens_t reset_token = tokens[0];
            
            // 检查token是否过期
            uint64_t now = get_timestamp_milliseconds();
            if (now > reset_token.expires_at) {
                // token已过期
                rest_response<std::string_view> data{false, "重置密码链接已过期"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::bad_request, std::move(json));
                return;
            }
            
            // 查找用户
            auto users = conn->query_s<users_t>("id = ?", reset_token.user_id);
            if (users.empty()) {
                // 用户不存在
                rest_response<std::string_view> data{false, "用户不存在"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::bad_request, std::move(json));
                return;
            }
            
            users_t user = users[0];
            
            // 更新用户密码
            user.pwd_hash = info.new_password;
            bool update_success = conn->update<users_t>(user, "id = ?", user.id);
            if (!update_success) {
                auto err = conn->get_last_error();
                std::cout << err << "\n";
                rest_response<std::string_view> data{false, "重置密码失败，请稍后重试"};
                std::string json;
                iguana::to_json(data, json);
                resp.set_status_and_content(status_type::internal_server_error, std::move(json));
                return;
            }
            
            // 删除已使用的token
            std::string delete_sql = "DELETE FROM password_reset_tokens WHERE id = " + std::to_string(reset_token.id);
            conn->execute(delete_sql);
            
            // 返回成功响应
            rest_response<std::string_view> data{true, "密码重置成功"};
            std::string json;
            iguana::to_json(data, json);
            resp.set_status_and_content(status_type::ok, std::move(json));
        }
    };
} // namespace purecpp
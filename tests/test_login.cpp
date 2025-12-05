#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

// 包含必要的头文件
#include "../entity.hpp"
#include "../user_aspects.hpp"
#include "../user_login.hpp"

#include <iguana/json_writer.hpp>
#include <iguana/json_reader.hpp>
#include <cinatra/coro_http_client.hpp>

using namespace purecpp;
using namespace cinatra;
using namespace iguana;

// 全局测试数据
std::string g_username;
std::string g_email;
std::string g_password = "Password123";

// 注册测试用户
void register_test_user() {
    coro_http_client client{};
    std::string url{"http://127.0.0.1:3389/api/v1/register"};
    register_info info;

    // 生成唯一的用户名和邮箱
    g_username = "testuser_" + std::to_string(get_timestamp_milliseconds());
    g_email = g_username + "@example.com";
    
    info.username = g_username;
    info.email = g_email;
    info.password = g_password;
    info.cpp_answer = "const";  // 第一个问题的正确答案
    info.question_index = 0;
    
    string_stream ss;
    to_json(info, ss);
    
    auto resp = client.post(url, ss.c_str(), req_content_type::json);
    std::cout << "Register response: " << resp.resp_body << std::endl;
    
    // 验证注册成功
    CHECK(resp.resp_body.find("\"success\":true") != std::string::npos);
}

// 测试用户登录功能
TEST_CASE("User Login API Tests") {
    // 在所有测试之前注册一个测试用户
    register_test_user();
    
    SUBCASE("Login with correct username and password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_username;
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with username response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含成功标志
        CHECK(resp.resp_body.find("\"success\":true") != std::string::npos);
        // 验证响应中包含token
        CHECK(resp.resp_body.find("\"token\"") != std::string::npos);
        // 验证响应中包含用户信息
        CHECK(resp.resp_body.find("\"username\"") != std::string::npos);
        CHECK(resp.resp_body.find("\"email\"") != std::string::npos);
    }
    
    SUBCASE("Login with correct email and password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_email;  // 使用邮箱作为用户名
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with email response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含成功标志
        CHECK(resp.resp_body.find("\"success\":true") != std::string::npos);
        // 验证响应中包含token
        CHECK(resp.resp_body.find("\"token\"") != std::string::npos);
    }
    
    SUBCASE("Login with incorrect password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_username;
        info.password = "wrongPassword123";
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with wrong password response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("用户名或密码错误") != std::string::npos);
    }
    
    SUBCASE("Login with non-existent username") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = "nonexistent_user_123456";
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with non-existent username response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("用户名或密码错误") != std::string::npos);
    }
    
    SUBCASE("Login with non-existent email") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = "nonexistent_email_123456@example.com";
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with non-existent email response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("用户名或密码错误") != std::string::npos);
    }
    
    SUBCASE("Login with empty request body") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        
        // 发送空的请求体
        auto resp = client.post(url, "", req_content_type::json);
        std::cout << "Login with empty body response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("login info is empty") != std::string::npos);
    }
    
    SUBCASE("Login with invalid JSON format") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        
        // 发送格式错误的JSON
        std::string invalid_json = "{\"username\":\"testuser\",\"password\":\"password123\"";
        auto resp = client.post(url, invalid_json, req_content_type::json);
        std::cout << "Login with invalid JSON response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("login info is not a required json") != std::string::npos);
    }
}

// 主测试用例
TEST_CASE("User Login API Comprehensive Tests") {
    // 首先注册测试用户
    register_test_user();
    
    // 运行所有登录相关测试
    
    SUBCASE("Login with correct username and password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_username;
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with username response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含成功标志
        CHECK(resp.resp_body.find("\"success\":true") != std::string::npos);
        CHECK(resp.resp_body.find("\"username\":\"" + g_username + "\"") != std::string::npos);
        CHECK(resp.resp_body.find("\"email\":\"" + g_email + "\"") != std::string::npos);
        CHECK(resp.resp_body.find("\"token\":") != std::string::npos);
    }
    
    SUBCASE("Login with correct email and password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_email;
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with email response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含成功标志
        CHECK(resp.resp_body.find("\"success\":true") != std::string::npos);
        // 验证响应中包含token
        CHECK(resp.resp_body.find("\"token\"") != std::string::npos);
    }
    
    SUBCASE("Login with incorrect password") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = g_username;
        info.password = "wrongPassword123";
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with wrong password response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("用户名或密码错误") != std::string::npos);
    }
    
    SUBCASE("Login with non-existent username") {
        coro_http_client client{};
        std::string url{"http://127.0.0.1:3389/api/v1/login"};
        login_info info;
        
        info.username = "nonexistent_user_123456";
        info.password = g_password;
        
        string_stream ss;
        to_json(info, ss);
        
        auto resp = client.post(url, ss.c_str(), req_content_type::json);
        std::cout << "Login with non-existent username response: " << resp.resp_body << std::endl;
        
        // 验证响应中包含失败标志
        CHECK(resp.resp_body.find("\"success\":false") != std::string::npos);
        // 验证错误信息
        CHECK(resp.resp_body.find("用户名或密码错误") != std::string::npos);
    }
}

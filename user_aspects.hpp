#pragma once
#include <any>
#include <chrono>
#include <cinatra.hpp>
#include <iguana/json_reader.hpp>
#include <iguana/json_writer.hpp>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <system_error>

using namespace cinatra;
using namespace iguana;

#include <optional>
#include <regex>
#include <string>
#include <vector>
#include "user_dto.hpp"
#include "error_info.hpp"

namespace purecpp {

// 获取当前时间戳（毫秒）
inline uint64_t get_timestamp_milliseconds() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

inline const std::vector<std::string_view> cpp_questions{
    "C++中声明指向int的常量指针, 语法是____ int* "
    "p。(请把空白部分的代码补充完整)",
    "sizeof(uint64_t)的返回值是?",
    "请输入C++中共享的智能指针。std::____ (请把空白部分的代码补充完整)",
    "请输入C++中独占的智能指针。std::____ (请把空白部分的代码补充完整)",
    "auto foo(){return new int(1);} void "
    "call_foo(){foo();} 这个call_foo函数有什么问题? ",
    "std::string str; str.reserve(100); 这个str的长度是多少?"};

inline const std::vector<std::string_view> cpp_answers{
    "const", "8", "shared_ptr", "unique_ptr", "内存泄漏", "0"};

// 生成错误响应的辅助函数
inline std::string make_error(std::string_view err_msg, int code = 400) {
  purecpp::rest_response<std::string_view> data{false, std::string(err_msg)};
  data.code = code;
  std::string json;
  iguana::to_json(data, json);
  return json;
}

// 生成包含错误列表的错误响应
inline std::string make_error(const std::vector<std::string>& errors, std::string_view err_msg = "请求参数错误", int code = 400) {
  purecpp::rest_response<std::string_view> data{false, std::string(err_msg)};
  data.code = code;
  data.errors = errors;
  std::string json; 
  iguana::to_json(data, json);
  return json;
}

struct check_register_input {
  bool before(coro_http_request &req, coro_http_response &res) {
    auto body = req.get_body();
    if (body.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_REGISTER_INFO_EMPTY));
      return false;
    }

    register_info info{};
    std::error_code ec;
    iguana::from_json(info, body, ec);
    if (ec) {
      res.set_status_and_content(
          status_type::bad_request,
          make_error(PURECPP_ERROR_REGISTER_JSON_INVALID));
      return false;
    }

    req.set_user_data(info);
    return true;
  }
};

struct check_cpp_answer {
  bool before(coro_http_request &req, coro_http_response &res) {
    register_info info = std::any_cast<register_info>(req.get_user_data());
    bool r = cpp_answers[info.question_index] == info.cpp_answer;

    if (!r) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_CPP_ANSWER_WRONG));
      return false;
    }
    return true;
  }
};

struct check_user_name {
  bool before(coro_http_request &req, coro_http_response &res) {
    register_info info = std::any_cast<register_info>(req.get_user_data());
    if (info.username.empty() || info.username.size() > 64) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_USERNAME_LENGTH));
      return false;
    }

    const std::regex username_regex("^[a-zA-Z0-9_-]+$");

    bool r = std::regex_match(std::string(info.username), username_regex);
    if (!r) {
      res.set_status_and_content(
          status_type::bad_request,
          make_error(PURECPP_ERROR_USERNAME_CHARACTER));
      return false;
    }
    return true;
  }
};

struct check_email {
  bool before(coro_http_request &req, coro_http_response &res) {
    register_info info = std::any_cast<register_info>(req.get_user_data());
    if (info.email.empty() || info.email.size() > 254) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_EMAIL_FORMAT));
      return false;
    }

    const std::regex email_regex(
        R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    bool r = std::regex_match(std::string{info.email}, email_regex);

    if (!r) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_EMAIL_FORMAT));
      return false;
    }
    return true;
  }
};

struct check_password {
  bool before(coro_http_request &req, coro_http_response &res) {
    register_info info = std::any_cast<register_info>(req.get_user_data());
    if (info.password.size() < 6 || info.password.size() > 20) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_LENGTH));
      return false;
    }

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;

    // 2. 字符类型要求 (至少包含大小写字母、数字和特殊字符各一个)
    for (char c : info.password) {
      if (std::isupper(static_cast<unsigned char>(c))) {
        has_upper = true;
      }
      else if (std::islower(static_cast<unsigned char>(c))) {
        has_lower = true;
      }
      else if (std::isdigit(static_cast<unsigned char>(c))) {
        has_digit = true;
      }
    }
    bool r = has_upper && has_lower && has_digit;
    if (!r) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_COMPLEXITY));
      return false;
    }
    return true;
  }
};

// 登录相关的验证结构体
struct check_login_input {
  bool before(coro_http_request &req, coro_http_response &res) {
    auto body = req.get_body();
    if (body.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_LOGIN_INFO_EMPTY));
      return false;
    }

    login_info info{};
    std::error_code ec;
    iguana::from_json(info, body, ec);
    if (ec) {
      res.set_status_and_content(
          status_type::bad_request,
          make_error(PURECPP_ERROR_LOGIN_JSON_INVALID));
      return false;
    }
    // 校验用户名、密码不能为空
    if (info.username.empty() || info.password.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_LOGIN_CREDENTIALS_EMPTY));
      return false;
    }

    req.set_user_data(info);
    return true;
  }
};

// 修改密码相关的验证结构体
struct check_change_password_input {
  bool before(coro_http_request &req, coro_http_response &res) {
    auto body = req.get_body();
    if (body.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_CHANGE_PASSWORD_EMPTY));
      return false;
    }

    change_password_info info{};
    std::error_code ec;
    iguana::from_json(info, body, ec);
    if (ec) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_CHANGE_PASSWORD_JSON_INVALID));
      return false;
    }

    // 校验用户ID、旧密码、新密码不能为空
    if (info.user_id == 0 || info.old_password.empty() ||
        info.new_password.empty()) {
      res.set_status_and_content(
          status_type::bad_request,
          make_error(PURECPP_ERROR_CHANGE_PASSWORD_REQUIRED_FIELDS));
      return false;
    }

    req.set_user_data(info);
    return true;
  }
};

struct check_new_password {
  bool before(coro_http_request &req, coro_http_response &res) {
    change_password_info info =
        std::any_cast<change_password_info>(req.get_user_data());

    // 检查新密码长度
    if (info.new_password.size() < 6 || info.new_password.size() > 20) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_LENGTH));
      return false;
    }

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;

    // 检查新密码字符类型要求
    for (char c : info.new_password) {
      if (std::isupper(static_cast<unsigned char>(c))) {
        has_upper = true;
      }
      else if (std::islower(static_cast<unsigned char>(c))) {
        has_lower = true;
      }
      else if (std::isdigit(static_cast<unsigned char>(c))) {
        has_digit = true;
      }
    }

    // 至少包含大小写字母和数字
    bool valid = has_upper && has_lower && has_digit;
    if (!valid) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_COMPLEXITY));
      return false;
    }

    // 新密码不能与旧密码相同
    if (info.new_password == info.old_password) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_NEW_SAME_AS_OLD));
      return false;
    }

    return true;
  }
};

// 忘记密码相关的验证结构体
struct check_forgot_password_input {
  bool before(coro_http_request &req, coro_http_response &res) {
    auto body = req.get_body();
    if (body.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_EMAIL_EMPTY));
      return false;
    }

    forgot_password_info info{};
    std::error_code ec;
    iguana::from_json(info, body, ec);
    if (ec) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_FORGOT_PASSWORD_JSON_INVALID));
      return false;
    }

    // 校验邮箱不能为空
    if (info.email.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_EMAIL_EMPTY));
      return false;
    }

    req.set_user_data(info);
    return true;
  }
};

// 重置密码相关的验证结构体
struct check_reset_password_input {
  bool before(coro_http_request &req, coro_http_response &res) {
    auto body = req.get_body();
    if (body.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_RESET_PASSWORD_EMPTY));
      return false;
    }

    reset_password_info info{};
    std::error_code ec;
    iguana::from_json(info, body, ec);
    if (ec) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_RESET_PASSWORD_JSON_INVALID));
      return false;
    }

    // 校验token和新密码不能为空
    if (info.token.empty() || info.new_password.empty()) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_RESET_PASSWORD_REQUIRED_FIELDS));
      return false;
    }

    req.set_user_data(info);
    return true;
  }
};

// 重置密码时的密码验证
struct check_reset_password {
  bool before(coro_http_request &req, coro_http_response &res) {
    reset_password_info info =
        std::any_cast<reset_password_info>(req.get_user_data());

    // 检查新密码长度
    if (info.new_password.size() < 6 || info.new_password.size() > 20) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_LENGTH));
      return false;
    }

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;

    // 检查新密码字符类型要求
    for (char c : info.new_password) {
      if (std::isupper(static_cast<unsigned char>(c))) {
        has_upper = true;
      }
      else if (std::islower(static_cast<unsigned char>(c))) {
        has_lower = true;
      }
      else if (std::isdigit(static_cast<unsigned char>(c))) {
        has_digit = true;
      }
    }

    // 至少包含大小写字母和数字
    bool valid = has_upper && has_lower && has_digit;
    if (!valid) {
      res.set_status_and_content(status_type::bad_request,
                                 make_error(PURECPP_ERROR_PASSWORD_COMPLEXITY));
      return false;
    }

    return true;
  }
};

// 日志切面工具
struct log_request_response {
  // 日志数据结构体
  struct LogData {
    std::chrono::system_clock::time_point start_time;
  };

  // 在请求处理前记录请求信息
  bool before(coro_http_request &req, coro_http_response &res) {
    // 记录请求开始时间
    auto start_time = std::chrono::system_clock::now();

    // 使用std::any存储日志数据
    req.set_user_data(LogData{start_time});

    // 记录请求信息
    std::ostringstream log_stream;

    // 格式化时间戳
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch() % std::chrono::seconds(1));

    std::tm now_tm = *std::localtime(&now_c);

    log_stream << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "."
               << std::setfill('0') << std::setw(3) << now_ms.count() << "] "
               << "[REQUEST] " << req.get_method() << " " << req.full_url()
               << " " << "User-Agent: " << req.get_header_value("User-Agent")
               << std::endl;

    // 记录请求头
    log_stream << "[REQUEST HEADERS]:" << std::endl;
    auto headers = req.get_headers();
    for (auto &header : headers) {
      log_stream << "  " << header.name << ": " << header.value << std::endl;
    }

    // 记录请求体
    auto body = req.get_body();
    if (!body.empty()) {
      log_stream << "[REQUEST BODY]: "
                 << (body.size() > 1000
                         ? std::string(body.substr(0, 1000)) + "..."
                         : std::string(body))
                 << std::endl;
    }

    // 输出日志
    std::cout << log_stream.str() << std::flush;

    return true;  // 继续处理请求
  }

  // 在请求处理后记录响应信息
  bool after(coro_http_request &req, coro_http_response &res) {
    // 获取请求开始时间
    auto user_data = req.get_user_data();
    if (!user_data.has_value()) {
      return true;  // 如果没有日志数据，直接返回
    }

    auto *log_data = std::any_cast<LogData>(&user_data);
    if (!log_data) {
      return true;  // 如果类型转换失败，直接返回
    }

    auto end_time = std::chrono::system_clock::now();

    // 计算处理时间
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - log_data->start_time)
                        .count();

    // 记录响应信息
    std::ostringstream log_stream;

    // 格式化时间戳
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch() % std::chrono::seconds(1));

    std::tm now_tm = *std::localtime(&now_c);

    log_stream << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "."
               << std::setfill('0') << std::setw(3) << now_ms.count() << "] "
               << "[RESPONSE] " << req.get_method() << " " << req.full_url()
               << " " << "Status: " << static_cast<int>(res.status()) << " "
               << "Duration: " << duration << "ms" << std::endl;

    // 记录响应体（只记录前1000个字符以避免日志过长）
    auto body = res.content();
    if (!body.empty()) {
      log_stream << "[RESPONSE BODY]: "
                 << (body.size() > 1000
                         ? std::string(body.substr(0, 1000)) + "..."
                         : std::string(body))
                 << std::endl;
    }

    log_stream << "----------------------------------------" << std::endl;

    // 输出日志
    std::cout << log_stream.str() << std::flush;

    return true;  // 继续处理后续操作
  }
};

}  // namespace purecpp
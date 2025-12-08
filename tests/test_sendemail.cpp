#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../send_email.hpp"
#include "doctest/doctest.h"

// 主函数测试
TEST_CASE("User Login API Tests") {
  // 配置参数（需替换为实际信息）
  std::string smtp_host = "smtp.qq.com";
  int smtp_port = 465;  // QQ邮箱SMTPS端口465，STARTTLS端口587
  bool is_smtps = true;
  std::string username = "raohj1987@qq.com";  // 发件人邮箱
  std::string password = "yeeubaecvdcbfihe";  // 邮箱授权码（非密码）
  std::string from = username;
  std::string to = "raohj1987@163.com";  // 收件人邮箱
  std::string subject = "Test Email (SMTP Protocol)";
  std::string body =
      "Hello, this is a test email sent via manual SMTP implementation in "
      "C++!\n";

  // 发送邮件
  bool result = purecpp::send_email(smtp_host, smtp_port, is_smtps, username,
                                    password, from, to, subject, body);
  CHECK(result == true);
}
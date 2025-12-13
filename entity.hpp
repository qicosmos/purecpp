#pragma once
#include <cinttypes>
#include <string>

#include <ormpp/connection_pool.hpp>
#include <ormpp/dbng.hpp>
#include <ormpp/mysql.hpp>
using namespace ormpp;

namespace purecpp {
// database config
struct db_config {
  std::string db_ip;
  int db_port;
  std::string db_name;
  std::string db_user_name;
  std::string db_pwd;

  int db_conn_num;
  int db_conn_timeout; // seconds
};

struct users_t {
  uint64_t id;
  std::array<char, 21> user_name; // unique, not null
  std::array<char, 254> email;    // unique, not null
  std::string_view pwd_hash;      // not null
  std::string status;             // 在线状态Online, Offline, Away
  int is_verifyed;                // 邮箱是否已验证
  uint64_t created_at;
  uint64_t last_active_at; // 最后活跃时间
};

inline constexpr std::string_view get_alias_struct_name(users_t *) {
  return "users"; // 表名默认结构体名字(users_t), 这里可以修改表名
}

// 文章相关的表
struct articles_t {
  uint64_t article_id = 0;
  int tag_id; // 外键
  std::string title;
  std::string abstraction; // 摘要
  std::string content;
  std::array<char, 8> slug; // 随机数，用于生成url的后缀
  uint64_t author_id;       // 外键
  uint64_t created_at;
  uint64_t updated_at;
  uint32_t views_count;
  uint32_t comments_count;
  uint64_t reviewer_id;       // 审核人id 外键
  std::string review_comment; // 审核意见
  int featured_weight;        // 置顶，精华
  uint64_t review_date;       // 审核完成时间
  std::string
      review_status; // pending_review (待审核), rejected (已拒绝), accepted
  std::string status; // 状态：published, draft, archived
  bool is_deleted;
};
inline constexpr std::string_view get_alias_struct_name(articles_t *) {
  return "articles";
}

struct article_comments {
  uint64_t comment_id;
  uint64_t article_id; // 外键
  uint64_t user_id;    // 外键
  std::string content;
  uint64_t created_at;
  uint64_t updated_at;
  uint64_t parent_id; // 指向父评论
};
inline constexpr std::string_view get_alias_struct_name(article_comments *) {
  return "article_comments";
}

struct tags_t {
  int tag_id;
  std::array<char, 50> name;
};
inline constexpr std::string_view get_alias_struct_name(tags_t *) {
  return "tags";
}

template <typename T> struct rest_response {
  bool success = true;
  std::string message;
  std::optional<std::vector<std::string>> errors;
  std::optional<T> data;
  std::string timestamp;
  int code = 200;
};
} // namespace purecpp
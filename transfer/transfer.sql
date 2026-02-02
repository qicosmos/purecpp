-- 迁移用户数据
truncate table purecpp.users;
insert into purecpp.users (id, user_name, email, pwd_hash, status, is_verifyed, created_at, last_active_at, title, role,
                           experience, level, bio, avatar, location, skills, login_attempts, last_failed_login)
SELECT t1.id,
       t1.user_login,
       t1.user_email,
       SHA2(t1.user_pass, 256),
       'Offline',
       1,
       case
           when t1.user_registered <> '' then
               UNIX_TIMESTAMP(STR_TO_DATE(t1.user_registered, '%Y-%m-%d %H:%i:%s')) * 1000
           else UNIX_TIMESTAMP(STR_TO_DATE('2019-01-01 00:00:01', '%Y-%m-%d %H:%i:%s')) *
                1000 end,
       0,
       0,
       case when t1.user_role = 6 then 'superadmin' when t1.user_role = 3 then 'admin' else 'user' end,
       0,
       0,
       '',
       '/images/avatar.png',
       '',
       '',
       0,
       0
from purecpp_old.pp_user t1;

-- 迁移文章的tags
truncate table purecpp.tags;
insert into purecpp.tags (tag_id, name)
select t1.term_id + 100, t1.name
from purecpp_old.pp_terms t1
where t1.term_id in (select distinct(t2.category) from purecpp_old.pp_posts t2);
insert into purecpp.tags (tag_id, name)
values (100, '默认分类');

-- 迁移文章内容
truncate table purecpp.articles;
insert into purecpp.articles (articles.article_id, tag_ids, title, abstraction, content, slug, author_id, created_at,
                              updated_at, views_count, comments_count, reviewer_id, review_comment, featured_weight,
                              review_date, status, is_deleted)
SELECT t1.id,
       case when t1.category = '' then 200 else t1.category + 100 end,
       t1.post_title,
       t1.content_abstract,
       t1.post_content,
       (SELECT LOWER(SUBSTRING(REPLACE(UUID(), '-', ''), 1, 8)) AS slug),
       t1.post_author,
       case
           when t1.post_date <> '' then
               UNIX_TIMESTAMP(CONVERT_TZ(STR_TO_DATE(t1.post_date, '%Y-%m-%d %H:%i:%s'), 'UTC', 'UTC')) * 1000
           else UNIX_TIMESTAMP(CONVERT_TZ(STR_TO_DATE('2019-01-01 00:00:01', '%Y-%m-%d %H:%i:%s'), 'UTC', 'UTC')) *
                1000 end,
       case
           when t1.post_modified <> '' then
               UNIX_TIMESTAMP(STR_TO_DATE(t1.post_modified, '%Y-%m-%d %H:%i:%s')) * 1000
           else UNIX_TIMESTAMP(STR_TO_DATE('2019-01-01 00:00:01', '%Y-%m-%d %H:%i:%s')) *
                1000 end,
       1000,
       t1.comment_count,
       0,
       '',
       0,
       0,
       case
           when t1.post_status = 'inherit' then 'draft'
           when t1.post_status = 'publish' then 'published'
           when t1.post_status = 'pending' then 'pending_review'
           else 'draft' end,
       case when t1.post_status = 'trash' then 1 else 0 end
from purecpp_old.pp_posts t1;

-- 迁移文章评论
truncate table purecpp.article_comments;
insert into purecpp.article_comments (comment_id, article_id, user_id, content, parent_comment_id, comment_status,
                                      created_at, updated_at)
select t1.id,
       t1.post_id,
       t1.user_id,
       t1.comment_content,
       t1.comment_parant,
       case when t1.comment_status = 'trash' then 0 else 1 end,
       case
           when t1.comment_date <> '' then
               UNIX_TIMESTAMP(STR_TO_DATE(t1.comment_date, '%Y-%m-%d %H:%i:%s')) * 1000
           else UNIX_TIMESTAMP(STR_TO_DATE('2019-01-01 00:00:01', '%Y-%m-%d %H:%i:%s')) *
                1000 end,
       case
           when t1.comment_date <> '' then
               UNIX_TIMESTAMP(STR_TO_DATE(t1.comment_date, '%Y-%m-%d %H:%i:%s')) * 1000
           else UNIX_TIMESTAMP(STR_TO_DATE('2019-01-01 00:00:01', '%Y-%m-%d %H:%i:%s')) *
                1000 end
from purecpp_old.pp_comment t1;
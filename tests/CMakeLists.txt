# tests/CMakeLists.txt

# 启用测试
enable_testing()

# 添加登录测试
add_executable(test_login test_login.cpp)
target_link_libraries(test_login ormpp)
add_test(NAME test_login COMMAND test_login)

# 添加JWT token测试
add_executable(test_jwt_token test_jwt_token.cpp)
target_link_libraries(test_jwt_token ormpp)
add_test(NAME test_jwt_token COMMAND test_jwt_token)

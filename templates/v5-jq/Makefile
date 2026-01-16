# 定义 JSON 风格的配置字符串
define PROJECT_CONFIG
{
  "name": "MyProject",
  "version": "1.0.0",
  "targets": {
    "app": {
      "sources": ["src/main.cpp", "src/utils.cpp"],
      "libs": ["-lm", "-lpthread"],
      "flags": ["-O2", "-std=c++17"]
    },
    "test": {
      "sources": ["tests/test.cpp", "src/utils.cpp"],
      "libs": ["-lgtest", "-lgtest_main"],
      "flags": ["-g", "-std=c++17"]
    }
  }
}
endef

# 简单的提取函数（需要外部工具）
get_target_sources = $(shell echo '$(PROJECT_CONFIG)' | jq -r '.targets.$1.sources[]')
get_target_libs = $(shell echo '$(PROJECT_CONFIG)' | jq -r '.targets.$1.libs[]')

# 使用示例（需要安装 jq）
app_sources := $(call get_target_sources,app)
app_libs := $(call get_target_libs,app)

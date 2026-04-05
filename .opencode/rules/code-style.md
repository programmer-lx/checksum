# 代码风格规则

## 花括号位置规则

- 函数、if/else、for/while 等语句的花括号必须放在下一行
- 示例：
```cpp
void function() 
{
    // 代码
}

if (condition) 
{
    // 代码
}

for (int i = 0; i < count; ++i) 
{
    // 代码
}
```

## 命名约定
- 类名：CamelCase
- 函数名：snake_case
- 变量名：snake_case
- 宏：UPPER_SNAKE_CASE
- 常量: CamelCase

## 注释要求
- 公共接口必须有Doxygen风格注释
- 复杂算法必须有详细注释
- 关键决策必须有注释说明

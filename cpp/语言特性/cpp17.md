# std::variant
空状态类型: std::monostate

检查是否持有某种类型: std::holds_alternative

从variant提取值: std::get<T> 如果当前类型不是T则抛出异常,std::bad_variant_access

安全访问variant方式： std::visit


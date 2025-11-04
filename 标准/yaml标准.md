# yaml中的几种node类型

Null	YAML::NodeType::Null	表示空值，如 key: 或 key: null
Scalar	YAML::NodeType::Scalar	单个值，如字符串、数字、布尔等
Sequence	YAML::NodeType::Sequence	序列（数组/列表），用 - 表示
Map	YAML::NodeType::Map	键值对映射（字典/哈希表）
Undefined	YAML::NodeType::Undefined	无效或未初始化的节点
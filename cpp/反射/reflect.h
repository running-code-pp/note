#ifndef SIMPLE_REFLECTION_REFLECTION_HPP_
#define SIMPLE_REFLECTION_REFLECTION_HPP_

#include <string>
#include <type_traits>
#include <sstream>
#include <format>
#include <vector>

namespace reflection
{
    // 类型描述符基类
    class TypeDescriptor
    {
    public:
        TypeDescriptor(const char *name, size_t size) : _name(name), _size(size) {}
        virtual ~TypeDescriptor() = default;
        virtual const char *name() const { return _name; }
        virtual size_t size() const { return _size; }
        virtual std::string dump(const void *obj) const = 0;

    protected:
        const char *_name;
        size_t _size;
    };

    // 判断是否是支持的基本类型
    template <typename T>
    struct is_basic_supported
        : std::integral_constant<bool,
                                 (std::is_fundamental_v<T> && !std::is_void_v<T>) ||
                                     std::is_same_v<T, std::string>>
    {
    };

    // 基本类型描述符
    template <typename T, typename std::enable_if<is_basic_supported<T>::value, int>::type = 0>
    class BasicTypeDescriptor : public TypeDescriptor
    {
    public:
        BasicTypeDescriptor() : TypeDescriptor(typeid(T).name(), sizeof(T)) {}

        virtual std::string dump(const void *obj) const override
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return std::format("{}(\"{}\")", _name, *(const T *)obj);
            }
            else
            {
                return std::format("{}({})", _name, *(const T *)obj);
            }
        }
    };

    // 成员描述符
    struct MemberDescriptor
    {
        const char *name;
        size_t offset;
        TypeDescriptor *type;

        MemberDescriptor(const char *n, size_t o, TypeDescriptor *t)
            : name(n), offset(o), type(t)
        {
        }
    };

    // 结构体描述符
    class StructDescriptor : public TypeDescriptor
    {
    public:
        std::vector<MemberDescriptor> members;

    public:
        StructDescriptor(const char *name, size_t size)
            : TypeDescriptor(name, size)
        {
        }

        // 添加成员描述符的方法
        void addMember(const char *name, size_t offset, TypeDescriptor *type)
        {
            members.emplace_back(name, offset, type);
        }

        const std::vector<MemberDescriptor> &getMembers() const { return members; }

        virtual std::string dump(const void *obj) const override
        {
            std::ostringstream oss;
            oss << _name << " {\n";
            for (const auto &member : members)
            {
                oss << "  " << member.name << " = ";
                const char *memberAddr = reinterpret_cast<const char *>(obj) + member.offset;
                oss << member.type->dump(memberAddr);
                oss << "\n";
            }
            oss << "}";
            return oss.str();
        }
    };

    // 细节实现
    namespace details
    {
        template <typename T>
        TypeDescriptor *getBasicDescriptor()
        {
            static BasicTypeDescriptor<T> descriptor;
            return &descriptor;
        }
    } // namespace details

    // 获取类型描述符
    template <typename T>
    TypeDescriptor *getDescriptor()
    {
        if constexpr (requires { T::getReflection; })
        {
            return &T::getReflection();
        }
        else if constexpr (is_basic_supported<T>::value)
        {
            return details::getBasicDescriptor<T>();
        }
        else
        {
            static_assert(is_basic_supported<T>::value,
                          "This type does not support reflection");
            return nullptr;
        }
    }

} // namespace reflection

// 在类内部声明反射描述符和辅助函数
#define ENABLE_REFLECT()                                                                                           \
    static reflection::StructDescriptor &_getReflectionImpl(                                                       \
        const char *className, size_t classSize)                                                                   \
    {                                                                                                              \
        static reflection::StructDescriptor desc(className, classSize);                                            \
        return desc;                                                                                               \
    }                                                                                                              \
    static reflection::StructDescriptor &getReflection()                                                           \
    {                                                                                                              \
        return _getReflectionImpl("", 0);                                                                          \
    }                                                                                                              \
    template <typename Class, typename MemberType>                                                                 \
    static void _registerMember(const char *className, size_t classSize,                                           \
                                const char *name, MemberType Class::*ptr)                                          \
    {                                                                                                              \
        size_t offset = reinterpret_cast<size_t>(                                                                  \
            &(reinterpret_cast<Class const volatile *>(0)->*ptr));                                                 \
        _getReflectionImpl(className, classSize).addMember(name, offset, reflection::getDescriptor<MemberType>()); \
    }

// 定义成员变量并注册到反射系统
#define REFLECTABLE(class_name, member_type, member_name) \
    member_type member_name;                              \
    inline static const bool _reflectionInit_##member_name = [] {            \
        class_name::_registerMember<class_name, member_type>(                \
            #class_name, sizeof(class_name), #member_name, &class_name::member_name); \
        return true; }();

#endif // SIMPLE_REFLECTION_REFLECTION_HPP_
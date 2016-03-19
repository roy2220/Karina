#pragma once


#include <cassert>
#include <utility>
#include <new>


namespace Karina {

class String;
class Array;
class Dictionary;
class Closure;


class Value final
{
public:
    template<class... Args>
    inline static Value MakeString(Args... args);
    template<class... Args>
    inline static Value MakeArray(Args... args);
    template<class... Args>
    inline static Value MakeDictionary(Args... args);
    template<class... Args>
    inline static Value MakeClosure(Args... args);

    inline explicit Value();
    inline explicit Value(bool);
    inline explicit Value(unsigned long);
    inline explicit Value(double);
    inline explicit Value(Value *);
    inline Value(const Value &);
    inline Value(Value &&);
    inline ~Value();

    template<class T>
    inline void operator=(T &&);

    inline Value *tryDereference();

    inline bool isNull() const;
    inline bool isBoolean() const;
    inline bool isInteger() const;
    inline bool isFloatingPoint() const;
    inline bool isString() const;
    inline bool isArray() const;
    inline bool isDictionary() const;
    inline bool isClosure() const;

    inline bool *getBoolean();
    inline unsigned long *getInteger();
    inline double *getFloatingPoint();
    inline String *getString();
    inline Array *getArray();
    inline Dictionary *getDictionary();
    inline Closure *getClosure();

private:
    enum class Type
    {
        Null = 0,
        Boolean,
        Integer,
        FloatingPoint,
        String,
        Array,
        Dictionary,
        Closure,
        Reference,
    };

    Type type_;

    union {
        Value *reference_;
        bool boolean_;
        unsigned long integer_;
        double floatingPoint_;
        String *string_;
        Array *array_;
        Dictionary *dictionary_;
        Closure *closure_;
    };

    inline explicit Value(String *) noexcept;
    inline explicit Value(Array *) noexcept;
    inline explicit Value(Dictionary *) noexcept;
    inline explicit Value(Closure *) noexcept;
};


class ValueData
{
    ValueData(const ValueData &) = delete;
    void operator=(const ValueData &) = delete;

public:
    inline ValueData *copy();
    inline void destroy();

private:
    int copyCount_;

    inline explicit ValueData();
    inline virtual ~ValueData() = default;
};


class String final: public ValueData
{
    String(const String &) = delete;
    void operator=(const String &) = delete;

public:
    inline ~String() override;
};


class Array final: public ValueData
{
    Array(const Array &) = delete;
    void operator=(const Array &) = delete;

public:
    inline ~Array() override;
};


class Dictionary final: public ValueData
{
    Dictionary(const Dictionary &) = delete;
    void operator=(const Dictionary &) = delete;

public:
    inline ~Dictionary() override;
};


class Closure final: public ValueData
{
    Closure(const Closure &) = delete;
    void operator=(const Closure &) = delete;

public:
    inline ~Closure() override;
};


#define VALUE_MAKER(valueType)                             \
template<class... Args>                                    \
Value                                                      \
Value::Make##valueType(Args... args)                       \
{                                                          \
    return Value(new String(std::forward<Args>(args)...)); \
}

VALUE_MAKER(String)
VALUE_MAKER(Array)
VALUE_MAKER(Dictionary)
VALUE_MAKER(Closure)

#undef VALUE_MAKER


Value::Value()
  : type_(Type::Null)
{
}


#define VALUE_CONSTRUCTOR1(valueType, simpleValueDataT, simpleValueData) \
    Value::Value(simpleValueDataT x)                                     \
      : type_(Type::valueType),                                          \
        simpleValueData(x)                                               \
    {                                                                    \
    }

VALUE_CONSTRUCTOR1(Boolean, bool, boolean_)
VALUE_CONSTRUCTOR1(Integer, unsigned long, integer_)
VALUE_CONSTRUCTOR1(FloatingPoint, double, floatingPoint_)
VALUE_CONSTRUCTOR1(Reference, Value *, reference_)

#undef VALUE_CONSTRUCTOR1


#define VALUE_CONSTRUCTOR2(valueType, valueDataT, valueData) \
    Value::Value(valueDataT x) noexcept                      \
      : type_(Type::valueType),                              \
        valueData(x)                                         \
    {                                                        \
    }

VALUE_CONSTRUCTOR2(String, String *, string_)
VALUE_CONSTRUCTOR2(Array, Array *, array_)
VALUE_CONSTRUCTOR2(Dictionary, Dictionary *, dictionary_)
VALUE_CONSTRUCTOR2(Closure, Closure *, closure_)

#undef VALUE_CONSTRUCTOR2


Value::Value(const Value &other)
{
    switch (other.type_) {
    case Type::Null:
        type_ = Type::Null;
        break;

    case Type::Boolean:
        type_ = Type::Boolean;
        boolean_ = other.boolean_;
        break;

    case Type::Integer:
        type_ = Type::Integer;
        integer_ = other.integer_;
        break;

    case Type::FloatingPoint:
        type_ = Type::FloatingPoint;
        floatingPoint_ = other.floatingPoint_;
        break;

    case Type::String:
        type_ = Type::String;
        string_ = static_cast<String *>(other.string_->copy());
        break;

    case Type::Array:
        type_ = Type::Array;
        array_ = static_cast<Array *>(other.array_->copy());
        break;

    case Type::Dictionary:
        type_ = Type::Dictionary;
        dictionary_ = static_cast<Dictionary *>(other.dictionary_->copy());
        break;

    case Type::Closure:
        type_ = Type::Closure;
        closure_ = static_cast<Closure *>(other.closure_->copy());
        break;

    case Type::Reference:
        assert(false);
    }
}


Value::Value(Value &&other)
{
    switch (other.type_) {
    case Type::Null:
        type_ = Type::Null;
        break;

    case Type::Boolean:
        type_ = Type::Boolean;
        boolean_ = other.boolean_;
        break;

    case Type::Integer:
        type_ = Type::Integer;
        integer_ = other.integer_;
        break;

    case Type::FloatingPoint:
        type_ = Type::FloatingPoint;
        floatingPoint_ = other.floatingPoint_;
        break;

    case Type::String:
        type_ = Type::String;
        string_ = other.string_;
        other.type_ = Type::Null;
        break;

    case Type::Array:
        type_ = Type::Array;
        array_ = other.array_;
        other.type_ = Type::Null;
        break;

    case Type::Dictionary:
        type_ = Type::Dictionary;
        dictionary_ = other.dictionary_;
        other.type_ = Type::Null;
        break;

    case Type::Closure:
        type_ = Type::Closure;
        closure_ = other.closure_;
        other.type_ = Type::Null;
        break;

    case Type::Reference:
        assert(false);
    }
}


Value::~Value()
{
    switch (type_) {
    case Type::Null:
    case Type::Boolean:
    case Type::Integer:
    case Type::FloatingPoint:
    case Type::Reference:
        break;

    case Type::String:
        string_->destroy();
        break;

    case Type::Array:
        array_->destroy();
        break;

    case Type::Dictionary:
        dictionary_->destroy();
        break;

    case Type::Closure:
        closure_->destroy();
        break;
    }
}


template<class T>
void
Value::operator=(T &&other)
{
    assert(type_ != Type::Reference);
    assert(other.type_ != Type::Reference);

    if (this == &other) {
        return;
    } else {
        this->~Value();
        new (this) Value(std::forward(other));
        return;
    }
}


Value *
Value::tryDereference()
{
    if (type_ == Type::Reference) {
        assert(reference_->type_ != Type::Reference);
        return reference_;
    } else {
        return this;
    }
}


#define VALUE_TYPE_TESTER(valueType)      \
    bool                                  \
    Value::is##valueType() const          \
    {                                     \
        assert(type_ != Type::Reference); \
        return type_ == Type::valueType;  \
    }

VALUE_TYPE_TESTER(Null)
VALUE_TYPE_TESTER(Boolean)
VALUE_TYPE_TESTER(Integer)
VALUE_TYPE_TESTER(FloatingPoint)
VALUE_TYPE_TESTER(String)
VALUE_TYPE_TESTER(Array)
VALUE_TYPE_TESTER(Dictionary)
VALUE_TYPE_TESTER(Closure)

#undef VALUE_TYPE_TESTER


#define VALUE_DATA_GETTER1(valueType, simpleValueDataT, simpleValueData) \
    simpleValueDataT *                                                   \
    Value::get##valueType()                                              \
    {                                                                    \
        assert(type_ == Type::valueType);                                \
        return &simpleValueData;                                         \
    }

VALUE_DATA_GETTER1(Boolean, bool, boolean_)
VALUE_DATA_GETTER1(Integer, unsigned long, integer_)
VALUE_DATA_GETTER1(FloatingPoint, double, floatingPoint_)

#undef VALUE_DATA_GETTER1


#define VALUE_DATA_GETTER2(valueType, valueDataT, valueData) \
    valueDataT                                               \
    Value::get##valueType()                                  \
    {                                                        \
        assert(type_ == Type::valueType);                    \
        return valueData;                                    \
    }

VALUE_DATA_GETTER2(String, String *, string_)
VALUE_DATA_GETTER2(Array, Array *, array_)
VALUE_DATA_GETTER2(Dictionary, Dictionary *, dictionary_)
VALUE_DATA_GETTER2(Closure, Closure *, closure_)

#undef VALUE_DATA_GETTER2


ValueData::ValueData()
  : copyCount_(0)
{
}


ValueData *
ValueData::copy()
{
    ++copyCount_;
    return this;
}


void
ValueData::destroy()
{
    if (copyCount_ == 0) {
        delete this;
        return;
    } else {
        --copyCount_;
        return;
    }
}

} // namespace Karina

#include <variant>

template <typename T, typename E>
class Result
{
private:
    std::variant<T, E> data;

public:
    // Constructors
    Result(const T &value) : data(value) {}
    Result(const E &error) : data(error) {}

    // Check if result contains a value
    bool isSuccess() const { return std::holds_alternative<T>(data); }

    // Access the value (will throw if contains error)
    T &value() { return std::get<T>(data); }
    const T &value() const { return std::get<T>(data); }

    // Access the error (will throw if contains value)
    E &error() { return std::get<E>(data); }
    const E &error() const { return std::get<E>(data); }

    // Safe accessors with default values
    T valueOr(T defaultValue) const
    {
        return isSuccess() ? value() : defaultValue;
    }
};
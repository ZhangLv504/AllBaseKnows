#include <iostream>
#include <cstdint>
#include <type_traits>

enum class PacketType : std::uint8_t {
    Login = 1,
    Data = 2,
    Logout = 10
};

const char* packetTypeToString(PacketType type)
{
    switch (type)
    {
    case PacketType::Login:
        return "Login";
    case PacketType::Data:
        return "Data";
    case PacketType::Logout:
        return "Logout";
    default:
        return "Unknown";
    }
}

bool isValidPacketType(std::uint8_t raw)
{
    switch (raw)
    {
        case static_cast<std::uint8_t>(PacketType::Login):
        case static_cast<std::uint8_t>(PacketType::Data):
        case static_cast<std::uint8_t>(PacketType::Logout):
            return true;
        default:
            return false;
    }
}

int main()
{
    // 创建 PacketType::Data
    PacketType packet = PacketType::Data;
    // 输出 sizeof(PacketType)
    std::cout << sizeof(PacketType) << std::endl;
    // 使用 std::underlying_type_t 取得底层值并输出
    using PacketValue = std::underlying_type_t<PacketType>;
    auto raw = static_cast<PacketValue>(packet);
    std::cout << static_cast<int>(raw) << std::endl;
    // 模拟收到合法值 10，校验后转换并输出 Logout
    if (isValidPacketType(10))
    {
        std::cout << packetTypeToString(static_cast<PacketType>(10)) << std::endl;
    }
    // 模拟收到非法值 99，拒绝转换并输出错误信息
    if (!isValidPacketType(99))
    {
        std::cout << "接收到非法值" << std::endl;
    }
    // 添加 static_assert，确认底层类型是 std::uint8_t
    static_assert(std::is_same_v<std::underlying_type_t<PacketType>, std::uint8_t>, "Underlying type must be uint8_t");
    return 0;
}
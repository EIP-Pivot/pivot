#pragma once

#include <compare>
#include <cstdint>
#include <functional>

namespace pivot::graphics::trs
{

class Ticket
{
public:
    constexpr Ticket() noexcept = default;
    constexpr Ticket(std::uint64_t id, std::uint64_t version = 0) noexcept: id(id), version(version) {}
    constexpr Ticket(const Ticket &other): id(other.getId()), version(other.getVersion()){};

    constexpr const std::uint64_t getId() const noexcept { return id; }
    constexpr const std::uint64_t getVersion() const noexcept { return version; }
    constexpr bool valid() const noexcept { return id != 0; }

    constexpr Ticket newVersion() const noexcept { return Ticket(id, version + 1); }

    constexpr std::partial_ordering operator<=>(const Ticket &other) const
    {
        if (id != other.getId()) return std::partial_ordering::unordered;
        return version <=> other.getVersion();
    }
    constexpr Ticket &operator=(const Ticket &other)
    {
        *this = Ticket(other.getId(), other.getVersion());
        return *this;
    }

    constexpr operator bool() const noexcept { return valid(); }

public:
    static Ticket newTicket();

private:
    const std::uint64_t id = 0;
    const std::uint64_t version = 0;
};

}    // namespace pivot::graphics::trs

namespace std
{

template <>
struct hash<pivot::graphics::trs::Ticket> {
    constexpr std::size_t operator()(const pivot::graphics::trs::Ticket &ticket) const
    {
        return ticket.getId() ^ ticket.getVersion();
    }
};

}    // namespace std

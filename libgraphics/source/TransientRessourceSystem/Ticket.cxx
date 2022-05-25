#include "pivot/graphics/TransientRessourceSystem/Ticket.hxx"

#include <random>

namespace pivot::graphics::trs
{

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<std::uint64_t> s_UniformDistribution;

Ticket Ticket::newTicket() { return Ticket(s_UniformDistribution(eng)); }

}    // namespace pivot::graphics::trs

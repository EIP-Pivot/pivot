#pragma once

#include <string>
#include <variant>

namespace pivot::ecs
{

/** \brief Stores the provenance of a ressource
 *
 * The ressource can either be builtin, or come from an external ressource
 * specified by name.
 */
class Provenance : private std::variant<std::monostate, std::string>
{
public:
    /// Creates a default builting provenance
    Provenance(): variant(std::monostate()) {}

    /// Creates a builtin provenance
    static Provenance builtin() { return Provenance(); }

    /// Creates an external provenance from the ressource name
    static Provenance externalRessource(const std::string_view ressource) { return Provenance(ressource); }

    /// Checks if provenance is builtin
    bool isBuiltin() const { return std::holds_alternative<std::monostate>(*this); }

    /// Checks if provenance is external
    bool isExternalRessource() const { return std::holds_alternative<std::string>(*this); }

    /// Retrieves the external ressource name
    const std::string &getExternalRessource() const { return std::get<std::string>(*this); }

private:
    explicit Provenance(const std::string_view ressource): variant(std::string(ressource)) {}
};
}    // namespace pivot::ecs

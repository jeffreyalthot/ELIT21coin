#include "elit21/readiness.hpp"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace elit21 {

std::string ReadinessReport::to_markdown() const {
    std::ostringstream os;
    os << "# Rapport de préparation ELIT21coin\n\n";
    os << "- Ready for development: " << (ready_for_development ? "oui" : "non") << "\n";
    os << "- Wallets enregistrés: " << wallets_registered << "\n";
    os << "- Taille mempool: " << mempool_size << "\n";
    os << "- Hauteur de chaîne: " << chain_height << "\n";
    os << "- Validation: " << (validation.valid ? "valide" : "invalide") << " ("
       << validation.failure_reason << ")\n\n";

    os << "## Gates\n";
    for (const auto& gate : gates) {
        os << "- [" << (gate.passed ? 'x' : ' ') << "] " << gate.name;
        if (!gate.detail.empty()) {
            os << " — " << gate.detail;
        }
        os << '\n';
    }

    os << "\n## Balances\n";
    for (const auto& [address, balance] : balances) {
        os << "- " << address << ": " << balance << "\n";
    }

    return os.str();
}

ReadinessReport evaluate_readiness(const Blockchain& chain,
                                   std::size_t mempool_size,
                                   const std::map<std::string, std::uint64_t>& balances,
                                   std::size_t min_wallets,
                                   std::size_t max_mempool_threshold,
                                   std::size_t min_chain_height,
                                   const std::vector<std::string>& required_codecs) {
    ReadinessReport report;
    report.wallets_registered = balances.size();
    report.mempool_size = mempool_size;
    report.chain_height = chain.chain().size();
    report.validation = chain.validate_with_metrics();
    report.balances = balances;

    const auto add_gate = [&report](const std::string& name, bool passed, const std::string& detail) {
        report.gates.push_back(ReadinessGate{name, passed, detail});
    };

    add_gate("Blockchain validée",
             report.validation.valid,
             report.validation.valid ? "Chaîne cohérente" : report.validation.failure_reason);

    add_gate("Nombre minimal de wallets",
             report.wallets_registered >= min_wallets,
             "attendu >= " + std::to_string(min_wallets) + ", observé=" + std::to_string(report.wallets_registered));

    add_gate("Mempool sous contrôle",
             report.mempool_size <= max_mempool_threshold,
             "seuil=" + std::to_string(max_mempool_threshold) + ", observé=" + std::to_string(report.mempool_size));

    add_gate("Hauteur de chaîne minimale",
             report.chain_height >= min_chain_height,
             "attendu >= " + std::to_string(min_chain_height) + ", observé=" + std::to_string(report.chain_height));

    bool codecs_ok = true;
    for (const auto& codec : required_codecs) {
        if (!is_supported_codec(codec)) {
            codecs_ok = false;
            break;
        }
    }
    add_gate("Codecs requis disponibles", codecs_ok,
             codecs_ok ? "RLE/RAW disponibles" : "Un codec requis est manquant");

    const auto total_balance = std::accumulate(
        balances.begin(), balances.end(), std::uint64_t{0}, [](std::uint64_t acc, const auto& pair) {
            return acc + pair.second;
        });
    add_gate("Liquidité non nulle", total_balance > 0, "somme des soldes=" + std::to_string(total_balance));

    report.ready_for_development = std::all_of(report.gates.begin(), report.gates.end(), [](const auto& gate) {
        return gate.passed;
    });

    return report;
}

}  // namespace elit21

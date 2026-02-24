#include "elit21/transaction.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>

namespace elit21 {

std::string Transaction::id() const {
    const auto seed = from + "|" + to + "|" + std::to_string(amount) + "|" +
                      std::to_string(fee) + "|" + std::to_string(nonce) + "|" + memo;
    return std::to_string(std::hash<std::string>{}(seed));
}

std::string Transaction::serialize() const {
    std::ostringstream os;
    os << from.size() << '|' << from
       << '|' << to.size() << '|' << to
       << '|' << amount
       << '|' << fee
       << '|' << nonce
       << '|' << memo.size() << '|' << memo;
    return os.str();
}

Transaction Transaction::deserialize(const std::string& raw) {
    std::size_t cursor = 0;
    auto consume_token = [&](const char* field_name) {
        const auto sep = raw.find('|', cursor);
        if (sep == std::string::npos) {
            throw std::runtime_error(std::string("invalid transaction: ") + field_name);
        }
        const auto token = raw.substr(cursor, sep - cursor);
        cursor = sep + 1;
        return token;
    };

    auto consume_sized = [&](std::size_t size, const char* field_name) {
        if (raw.size() - cursor < size + 1) {
            throw std::runtime_error(std::string("invalid transaction: ") + field_name);
        }
        const auto out = raw.substr(cursor, size);
        cursor += size;
        if (raw[cursor] != '|') {
            throw std::runtime_error(std::string("invalid transaction: ") + field_name);
        }
        ++cursor;
        return out;
    };

    Transaction tx;

    const auto from_size = static_cast<std::size_t>(std::stoull(consume_token("from_size")));
    tx.from = consume_sized(from_size, "from");

    const auto to_size = static_cast<std::size_t>(std::stoull(consume_token("to_size")));
    tx.to = consume_sized(to_size, "to");

    tx.amount = static_cast<std::uint64_t>(std::stoull(consume_token("amount")));
    tx.fee = static_cast<std::uint64_t>(std::stoull(consume_token("fee")));
    tx.nonce = static_cast<std::uint64_t>(std::stoull(consume_token("nonce")));

    const auto memo_size = static_cast<std::size_t>(std::stoull(consume_token("memo_size")));
    if (raw.size() - cursor < memo_size) {
        throw std::runtime_error("invalid transaction: memo");
    }
    tx.memo = raw.substr(cursor, memo_size);

    if (!is_valid_transaction(tx)) {
        throw std::runtime_error("invalid transaction semantic");
    }
    return tx;
}

bool is_valid_transaction(const Transaction& tx) {
    if (tx.from.empty() || tx.to.empty()) {
        return false;
    }
    if (tx.amount == 0) {
        return false;
    }
    if (tx.from == tx.to) {
        return false;
    }
    return true;
}

}  // namespace elit21

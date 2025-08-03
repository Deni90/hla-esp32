#ifndef loom_info_h
#define loom_info_h

#include <inttypes.h>
#include <optional>
#include <string>

#include "cJSON.h"

namespace hla {
/**
 * @brief Enumeration representing loom states
 */
enum class LoomState { Init, Idle, Running, Paused };

constexpr const char* loomStateToString(LoomState ls) {
    switch (ls) {
    case LoomState::Init:
        return "init";
    case LoomState::Idle:
        return "idle";
    case LoomState::Running:
        return "running";
    case LoomState::Paused:
        return "paused";
    default:
        return "unknown";
    }
}

/**
 * @brief Represents a Wifi info class
 *
 */
struct LoomInfo {
    LoomInfo();

    /**
     * @brief Construct a new Loom Info object
     *
     * @param state state
     * @param liftplanName Name of the liftplan file
     * @param liftplanLength Length of the liftplan, number of steps
     * @param liftplanIndex Index of the active row
     */
    LoomInfo(const LoomState& state, const std::string& liftplanName,
             unsigned int liftplanLength, unsigned int liftplanIndex);

    /**
     * @brief Default destructor
     */
    ~LoomInfo() = default;

    /**
     * @brief Default copy constructor
     * @param other LoomInfo object
     */
    LoomInfo(const LoomInfo& other) = default;

    /**
     * @brief Default copy assignment constructor
     * @param other LoomInfo object
     */
    LoomInfo& operator=(const LoomInfo& other) = default;

    LoomState state;
    std::optional<std::string> liftplanName;
    std::optional<unsigned int> liftplanLength;
    std::optional<unsigned int> liftplanIndex;
};
}   // namespace hla
#endif   // loom_info_h
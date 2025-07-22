#include "loom_info.h"

using hla::LoomInfo;
using hla::LoomState;

LoomInfo::LoomInfo()
    : state(LoomState::idle), liftplanName(std::nullopt),
      liftplanIndex(std::nullopt) {}

LoomInfo::LoomInfo(const LoomState& state, const std::string& liftplanName,
                   unsigned int liftplanIndex)
    : state(state), liftplanName(liftplanName), liftplanIndex(liftplanIndex) {}
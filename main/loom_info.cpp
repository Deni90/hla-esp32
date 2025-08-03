#include "loom_info.h"

using hla::LoomInfo;
using hla::LoomState;

LoomInfo::LoomInfo()
    : state(LoomState::Init), liftplanName(std::nullopt),
      liftplanLength(std::nullopt), liftplanIndex(std::nullopt) {}

LoomInfo::LoomInfo(const LoomState& state, const std::string& liftplanName,
                   unsigned int liftplanLength, unsigned int liftplanIndex)
    : state(state), liftplanName(liftplanName), liftplanLength(liftplanLength),
      liftplanIndex(liftplanIndex) {}
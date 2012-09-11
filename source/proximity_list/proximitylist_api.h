#pragma once
#include "type/type.pb.h"

namespace navitia {

// Forward declare
namespace type {
struct Data;
enum class Type_e;
struct GeographicalCoord;
}

namespace proximitylist {
pbnavitia::Response find(type::GeographicalCoord coord, double distance, const std::vector<type::Type_e> & filter, const type::Data & data);
}} // namespace navitia::proximitylist

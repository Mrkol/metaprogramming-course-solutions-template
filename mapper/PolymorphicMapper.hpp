#pragma once

#include <optional>

template
  < class From
  , auto target >
struct Mapping;

template
  < class Base
  , class Target
  , class... Mappings >
struct PolymorphicMapper;

template
  < class Base
  , class Target
  , template < class From, auto target> class Mapping
  , class From
  , auto target
  , class... RemainMappings >
// Consider next mapping<from, target> from the pack
struct PolymorphicMapper
  < Base
  , Target
  , Mapping<From, target>
  , RemainMappings... >
{
  static std::optional<Target> map(const Base& object) {
    static_assert(
      std::is_same_v<std::remove_const_t<decltype(target)>,
      Target>);
    // Check if it's not From or inheritance of the From typename and if it's not derived
    if (dynamic_cast<const From*>(&object) == nullptr || !std::derived_from<From, Base>) {
      // The check next mapping
      return PolymorphicMapper<Base, Target, RemainMappings...>::map(object);
    } else {
      // Else return target
      return target;
    }
  }
};

template
  < class Base
  , class Target >
struct PolymorphicMapper
  < Base
  , Target>
{
  // If we pass all arguments from the pack, then we unforetunately didn't reach the mapping
  // and we return nullopt
  static std::optional<Target> map(const Base& object __attribute__((unused))){
    return std::nullopt;
  }
};

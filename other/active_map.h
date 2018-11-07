#pragma once

#include <utility>
#include <memory>

namespace shri314::util {

template <template <class, class, class...> class underlying_map_t, class key_t, class value_t, class... more_args_t>
class active_map_adapter_t {
private:
   using entry_t = std::pair<value_t, std::weak_ptr<value_t>>;
   using map_t = underlying_map_t<key_t, entry_t, more_args_t...>;

public:
   template<class... args_t>
   constexpr active_map_adapter_t(args_t&&... args)
      : store(std::forward<args_t>(args)...)
   {
   }

   active_map_adapter_t(const active_map_adapter_t&) = delete;
   active_map_adapter_t(active_map_adapter_t&&) = delete;
   active_map_adapter_t& operator=(const active_map_adapter_t&) = delete;
   active_map_adapter_t& operator=(active_map_adapter_t&&) = delete;
   ~active_map_adapter_t() = default;

   struct[[nodiscard]] handle_t {
      friend class active_map_adapter_t;

      constexpr explicit operator bool() const noexcept {
         return valid();
      }

      constexpr value_t& value() noexcept {
         return *x;
      }

      constexpr const value_t& value() const noexcept {
         return *x;
      }

      handle_t(const handle_t&) = default;
      handle_t(handle_t&&) = default;
      handle_t& operator=(const handle_t&) = default;
      handle_t& operator=(handle_t&&) = default;
      ~handle_t() = default;

   private:
      constexpr bool valid() const noexcept {
         return x != nullptr;
      }

      constexpr explicit handle_t(std::shared_ptr<value_t> x) noexcept
         : x{x}
      {
      }

      constexpr explicit handle_t() noexcept
         : x{}
      {
      }

   private:
      std::shared_ptr<value_t> x;
   };


public:
   template <class compatible_key_t, class compatible_value_t>
   constexpr handle_t put(compatible_key_t&& key, compatible_value_t&& value) {
      auto& e = store[key];

      e.first = value;

      auto handle = e.second.lock();

      if (!handle) {
         e.second = handle = std::shared_ptr<value_t>(
               &e.first,
               [ pstore = &store, key ](auto) {
                  pstore->erase(key);
               }
            );
      }

      return handle_t{handle};
   }

   template <class compatible_key_t>
   constexpr handle_t get(compatible_key_t&& key) {
      auto i = store.find(key);
      if (i != store.end()) {
         auto handle = i->second.second.lock();
         return handle_t{handle};
      }

      return handle_t{};
   }

   template <class compatible_key_t>
   constexpr const handle_t get(compatible_key_t&& key) const {
      auto i = store.find(key);
      if (i != store.end()) {
         auto handle = i->second.second.lock();
         return handle_t{handle};
      }

      return handle_t{};
   }

   template <class F>
   constexpr void for_each(F&& fun) const {
      for (const auto& i : store) {
         fun(i.first, i.second.first);
      }
   }

   template <class F>
   constexpr void for_each(F&& fun) {
      for (auto& i : store) {
         fun(i.first, i.second.first);
      }
   }

   constexpr auto size() const noexcept {
      return store.size();
   }

private:
   map_t store;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <unordered_map>

namespace shri314::util {

template <class key_t, class value_t, class... more_args_t>
using active_map_t = active_map_adapter_t<std::map, key_t, value_t, more_args_t...>;

template <class key_t, class value_t, class... more_args_t>
using active_unordered_map_t = active_map_adapter_t<std::unordered_map, key_t, value_t, more_args_t...>;

}

///////////////////////////////////////////////////////////////////////////////////////////////////

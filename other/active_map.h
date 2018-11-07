#pragma once

#include <utility>
#include <memory>

namespace shri314::util {

template <template <class, class, class...> class underlying_map_t, class key_t, class value_t, class... more_args_t>
class active_map_adapter_t {
private:
   using entry_t = std::pair<value_t, std::weak_ptr<value_t>>;
   using map_t = underlying_map_t<key_t, entry_t, more_args_t...>;

   template<class value_ptr_t>
   struct[[nodiscard]] handle_impl_t {
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

      constexpr explicit handle_impl_t() noexcept
         : x{}
      {
      }

      inline ~handle_impl_t() = default;

      constexpr handle_impl_t(const handle_impl_t& rhs) noexcept = default;

      constexpr handle_impl_t(handle_impl_t&& rhs) noexcept = default;

      constexpr handle_impl_t& operator=(const handle_impl_t& rhs) noexcept = default;

      constexpr handle_impl_t& operator=(handle_impl_t&& rhs) noexcept = default;

   private:
      constexpr bool valid() const noexcept {
         return x != nullptr;
      }

      constexpr explicit handle_impl_t(value_ptr_t x) noexcept
         : x{x}
      {
      }

   private:
      value_ptr_t x;
   };

public:
   using handle_t = handle_impl_t<std::shared_ptr<value_t>>;
   using const_handle_t = handle_impl_t<std::shared_ptr<const value_t>>;

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

   template <class compatible_key_t, class compatible_value_t>
   constexpr handle_t put(compatible_key_t&& key, compatible_value_t&& value) {
      auto i = store.find(key);
      if (i != store.end()) {
         i->second.first = std::forward<compatible_value_t>(value);
      } else {
         i = store.emplace( std::forward<compatible_key_t>(key), entry_t{std::forward<compatible_value_t>(value), {}} ).first;
      }

      auto handle = i->second.second.lock();

      if (!handle) {
         i->second.second = handle = std::shared_ptr<value_t>(
               &i->second.first,
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
         return handle_t{i->second.second.lock()};
      } else {
         return handle_t{};
      }
   }

   template <class compatible_key_t>
   constexpr const_handle_t get(compatible_key_t&& key) const {
      auto i = store.find(key);
      if (i != store.end()) {
         return const_handle_t{i->second.second.lock()};
      } else {
         return const_handle_t{};
      }
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

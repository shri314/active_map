#pragma once

#include <utility>

namespace shri314::util {

template <template <class, class, class...> class underlying_map_t, class key_t, class value_t, class... more_args_t>
class active_map_adapter_t {
private:
   using entry_t = std::pair<value_t, size_t>;
   using map_t = underlying_map_t<key_t, entry_t, more_args_t...>;

public:
   struct[[nodiscard]] handle_t {
      friend class active_map_adapter_t;

      constexpr explicit operator bool() const noexcept {
         return valid();
      }

      constexpr value_t& operator*() noexcept {
         return entry_iter->second.first;
      }

      constexpr const value_t& operator*() const noexcept {
         return entry_iter->second.first;
      }

      constexpr value_t* operator->() noexcept {
         return &entry_iter->second.first;
      }

      constexpr const value_t* operator->() const noexcept {
         return &entry_iter->second.first;
      }

      constexpr explicit handle_t() noexcept
         : owner{}
      {
      }

      ~handle_t() noexcept {
         this->release();
      }

      constexpr handle_t(const handle_t& rhs) noexcept
         : owner{rhs.owner}
         , entry_iter{rhs.entry_iter}
      {
         this->acquire();
      }

      constexpr handle_t(handle_t&& rhs) noexcept
         : owner(rhs.owner)
         , entry_iter(std::move(rhs.entry_iter))
      {
         rhs.owner = nullptr;
      }

      constexpr handle_t& operator=(const handle_t& rhs) noexcept {

         if(&rhs != this) {
            this->release();

            owner = rhs.owner;
            entry_iter = rhs.entry_iter;

            this->acquire();
         }

         return *this;
      }

      constexpr handle_t& operator=(handle_t&& rhs) noexcept {
         this->release();

         owner = rhs.owner;
         entry_iter = rhs.entry_iter;

         rhs.owner = nullptr;

         return *this;
      }

   private:
      constexpr bool valid() const noexcept {
         return owner != nullptr && entry_iter != owner->end();
      }

      constexpr void acquire() noexcept {
         if (valid())
            ++(entry_iter->second.second);
      }

      constexpr void release() noexcept {
         if (valid())
            if (--(entry_iter->second.second) == 0)
               owner->erase(entry_iter);
      }

      constexpr explicit handle_t(map_t* owner, typename map_t::iterator entry_iter) noexcept
         : owner(owner)
         , entry_iter(entry_iter)
      {
         this->acquire();
      }

   private:
      map_t* owner;
      typename map_t::iterator entry_iter;
   };

   struct[[nodiscard]] const_handle_t {
      friend class active_map_adapter_t;

      constexpr explicit operator bool() const noexcept {
         return valid();
      }

      constexpr const value_t& operator*() noexcept {
         return entry_iter->second.first;
      }

      constexpr const value_t& operator*() const noexcept {
         return entry_iter->second.first;
      }

      constexpr const value_t* operator->() noexcept {
         return &entry_iter->second.first;
      }

      constexpr const value_t* operator->() const noexcept {
         return &entry_iter->second.first;
      }

      constexpr explicit const_handle_t() noexcept
         : owner{}
      {
      }

      ~const_handle_t() = default;

      const_handle_t(const const_handle_t& rhs) = delete;

      constexpr const_handle_t(const_handle_t&& rhs) noexcept
         : owner(rhs.owner)
         , entry_iter(std::move(rhs.entry_iter))
      {
         rhs.owner = nullptr;
      }

      const_handle_t& operator=(const const_handle_t& rhs) = delete;

      const_handle_t& operator=(const_handle_t&& rhs) = delete;

   private:
      constexpr bool valid() const noexcept {
         return owner != nullptr && entry_iter != owner->end();
      }

      constexpr explicit const_handle_t(const map_t* owner, typename map_t::const_iterator entry_iter) noexcept
         : owner(owner)
         , entry_iter(entry_iter)
      {
      }

   private:
      const map_t* owner;
      typename map_t::const_iterator entry_iter;
   };

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

      return handle_t{&store, i};
   }

   template <class compatible_key_t>
   constexpr handle_t get(compatible_key_t&& key) {
      return handle_t{&store, store.find(key)};
   }

   template <class compatible_key_t>
   constexpr const_handle_t get(compatible_key_t&& key) const {
      return const_handle_t{&store, store.find(key)};
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

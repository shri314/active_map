#include <utility>

namespace shri314::util {

template <template <class, class, class...> class underlying_map_t, class key_t, class value_t, class... more_args_t>
class active_map_adaptor {
private:
   using entry_t = std::pair<value_t, size_t>;
   using map_t = underlying_map_t<key_t, entry_t, more_args_t...>;

public:

   template<class... args_t>
   active_map_adaptor(args_t&&... args)
      : store(std::forward<args_t>(args)...)
   {
   }

   struct[[nodiscard]] handle_t {
      friend class active_map_adaptor;

      constexpr explicit operator bool() const noexcept {
         return valid();
      }

      constexpr value_t& value() noexcept {
         return entry_iter->second.first;
      }

      constexpr const value_t& value() const noexcept {
         return entry_iter->second.first;
      }

      ~handle_t() noexcept {
         if (valid())
            if (--(entry_iter->second.second) == 0)
               owner.erase(entry_iter);
      }

      constexpr handle_t(const handle_t& rhs) noexcept
         : owner{rhs.owner}
         , entry_iter{rhs.entry_iter}
      {
         if (valid())
            ++(entry_iter->second.second);
      }

      constexpr handle_t(handle_t&& rhs) noexcept
         : owner(rhs.owner)
         , entry_iter(std::move(rhs.entry_iter))
      {
         rhs.entry_iter = rhs.owner.end();
      }

      void operator=(handle_t&&) = delete;
      void operator=(handle_t&) = delete;

   private:
      constexpr bool valid() const noexcept {
         return entry_iter != owner.end();
      }

      constexpr explicit handle_t(map_t& owner, typename map_t::iterator entry_iter) noexcept : owner(owner), entry_iter(entry_iter) {
         if (valid())
            ++(entry_iter->second.second);
      }

   private:
      map_t& owner;
      typename map_t::iterator entry_iter;
   };


public:
   template <class compatible_key_t, class compatible_value_t>
   constexpr handle_t put(compatible_key_t&& key, compatible_value_t&& value) {
      auto i = store.find(key);
      if (i != store.end()) {
         i->second.first = std::forward<compatible_value_t>(value);
      } else {
         i = store.emplace(std::forward<compatible_key_t>(key), entry_t{std::forward<compatible_value_t>(value), 0}).first;
      }

      return handle_t{store, i};
   }

   template <class compatible_key_t>
   constexpr handle_t get(compatible_key_t&& key) {
      return handle_t{store, store.find(key)};
   }

   template <class compatible_key_t>
   constexpr const handle_t get(compatible_key_t&& key) const {
      return handle_t{store, store.find(key)};
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
using active_map_t = active_map_adaptor<std::map, key_t, value_t, more_args_t...>;

template <class key_t, class value_t, class... more_args_t>
using active_unordered_map_t = active_map_adaptor<std::unordered_map, key_t, value_t, more_args_t...>;

}

///////////////////////////////////////////////////////////////////////////////////////////////////

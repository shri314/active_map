#include "active_map.h"

#include <memory>
#include <cassert>

using namespace shri314::util;

int main() {
   using AM = active_map_t<int, int>;
   
   AM wcm;

   {
      std::unique_ptr<AM::handle_t> h2;

      {
         assert( wcm.size() == 0 );

         auto&& h1 = wcm.put(10, 100);

         wcm.put(10, 101);
         wcm.put(11, 110);

         assert( wcm.size() == 1 );

         h2.reset(new AM::handle_t(h1));
      }

      assert( wcm.size() == 1 );
      assert( h2 );
      assert( h2->value() == 101 );

      {
         auto ha = wcm.get(10);
         assert( ha );
         assert( ha.value() == 101 );
      }

      {
         auto ha = wcm.get(11);
         assert( !ha );
      }
   }

   {
      auto ha = wcm.get(10);
      assert( !ha );
   }

   assert( wcm.size() == 0 );

   {
      auto hx = wcm.put(12, 120);
      auto hy = wcm.put(13, 130);

      assert( wcm.size() == 2 );

      int s = 0;
      wcm.for_each([&s](auto& k, auto& v) {
         s += v++;
      });

      assert(s == 250);

      s = 0;
      wcm.for_each([&s](auto& k, auto& v) {
         s += v;
      });

      assert(s == 252);
      assert( wcm.size() == 2 );
   }

   assert( wcm.size() == 0 );
}

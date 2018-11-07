#include "active_map.h"

#include <memory>
#include <cassert>

using namespace shri314::util;

int main() {
   using AM = active_map_t<int, int>;
   
   AM test_object;

   {
      std::unique_ptr<AM::handle_t> h2;

      {
         assert( test_object.size() == 0 );

         auto&& h1 = test_object.put(10, 100);

         test_object.put(10, 101);
         test_object.put(11, 110);

         assert( test_object.size() == 1 );

         h2.reset(new AM::handle_t(h1));
      }

      assert( test_object.size() == 1 );
      assert( h2 );
      assert( h2->value() == 101 );

      {
         auto ha = test_object.get(10);
         assert( ha );
         assert( ha.value() == 101 );
      }

      {
         auto ha = test_object.get(11);
         assert( !ha );
      }
   }

   {
      auto ha = test_object.get(10);
      assert( !ha );
   }

   assert( test_object.size() == 0 );

   {
      auto hx = test_object.put(12, 120);
      auto hy = test_object.put(13, 130);

      assert( test_object.size() == 2 );

      int s = 0;
      test_object.for_each([&s](auto& k, auto& v) {
         s += v++;
      });

      assert(s == 250);

      s = 0;
      test_object.for_each([&s](auto& k, auto& v) {
         s += v;
      });

      assert(s == 252);
      assert( test_object.size() == 2 );
   }

   assert( test_object.size() == 0 );
}

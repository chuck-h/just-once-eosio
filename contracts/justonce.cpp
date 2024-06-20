#include "justonce.hpp"
#include <eosio/crypto.hpp>

//contractName:justonce

void justonce::reset() {
  require_auth(get_self());
  for(auto itr = events.begin(); itr!=events.end(); ) {  
    itr = events.erase(itr);
  }
}

void justonce::antirepeat( uint64_t nonce, uint64_t lifetime_sec,
     name scope, name ram_payer ) {
  require_auth(ram_payer);
  check( lifetime_sec >= min_lifetime && lifetime_sec <= max_lifetime,
         "invalid lifetime");
         
  // retire expired events
  // loop through event table starting from earliest expiration time
  // limit number of items removed to max_purges
  const uint32_t time_now = current_time_point().sec_since_epoch();
  auto expidx = events.get_index<"byexpiry"_n>();
  int i = max_purges;
  for (auto exp = expidx.cbegin(), end = expidx.cend();
      exp != end && exp->expiry < time_now && --i > 0 ;
      exp = expidx.erase(exp)) {}
  
  // emplace new event
  const struct {
    uint64_t n;
    uint64_t s;
  } data = {nonce, scope.value};
  checksum160 sha1hash = sha1((const char *)&data, sizeof(data));
  // event_id is first 8 bytes of sha1 hash
  uint64_t event_id = * (uint64_t*)&sha1hash;
  auto itr = events.find(event_id);
  check(itr == events.end(), "repeated event");
  
  events.emplace(ram_payer, [&](auto& s) {
    s.event_id = event_id;
    s.expiry = time_now + lifetime_sec;
  });
}
  

        


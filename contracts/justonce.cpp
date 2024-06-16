#include "justonce.hpp"

//contractName:justonce

void justonce::reset() {
  require_auth(get_self());
  for(auto itr = events.begin(); itr!=events.end(); ) {  
    itr = events.erase(itr);
  }
}

void justonce::newevent( name ram_payer, uint64_t eventid, uint64_t lifetime_sec ) {
  require_auth(ram_payer);
  check( lifetime_sec >= min_lifetime && lifetime_sec <= max_lifetime,
         "invalid lifetime");
         
  // retire expired events
  // loop through event table starting from earliest expiration time
  const uint32_t time_now = current_time_point().sec_since_epoch();
  auto expidx = events.get_index<"byexpiry"_n>();
  for (auto exp = expidx.cbegin(), end = expidx.cend();
      exp != end && exp->expiry < time_now;
      exp = expidx.erase(exp)) {}
  
  // emplace new event
  auto itr = events.find(eventid);
  if (itr == events.end() ) {
    events.emplace(ram_payer, [&](auto& s) {
      s.event_id = eventid;
      s.expiry = time_now + lifetime_sec;
    });
  } else {
    check(false, "repeated event");
  }
}
  

        


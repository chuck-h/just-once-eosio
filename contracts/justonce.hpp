#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

using namespace eosio;
using std::string;

   /**
    * The `justonce' contract provides protection against accidentally submitting the same transaction
    * twice. The expected usecase is in an online marketplace or Point-of-Sale situation where the
    * customer buys something by scanning a QR code containing a `transfer` action.
    *
    * The merchant computes a nonce (which could be an invoice number, or a hash of the transaction
    * details) and appends a `justonce` `antirepeat` action to the sales transaction. This action
    * places the event into a history table. The event_id in the table is a hash of the nonce and the
    * scope field.
    *
    * If the customer scans the same QR code a second time, the `antirepeat` action will fail with
    * a "repeated event" error.
    *
    * A repeated event is detected if the nonce and scope match a history table entry. The merchant
    * is expected to use their account name as scope, and merchant is responsible for ensuring that the
    * nonces are sufficiently unique within that scope.
    *
    * In order to limit the history table size, each event action submission includes a lifetime parameter;
    * events are purged from the table after that lifetime.
    **/

CONTRACT justonce : public contract {
    public:
        using contract::contract;
        justonce(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds),
              events(receiver, receiver.value)
              {}

        ACTION reset();

        ACTION antirepeat( uint64_t nonce, uint64_t lifetime_sec,
           name scope, name ram_payer );

    private:

        static const uint32_t min_lifetime = 1;
        static const uint32_t max_lifetime = 2*31536000; // 2 years
        static const int max_purges = 10; // to protect against execution timeout
        
        TABLE event_ids {
            uint64_t    event_id;
            uint32_t    expiry; // as eosio::time_point_sec.secs_since_epoch

            uint64_t primary_key() const {
                return event_id;
            }
            uint64_t by_expiry() const { return (uint64_t)expiry; }

        };

        typedef eosio::multi_index<"events"_n, event_ids, indexed_by
            < "byexpiry"_n,
              const_mem_fun<event_ids, uint64_t, &event_ids::by_expiry > >
            > event_table;
        event_table events;

};

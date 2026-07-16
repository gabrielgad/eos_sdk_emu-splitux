#ifndef EOS_LAN_CONNECT_INTERNAL_H
#define EOS_LAN_CONNECT_INTERNAL_H

#include "eos/eos_connect.h"
#include "eos/eos_connect_types.h"
#include "platform_internal.h"
#include <stdbool.h>

#define MAX_LOCAL_USERS 4
#define PRODUCT_USER_ID_LENGTH 32
#define MAX_LOGIN_NOTIFICATIONS 8
#define MAX_AUTH_EXPIRATION_NOTIFICATIONS 8

// Deferred external-account-mapping queries (see connect.c). Redpoint's friend
// pipeline calls QueryExternalAccountMappings then GetExternalAccountMapping and
// caches the friend's PUID from that ONE resolution. A LAN peer's Steam->PUID
// mapping only becomes known once its beacon arrives (~seconds after goldberg
// surfaces the Steam friend), so an instant-Success query races: the first
// resolve returns NULL and the friend is baked with a null PUID forever. We hold
// the query callback until every requested id resolves (or a timeout) so the
// first resolution succeeds. Redpoint tolerates the async latency (real EOS
// mapping queries hit a backend).
#define MAX_PENDING_MAPPING_QUERIES 32
#define PENDING_QUERY_MAX_IDS 16
#define PENDING_QUERY_ID_LEN 64
#define PENDING_QUERY_TIMEOUT_MS 8000

// Internal representation of a ProductUserId
typedef struct EOS_ProductUserIdDetails {
    uint32_t magic;          // 0x50554944 = "PUID"
    char id_string[PRODUCT_USER_ID_LENGTH + 1];  // 32 hex chars + null
} EOS_ProductUserIdDetails;

// Local user state
typedef struct {
    EOS_ProductUserIdDetails user_id;
    EOS_ELoginStatus status;
    bool in_use;
} LocalUser;

// Login status change notification handler
typedef struct {
    EOS_Connect_OnLoginStatusChangedCallback callback;
    void* client_data;
    EOS_NotificationId id;
    bool active;
} LoginStatusNotification;

// Auth expiration notification handler
typedef struct {
    EOS_Connect_OnAuthExpirationCallback callback;
    void* client_data;
    EOS_NotificationId id;
    bool active;
} AuthExpirationNotification;

// A QueryExternalAccountMappings call held until its ids resolve (or timeout).
typedef struct {
    bool active;
    EOS_Connect_OnQueryExternalAccountMappingsCallback callback;
    void* client_data;
    EOS_ProductUserId local_user_id;
    EOS_EExternalAccountType account_type;
    char ids[PENDING_QUERY_MAX_IDS][PENDING_QUERY_ID_LEN];
    int id_count;
    uint64_t deadline_ms;
} PendingMappingQuery;

// Connect state
typedef struct ConnectState {
    uint32_t magic;  // 0x434F4E4E = "CONN"

    // Back-reference to platform
    PlatformState* platform;

    // Instance identifier (for generating unique user IDs)
    char instance_id[17];  // 16 hex chars + null

    // Local logged-in users
    LocalUser users[MAX_LOCAL_USERS];
    int user_count;

    // Notification handlers
    LoginStatusNotification login_notifications[MAX_LOGIN_NOTIFICATIONS];
    int login_notification_count;
    AuthExpirationNotification auth_expiration_notifications[MAX_AUTH_EXPIRATION_NOTIFICATIONS];
    int auth_expiration_notification_count;
    EOS_NotificationId next_notification_id;

    // Deferred external-account-mapping queries, drained by connect_tick().
    PendingMappingQuery pending_queries[MAX_PENDING_MAPPING_QUERIES];
    int pending_query_count;

} ConnectState;

// Creation/destruction (called by platform)
ConnectState* connect_create(PlatformState* platform);
void connect_destroy(ConnectState* state);

// Per-tick pump: fires any deferred external-account-mapping query whose ids
// have resolved (peer beacons arrived) or whose deadline has passed.
void connect_tick(ConnectState* state);

// Internal helpers
EOS_ProductUserId connect_generate_user_id(ConnectState* state, int user_index);
bool connect_validate_user_id(EOS_ProductUserId id);
LocalUser* connect_find_user_by_id(ConnectState* state, EOS_ProductUserId id);
void connect_fire_login_status_notifications(ConnectState* state, EOS_ProductUserId user_id,
                                              EOS_ELoginStatus prev_status, EOS_ELoginStatus curr_status);

#endif // EOS_LAN_CONNECT_INTERNAL_H

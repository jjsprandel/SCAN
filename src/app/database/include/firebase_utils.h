#ifndef FIREBASE_UTILS_H
#define FIREBASE_UTILS_H

// Struct to store user information

typedef struct
{
    char active_user[5];
    char check_in_status[20];
    char first_name[32];
    char last_name[32];
    char location[64];
    char role[32];
    char passkey[64];
} UserInfo;

extern UserInfo user_info_instance;
extern UserInfo *user_info;

bool get_user_info(const char *user_id);
bool check_in_user(const char *user_id);
bool check_out_user(const char *user_id);

#endif /* FIREBASE_UTILS_H */

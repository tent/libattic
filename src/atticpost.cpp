#include "atticpost.h"

#include "crypto.h"
#include "constants.h"

AtticPost::AtticPost() {
    set_type(cnst::g_attic_cred_type);
}

AtticPost::~AtticPost() {}

void AtticPost::Serialize(Json::Value& root) {

    Json::Value cred;
    std::string salt, mk;
    crypto::Base64EncodeString(salt_, salt);
    crypto::Base64EncodeString(master_key_, mk);
    cred["salt"] = salt;
    cred["mk"] = mk;

    std::string rsalt, rmk;
    crypto::Base64EncodeString(recovery_salt_, rsalt);
    crypto::Base64EncodeString(recovery_master_key_, rmk);
    cred["rsalt"] = rsalt;
    cred["rmk"] = rmk;

    std::string qsalt, qmk, q1, q2, q3;
    crypto::Base64EncodeString(question_salt_, qsalt);
    crypto::Base64EncodeString(question_master_key_, qmk);
    crypto::Base64EncodeString(question_one_, q1);
    crypto::Base64EncodeString(question_two_, q2);
    crypto::Base64EncodeString(question_three_, q3);
    cred["qsalt"] = qsalt;
    cred["qmk"] = qmk;
    cred["q1"] = q1;
    cred["q2"] = q2;
    cred["q3"] = q3;

    set_content("cred", cred);

    

    Post::Serialize(root);
}

void AtticPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value cred;
    get_content("cred", cred);

    std::string salt = cred.get("salt", "").asString();
    std::string mk = cred.get("mk", "").asString();
    crypto::Base64DecodeString(salt, salt_);
    crypto::Base64DecodeString(mk, master_key_);

    std::string rsalt = cred.get("rsalt", "").asString();
    std::string rmk = cred.get("rmk", "").asString();
    crypto::Base64DecodeString(rsalt, recovery_salt_);
    crypto::Base64DecodeString(rmk, recovery_master_key_);

    std::string qsalt = cred.get("qsalt", "").asString();
    std::string qmk = cred.get("qmk", "").asString();
    std::string q1 = cred.get("q1", "").asString();
    std::string q2 = cred.get("q2", "").asString();
    std::string q3 = cred.get("q3", "").asString();

    crypto::Base64DecodeString(qsalt, question_salt_);
    crypto::Base64DecodeString(qmk, question_master_key_);
    crypto::Base64DecodeString(q1, question_one_);
    crypto::Base64DecodeString(q2, question_two_);
    crypto::Base64DecodeString(q3, question_three_);
}



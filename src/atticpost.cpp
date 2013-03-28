#include "atticpost.h"

#include "crypto.h"
#include "constants.h"

AtticPost::AtticPost() {
    set_type(cnst::g_attic_cred_type);
}

AtticPost::~AtticPost() {}

void AtticPost::Serialize(Json::Value& root) {
    std::string salt, mk, mk_iv;
    crypto::Base64EncodeString(salt_, salt);
    crypto::Base64EncodeString(master_key_, mk);
    crypto::Base64EncodeString(master_key_iv_, mk_iv);
    root["salt"] = salt;
    root["mk"] = mk;
    root["mk_iv"] = mk_iv;

    std::string rsalt, rmk, riv;
    crypto::Base64EncodeString(recovery_salt_, rsalt);
    crypto::Base64EncodeString(recovery_master_key_, rmk);
    crypto::Base64EncodeString(recovery_master_key_iv_, riv);
    root["rsalt"] = rsalt;
    root["rmk"] = rmk;
    root["riv"] = riv;

    std::string qsalt, qmk, qiv, q1, q2, q3;
    crypto::Base64EncodeString(question_salt_, qsalt);
    crypto::Base64EncodeString(question_master_key_, qmk);
    crypto::Base64EncodeString(question_master_key_iv_, qiv);
    crypto::Base64EncodeString(question_one_, q1);
    crypto::Base64EncodeString(question_two_, q2);
    crypto::Base64EncodeString(question_three_, q3);
    root["qsalt"] = qsalt;
    root["qmk"] = qmk;
    root["qiv"] = qiv;
    root["q1"] = q1;
    root["q2"] = q2;
    root["q3"] = q3;
}

void AtticPost::Deserialize(Json::Value& root) {
    std::string salt = root.get("salt", "").asString();
    std::string mk = root.get("mk", "").asString();
    std::string iv = root.get("mk_iv", "").asString();
    crypto::Base64DecodeString(salt, salt_);
    crypto::Base64DecodeString(mk, master_key_);
    crypto::Base64DecodeString(iv, master_key_iv_);

    std::string rsalt = root.get("rsalt", "").asString();
    std::string rmk = root.get("rmk", "").asString();
    std::string riv = root.get("riv", "").asString();
    crypto::Base64DecodeString(rsalt, recovery_salt_);
    crypto::Base64DecodeString(rmk, recovery_master_key_);
    crypto::Base64DecodeString(riv, recovery_master_key_iv_);

    std::string qsalt = root.get("qsalt", "").asString();
    std::string qmk = root.get("qmk", "").asString();
    std::string qiv = root.get("qiv", "").asString();
    std::string q1 = root.get("q1", "").asString();
    std::string q2 = root.get("q2", "").asString();
    std::string q3 = root.get("q3", "").asString();

    crypto::Base64DecodeString(qsalt, question_salt_);
    crypto::Base64DecodeString(qmk, question_master_key_);
    crypto::Base64DecodeString(qiv, question_master_key_iv_);
    crypto::Base64DecodeString(q1, question_one_);
    crypto::Base64DecodeString(q2, question_two_);
    crypto::Base64DecodeString(q3, question_three_);
}



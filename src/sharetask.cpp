#include "sharetask.h"

#include <iostream>

#include "posthandler.h"
#include "entity.h"
#include "clientutils.h"
#include "publickeypost.h"
#include "filehandler.h"
#include "sharedfilepost.h"

namespace attic { 

ShareTask::ShareTask(FileManager* fm, 
                     CredentialsManager* cm,
                     const AccessToken& at,
                     const Entity& entity,
                     const TaskContext& context)
                     :
                     TentTask(Task::SHARE,
                              fm,
                              cm,
                              at,
                              entity,
                              context) {
}

ShareTask::~ShareTask() {}

void ShareTask::RunTask() {
    std::cout<< " share task " << std::endl;

    std::string post_id, entity_url;
    context_.get_value("post_id", post_id);
    context_.get_value("entity_url", entity_url);

    // Retrieve public key
    std::string public_key;
    if(RetrievePublicKey(entity_url, public_key))
        CreateSharedFilePost(post_id, entity_url, public_key);

    SetFinishedState();
}

bool ShareTask::CreateSharedFilePost(const std::string& file_post_id, 
        const std::string& entity_url,
        const std::string& public_key) {
    bool ret = false;
    // Decrypt Credentials from file info
    Credentials decrypted;
    if(RetrieveDecryptedCredentials(file_post_id, decrypted)) {
        std::string private_key;
        credentials_manager()->GetPrivateKey(private_key);
        // Create Shared Post
        //      Encrypt credentials with public key
        std::string nonce, encrypted_key;
        if(crypto::EncryptStringWithPublicKey(decrypted.key(), 
                public_key,
                private_key,
                nonce,
                encrypted_key)) {
            //      Copy over chunks
            FileInfo fi;
            if(file_manager()->GetFileInfoByPostId(file_post_id, fi)) {
                SharedFilePost sfp;
                sfp.set_filename(fi.filename());
                sfp.set_encrypted_key(encrypted_key);
                sfp.set_iv(decrypted.iv());
                sfp.set_nonce(nonce);
                sfp.set_chunk_data(*(fi.GetChunkInfoList()));
                //      Mention entity
                sfp.MentionEntity(entity()->entity(), entity_url);
                // Make post
                PostHandler<SharedFilePost> ph(access_token()); 
                if(ph.Post(entity()->GetPreferredServer().posts_feed(),
                        NULL,
                        sfp) == ret::A_OK) {
                    SharedFilePost rp = ph.GetReturnPost();
                    // Insert into new table
                    ret = file_manager()->InsertSharedFile(rp.id(),
                            file_post_id,
                            entity_url);
                }
            }
        }
    }

    return ret;
}

bool ShareTask::RetrieveDecryptedCredentials(const std::string& post_id, Credentials& out) {
    bool ret = false;
    std::string mk;
    if(GetMasterKey(mk)) {
        FileHandler fh(file_manager());
        ret = fh.ExtractFileCredentials(post_id, mk, out);
    }
    return ret;
}

bool ShareTask::RetrievePublicKey(const std::string& entity_url, std::string& out) {
    bool ret = false;
    // Discover entity
    Entity entity;
    if(client::Discover(entity_url, NULL, entity) == ret::A_OK) {
        // query post_feed
        UrlParams params;                                                                    
        params.AddValue(std::string("types"), std::string(cnst::g_attic_publickey_type));   

        Response resp;
        netlib::HttpGet(entity.GetPreferredServer().posts_feed(), 
                        &params,
                        NULL,
                        resp);
        if(resp.code == 200) {
            Envelope env;
            jsn::DeserializeObject(&env, resp.body);

            if(env.posts()->size()) {
                PublicKeyPost pkp; 
                post::DeserializePostIntoObject(env.posts()->front(), &pkp);
                out = pkp.public_key();
                ret = true;
            }
        }
    }
    return ret;
}

} //namespace 



#include "filehandler.h"

#include "filesystem.h"
#include "crypto.h"
#include "credentials.h"
#include "utils.h"
#include "logutils.h"
#include "foldersem.h"

namespace attic { 

FileHandler::FileHandler(FileManager* fm) {
    file_manager_ = fm;
}

FileHandler::~FileHandler() {}

bool FileHandler::DoesFileExist(const std::string& filepath) {
    return file_manager_->DoesFileExist(filepath);
}
bool FileHandler::RetrieveFileInfo(const std::string& filepath, FileInfo& out) {
    return file_manager_->GetFileInfo(filepath, out);
}

bool FileHandler::RetrieveFileInfoById(const std::string& post_id, FileInfo& out) {
    return file_manager_->GetFileInfoByPostId(post_id, out);
}

// Note* there are 3 keys that file info objects are queried by, this only sets two of them,
// if the third is not set, a new file with the same path and folderid can be created for a 
// duplicate entry
bool FileHandler::CreateNewFile(const std::string& filepath, // full filepath
                                const std::string& master_key,
                                FileInfo& out) {
    if(master_key.empty()) {
        std::ostringstream err;
        err << " Invalid master key : "<< master_key << std::endl;
        log::LogString("0010-12851-", err.str());
        return false;
    }
    std::cout<<" create new file : " << filepath << std::endl;

    if(!file_manager_->DoesFileExist(filepath)) {
        std::string folderpath;
        if(fs::GetParentPath(filepath, folderpath)) {
            std::string folderid;
            utils::CheckUrlAndRemoveTrailingSlash(folderpath);
            if(file_manager_->GetFolderPostId(folderpath, folderid)) {
                std::string aliased, filename;
                utils::ExtractFileName(filepath, filename);
                file_manager_->GetAliasedPath(filepath, aliased);
                out.set_filename(filename);             // set filename
                out.set_filepath(aliased);              // set filepath
                out.set_folder_post_id(folderid);       // set folder id
                out.set_chunk_count(0);
                Credentials file_cred;
                while(file_cred.key_empty())
                    crypto::GenerateCredentials(file_cred);
                out.set_file_credentials(file_cred);    // set credentials

                // Generate plaintext mac for file
                std::string plaintext_hash;
                if(RollFileMac(filepath, plaintext_hash)) {
                    out.set_plaintext_hash(plaintext_hash);
                }
                else {
                    std::ostringstream err;
                    err << "Failed to generate plaintext mac for file : " << std::endl;
                    err << "\t " << filepath << std::endl;
                    log::LogString("19240=-124i8", err.str());
                }


                EncryptFileKey(out, master_key); // sets encryted file key on file info
                // verify key
                // set filesize
                out.set_file_size(utils::CheckFilesize(filepath));
                return file_manager_->InsertToManifest(&out);
            }
        }
    }
    return false;
}

bool FileHandler::GetCanonicalFilepath(const std::string& filepath, std::string& out) {
    return file_manager_->GetCanonicalPath(filepath, out);
}

bool FileHandler::UpdateFileInfo(FileInfo& fi) {
    return file_manager_->InsertToManifest(&fi);
}

bool FileHandler::UpdateFilepath(const std::string& post_id, const std::string& new_folder_post_id) {
    bool ret = false;
    FileInfo fi;
    if(file_manager_->GetFileInfoByPostId(post_id, fi)) {
        // Construct filepath for current local cache
        std::string folderpath, canonical_old;
        if(file_manager_->ConstructFolderpath(fi.folder_post_id(), folderpath)) {
            file_manager_->GetCanonicalPath(folderpath, canonical_old);
        }

        folderpath.clear();
        std::string canonical_new;
        if(file_manager_->ConstructFolderpath(new_folder_post_id, folderpath)) {
            file_manager_->GetCanonicalPath(folderpath, canonical_new);
        }

        std::string old_filepath, new_filepath;
        utils::AppendTrailingSlash(canonical_old);
        utils::AppendTrailingSlash(canonical_new);

        old_filepath = canonical_old + fi.filename();
        new_filepath = canonical_new + fi.filename(); 
        if(fs::CheckFilepathExists(old_filepath)){
            if(!fs::CheckFilepathExists(new_filepath)) {
                FolderSem fs;
                fs.AquireWrite(fi.folder_post_id());
                if(fs::RenamePath(old_filepath, new_filepath)) {
                    // Set post id
                    ret = file_manager_->SetFileFolderPostId(post_id, new_folder_post_id);
                }
                fs.ReleaseWrite(fi.folder_post_id());
            }
        }
    }
    return ret;
}

bool FileHandler::UpdateFilePostId(const std::string& filepath, const std::string& post_id) {
    return file_manager_->SetFilePostId(filepath, post_id);
}

bool FileHandler::UpdateChunkCount(const std::string& filepath, const std::string& count) {
    return file_manager_->SetFileChunkCount(filepath, count);
}

bool FileHandler::UpdateFileSize(const std::string& filepath, const std::string& size) {
    // TODO:: this
    std::cout<< " fh IMPLEMENT THIS " << std::endl;
    return false;
}

bool FileHandler::UpdateChunkMap(const std::string& filepath, FileInfo::ChunkMap& map) {
    return file_manager_->SetFileChunks(filepath, map);
}

bool FileHandler::UpdatePostVersion(const std::string& filepath, const std::string& version) {
    return file_manager_->SetFileVersion(filepath, version);
}

bool FileHandler::UpdateFolderEntry(FolderPost& fp) {
    return file_manager_->UpdateFolderEntry(fp.folder().foldername(), fp.id());
}

void FileHandler::PrepareCargo(FileInfo& fi, 
                               const std::string& master_key, 
                               std::string& cargo_out) {
    Cargo c;
    c.filename = fi.filename();
    c.filepath = fi.filepath();
    c.plaintext_hash = fi.plaintext_hash();

    std::string cargo_buffer;
    jsn::SerializeObject(&c, cargo_buffer);

    std::string file_key;
    DecryptFileKey(fi.encrypted_key(),
                   fi.file_credentials_iv(),
                   master_key,
                   file_key);

    // transient credentials
    Credentials t_cred;
    t_cred.set_key(file_key);
    t_cred.set_iv(fi.file_credentials_iv());

    std::string encrypted_cargo;
    crypto::Encrypt(cargo_buffer, t_cred, encrypted_cargo);
    crypto::Base64EncodeString(encrypted_cargo, cargo_out);
}

void FileHandler::UnpackCargo(FilePost& fp, 
                              const std::string& master_key,
                              Cargo& open_cargo) {
    std::string file_key;
    DecryptFileKey(fp.key_data(),
                   fp.iv_data(),
                   master_key,
                   file_key);

    // transient credentials
    Credentials t_cred;
    t_cred.set_key(file_key);
    t_cred.set_iv(fp.iv_data());

    //std::cout<<" ENCRYPTED CARGO : " << fp.cargo() << std::endl;
    std::string encrypted_cargo;
    crypto::Base64DecodeString(fp.cargo(), encrypted_cargo);

    std::string decrypted_cargo;
    crypto::Decrypt(encrypted_cargo, t_cred, decrypted_cargo);
    //std::cout<<" DECRYPTED CARGO : " << decrypted_cargo << std::endl;
    jsn::DeserializeObject(&open_cargo, decrypted_cargo);
}

void FileHandler::PrepareFilePost(FileInfo& fi, 
                                  const std::string& master_key,
                                  FilePost& out) {
    out.set_file_info(fi);
    // Prepare cargo
    std::string cargo;
    PrepareCargo(fi, master_key, cargo);
    out.set_cargo(cargo);
}

void FileHandler::DeserializeIntoFileInfo(FilePost& fp, 
                                          const std::string& master_key,
                                          FileInfo& out) {
    // TODO :: once cargo is working remove set file name set filepath

    out.set_encrypted_key(fp.key_data());
    out.set_file_credentials_iv(fp.iv_data());
    out.set_post_id(fp.id());
    out.set_folder_post_id(fp.folder_post());
    out.set_post_version(fp.version().id());
    out.set_file_size(fp.file_size());
    out.set_chunks(fp.chunk_data());
    out.set_chunk_count(fp.chunk_data().size());

    // Unpack Cargo
    Cargo cargo;
    UnpackCargo(fp, master_key, cargo);
    out.set_filename(cargo.filename);
    out.set_filepath(cargo.filepath);
    out.set_plaintext_hash(cargo.plaintext_hash);
}

bool FileHandler::GetTemporaryFilepath(FileInfo& fi, std::string& path_out) {
    std::string temp_path = file_manager_->temp_directory();
    if(fs::CheckFilepathExists(temp_path)) {
        utils::CheckUrlAndAppendTrailingSlash(temp_path);
        std::string randstr;
        utils::GenerateRandomString(randstr, 16);
        temp_path += fi.filename() + "_" + randstr;
        path_out = temp_path;
        return true;
    }
    return false;
}

// TODO :: reconsider the relevance of these crypto operations in this class,
//         perhaps move then to their own class? For now here is fine
bool FileHandler::EncryptFileKey(const std::string& filepath, 
                                 const std::string& file_key,
                                 const std::string& master_key) {
    if(!master_key.empty()) {
        FileInfo fi;
        if(RetrieveFileInfo(filepath, fi))
            return EncryptFileKey(fi, master_key);
    }
    return false;
}

bool FileHandler::EncryptFileKey(FileInfo& fi, const std::string& master_key) {

    if(!master_key.empty()) {
        std::string encrypted_key;
        if(EncryptFileKey(fi.file_credentials_key(),
                          fi.file_credentials_iv(), 
                          master_key, 
                          encrypted_key)) {
            fi.set_encrypted_key(encrypted_key);
            // Verify Encryption
            std::string decrypted_key;
            DecryptFileKey(encrypted_key, fi.file_credentials_iv(), master_key, decrypted_key);
            if(decrypted_key != fi.file_credentials_key())
                std::cout<<" FAILED TO VERIFY ENCRYPTED KEY ! " << std::endl;
            else {
                /*
                std::cout<<" KEY VERIFIED " << std::endl;
                std::cout<<" master key : " << master_key << std::endl;
                std::cout<<" encrypted key : " << encrypted_key << std::endl;
                std::cout<<" decrypted key : " << decrypted_key << std::endl;
                std::cout<<" iv : " << fi.file_credentials_iv() << std::endl;
                */
            }

            return true;
        }
    }
    return false;
}

bool FileHandler::EncryptFileKey(const std::string& file_key,
                                 const std::string& file_iv,
                                 const std::string& master_key,
                                 std::string& encrypted_out) {
    if(!master_key.empty() && !file_key.empty() && !file_iv.empty()) {
        Credentials tcred;                         // Create transient credentials
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(file_iv);                     // file specific iv
        // Encryption call
        crypto::Encrypt(file_key, tcred, encrypted_out);
        return true;
    }
    return false;
}

bool FileHandler::DecryptFileKey(const std::string& encrypted_key,
                                 const std::string& file_iv,
                                 const std::string& master_key,
                                 std::string& decrypted_out) {

    if(!master_key.empty() && !encrypted_key.empty() && !file_iv.empty()) {
        Credentials tcred;                         // Create transient credentials
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(file_iv);                     // file specific iv
        crypto::Decrypt(encrypted_key, tcred, decrypted_out);
        return true;
    }
    return false;
}

// Extract File credentials directly from a filepost
bool FileHandler::ExtractFileCredetials(const FilePost& fp,
                                        const std::string& master_key,
                                        Credentials& out) {
    if(!master_key.empty()) {
        Credentials tcred;                         // Create transient credentials
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(fp.iv_data());                // file specific iv

        std::string decrypted_key;
        if(crypto::Decrypt(fp.key_data(), tcred, decrypted_key)) {
            out.set_key(decrypted_key);
            out.set_iv(fp.iv_data());
            return true;
        }
    }
    return false;
}


bool FileHandler::RollFileMac(const std::string& filepath, std::string& out) {
    bool ret = false;
    if(fs::CheckFilepathExists(filepath)) {
        ret = crypto::GeneratePlaintextHashForFile(filepath, out);
    }
    return ret;
}

}//namespace

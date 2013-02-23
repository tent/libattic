#include "crypto.h"

//#include <iostream>
#include <fstream>

#include <hex.h>         
#include <filters.h>     
#include <gcm.h>         

#include <modes.h>
#include <sha.h>
#include <base64.h>
#include <hmac.h>

#include "utils.h"

//const int TAG_SIZE = 16;
//const int SALT_SIZE = 16;

Crypto::Crypto(unsigned int uStride)
{
    m_Stride = uStride;

}

Crypto::~Crypto()
{

}

Credentials Crypto::GenerateCredentials()
{
    // This is returning a copy on purpose, 
    // When this is called, credentials should 
    // be used, then stored (if needed), and fall 
    // out of scope as quickly as possible
    Credentials cred;
    GenerateCredentials(cred);
    return cred;
}

void Crypto::GenerateCredentials(Credentials& cred)
{
    byte key[CryptoPP::AES::MAX_KEYLENGTH+1];
    byte iv[CryptoPP::AES::BLOCKSIZE+1]; 

    m_Rnd.GenerateBlock( key, cred.GetKeySize()); // Generate a random key
    m_Rnd.GenerateBlock( iv, cred.GetIvSize()); // Generate a random IV

    cred.SetKey(key, CryptoPP::AES::MAX_KEYLENGTH);
    cred.SetIv(iv, CryptoPP::AES::BLOCKSIZE);
}


void Crypto::GenerateIv(std::string& out)
{
    byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0, CryptoPP::AES::BLOCKSIZE);
    m_Rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE); 

    out.append(reinterpret_cast<char*>(iv), CryptoPP::AES::BLOCKSIZE);
}



bool Crypto::GenerateHash( const std::string& source, 
                           std::string& hashOut)
{
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource src( source.c_str(), 
                                true,
                                new CryptoPP::HashFilter( hash,
                                    new CryptoPP::Base64Encoder (
                                        new CryptoPP::StringSink(hashOut)
                                        )
                                    )
                             );

    return true;
}

int Crypto::EncryptStringCFB( const std::string& data,
                              const Credentials& cred,
                              std::string& out)
{
    int status = ret::A_OK;

    try
    {
        std::string cipher;

        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        CryptoPP::StringSource( data,  // Plaintext
                                true, 
                                new CryptoPP::StreamTransformationFilter( e,
                                    new CryptoPP::StringSink(out)
                                ) // StreamTransformationFilter      
                    ); // StringSource
    }
    catch (CryptoPP::Exception &e)
    {
        status = ret::A_FAIL_ENCRYPT;
    }

    return status;
}

int Crypto::DecryptStringCFB( const std::string& cipher,
                              const Credentials& cred,
                              std::string& out)
{
    int status = ret::A_OK;

    try                                                                                 
    {                                                                                       
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d;        
        d.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        CryptoPP::StringSource( cipher, 
                                true, 
                                new CryptoPP::StreamTransformationFilter( d,
                                    new CryptoPP::StringSink( out )
                                         ) // StreamTransformationFilter
                               ); // StringSource
    }
    catch (CryptoPP::Exception &e)                               
    {                                                            
        status = ret::A_FAIL_DECRYPT;
    }                                                            

    return status;
}

int Crypto::EncryptStringGCM( const std::string& data,
                              const Credentials& cred,
                              std::string& out)
{
    int status = ret::A_OK;
    try
    {
        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        CryptoPP::StringSource( data,
                                true,
                                new CryptoPP::AuthenticatedEncryptionFilter( e,
                                new CryptoPP::StringSink(out),
                                false,
                                TAG_SIZE)
                              );
    }
    catch (CryptoPP::Exception &e)
    {
        status = ret::A_FAIL_ENCRYPT;
    }

    return status;
}

int Crypto::DecryptStringGCM( const std::string& cipher,
                              const Credentials& cred,
                              std::string& out)
{
    try                                                                                 
    {                                                                                       
        CryptoPP::GCM<CryptoPP::AES>::Decryption d;        
        d.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        // Recovered Plain Data                                                             
        std::string rpdata;                                                                 
        CryptoPP::AuthenticatedDecryptionFilter df ( d,                                              
                                                     new CryptoPP::StringSink(rpdata),
                                                     CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 
                                                     TAG_SIZE          
                                                   );                                                             
        CryptoPP::StringSource( cipher,                                                      
                                true,                                                 
                                new CryptoPP::Redirector( df /*  , PASS_EVERYTHING */ )
                              ); // StringSource

        // If the object does not throw, here's the only                 
        //  opportunity to check the data's integrity                    
        if (df.GetLastResult() == true )                                 
        {                                                                
            //std::cout<< "recovered text : " << rpdata << "\n";           
            // Write out data to ofstream
            out = rpdata;
        }                                                                

    }
    catch (CryptoPP::Exception &e)                               
    {                                                            
        //std::cerr << e.what() << "\n";                           
        return ret::A_FAIL_DECRYPT;
    }                                                            


    return ret::A_OK;
}
                       
ret::eCode Crypto::EncryptFile( const std::string &filepath, 
                                const std::string &outputPath, 
                                const Credentials &cred)
{
    // create ifstream (read in)
    std::ifstream ifs;
    // open file
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    // create ofstream (write out)
    std::ofstream ofs;
    ofs.open(outputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    // Get Sizeof file
    char* pBuffer = new char[m_Stride];
    // begin reading
    
    unsigned int totalread = 0;
    while(!ifs.eof())
    {
        memset(pBuffer, 0, (sizeof(char)*m_Stride));
        // read into buffer
        ifs.read(pBuffer, m_Stride);
        unsigned int readCount = ifs.gcount();

        if(!EncryptData(pBuffer, readCount, cred, ofs))
        {
            ifs.close();
            ofs.close();
            if(pBuffer)
            {
                delete pBuffer;
                pBuffer = 0;
            }
            return ret::A_FAIL_ENCRYPT;
        }
        totalread += readCount;
    }

    // Generate mac for file

    if(pBuffer)
    {
        delete pBuffer;
        pBuffer = 0;
    }

    ifs.close();
    ofs.close();
    // USE STRIDE for the size you read in per iteration
    // call encrypt data to write out to file (pass output stream)
    // close file
    return ret::A_OK;
}

// Depricated
bool Crypto::EncryptData( const char* pData, 
                          unsigned int size, 
                          const Credentials& cred, 
                          std::ofstream& ofs)
{
    // Take data,
    // Encrypt
    // write out to buffer
    if(!pData)
        return false;

    try
    {
        std::string cipher;
        std::string data;
        data.append(pData, size);

        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        CryptoPP::StringSource( data,
                                true,
                                new CryptoPP::AuthenticatedEncryptionFilter( e,
                                new CryptoPP::StringSink(cipher),
                                false,
                                TAG_SIZE)
                                                                                                                          );

       // Write out cipher to ofstream
       ofs.write(cipher.c_str(), cipher.size());

       /*
       std::string holdkey;
       holdkey.append(cred.m_Key, cred.GetKeySize());
       std::cout<< "HOLD KEY : " << holdkey << std::endl;
       */


    }
    catch (CryptoPP::Exception &e)
    {
            //std::cerr << e.what() << "\n";
            return false;
    }

    return true;
}

// Depricated
ret::eCode Crypto::DecryptFile( const std::string &szFilePath, 
                                const std::string &outputPath, 
                                const Credentials &cred)
{

    // szFilePath, is the path to the encrypted data.
    // create ifstream (read in)
    std::ifstream ifs;
    ifs.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    // create ofstream (write out)
    std::ofstream ofs;
    ofs.open(outputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    char* pBuffer = new char[m_Stride + TAG_SIZE];

    // begin reading
    while(!ifs.eof())
    {
        memset(pBuffer, 0, (sizeof(char)*m_Stride) + TAG_SIZE);
        ifs.read(pBuffer, m_Stride + TAG_SIZE);
        unsigned int readCount = ifs.gcount();

        // call decrypt data to write out to file (pass output stream)
        if(!DecryptData(pBuffer, readCount, cred, ofs))
        {
            //std::cerr<<"FAILED TO DECRYPT DATA\n";
            ifs.close();
            ofs.close();
            if(pBuffer)
            {
                delete pBuffer;
                pBuffer = 0;
            }
            return ret::A_FAIL_DECRYPT;
        }
    }

    if(pBuffer)
    {
        delete pBuffer;
        pBuffer = 0;
    }
    // close file
    ifs.close();
    ofs.close();
    return ret::A_OK;
}

bool Crypto::DecryptData( const char* pData, 
                          unsigned int size, 
                          const Credentials &cred, 
                          std::ofstream &ofs)
{
    // pData is the cypher
    // Read in cypher
    // Decrypt
    // write out to ostream

    try                                                                                 
    {                                                                                       
        std::string cipher;
        cipher.append(pData, size);
        CryptoPP::GCM<CryptoPP::AES>::Decryption d;        
        d.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

        // Recovered Plain Data                                                             
        std::string rpdata;                                                                 
        CryptoPP::AuthenticatedDecryptionFilter df ( d,                                              
                                                     new CryptoPP::StringSink(rpdata),
                                                     CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 
                                                     TAG_SIZE          
                                                   );                                                             
        // The StringSource dtor will be called immediately                 
        // after construction below. This will cause the    
        // destruction of objects it owns. To stop the                                         
        // behavior so we can get the decoding result from   
        // the DecryptionFilter, we must use a redirector 
        // or manually Put(...) into the filter without                              
        // using a StringSource.   
        CryptoPP::StringSource( cipher,                                                      
                                true,                                                 
                                new CryptoPP::Redirector( df /*  , PASS_EVERYTHING */ )
                              ); // StringSource



        // If the object does not throw, here's the only                 
        //  opportunity to check the data's integrity                    
        if (df.GetLastResult() == true )                                 
        {                                                                
            //std::cout<< "recovered text : " << rpdata << "\n";           
            // Write out data to ofstream
            ofs.write(rpdata.c_str(), rpdata.size());
        }                                                                

    }
    catch (CryptoPP::Exception &e)                               
    {                                                            
        //std::cerr << e.what() << "\n";                           
        return false;
    }                                                            

    return true;
}
extern "C"
{
    #include "crypto_scrypt.h"
    int crypto_scrypt( const uint8_t *,
                       size_t,
                       const uint8_t *,
                       size_t,
                       uint64_t,
                       uint32_t,
                       uint32_t,
                       uint8_t *,
                       size_t);
}

int Crypto::GenerateKeyFromPassphrase( const std::string& pass, 
                                       std::string& salt,
                                       Credentials& out)
{
    int status = ret::A_OK;
    std::string outKey; //, outIv;

    // Check salt for size and correctness
    status = CheckSalt(salt);
    if(status == ret::A_OK)
    {
        ScryptEncode(pass, salt, outKey, CryptoPP::AES::MAX_KEYLENGTH);
        //ScryptEncode(name, outIv, CryptoPP::AES::BLOCKSIZE);
        memcpy(out.m_Key, outKey.c_str(), CryptoPP::AES::MAX_KEYLENGTH);
        memcpy(out.m_Iv, salt.c_str(), salt.size());
    }

    return status;
}

bool Crypto::ScryptEncode( const std::string &input, 
                           const std::string &salt,
                           std::string &out,
                           unsigned int size)
{
    // Note* pass in 16 bit salt
    //uint8_t salt[32]; // 16 <- do 16, 64 or 128

    uint8_t* password;
    size_t plen;

    uint64_t N = 16384;
    uint32_t r = 8;
    uint32_t p = 1;

    //uint8_t dk[64]; // Derived key
    uint8_t dk[size]; // Derived key

   /* 
    std::cout<< " SIZE : " << size << std::endl;
    std::cout<< " INPUT SIZE : " << input.size() << std::endl;
    */
    
    byte* pInput = new byte[input.size()];
    memcpy(pInput, reinterpret_cast<const unsigned char*>(input.c_str()), input.size());

    byte* pSalt = new byte[salt.size()];
    memcpy(pSalt, reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size());

    /*std::cout << */crypto_scrypt( pInput,
                                input.size(),
                                pSalt,
                                salt.size(),
                                N,
                                r,
                                p,
                                dk,
                                size); /*<< std::endl;*/
    

    /*
    std::cout << crypto_scrypt( (uint8_t*)input.c_str(),
                                input.size(),
                                (uint8_t*)"supersalt",
                                9,
                                N,
                                r,
                                p,
                                dk,
                                size) << std::endl;
                                */
    
    out.append( reinterpret_cast<char*>(dk), sizeof(uint8_t)*size);

    if(pInput)
    {
        delete[] pInput;
        pInput = NULL;
    }

    if(pSalt)
    {
        delete[] pSalt;
        pSalt = NULL;
    }

    return true;
}

int Crypto::CheckSalt(std::string& salt)
{
    int status = ret::A_OK;

    // Check for correct size
    if(salt.size() != SALT_SIZE)
    {
        status = ret::A_FAIL_SCRYPT_INVALID_SALT_SIZE;
    }
    return status;
}

int Crypto::GenerateSalt(std::string& out)
{
    int status = ret::A_OK;
    utils::GenerateRandomString(out, SALT_SIZE);
    return status;
}

int Crypto::GenerateHMACForString( const std::string& input,
                                   const Credentials& cred,
                                   std::string& macOut)
{
    int status = ret::A_OK;
    std::string mac;
    try
    {
        CryptoPP::HMAC<CryptoPP::SHA256> hmac(cred.m_Key, cred.GetKeySize());
        CryptoPP::StringSource( input,
                                true,
                                new CryptoPP::HashFilter( hmac,
                                    new CryptoPP::StringSink(mac)
                                    )
                              );
    }
    catch(const CryptoPP::Exception& e)
    {
        std::cout << e.what() << std::endl;
        status = ret::A_FAIL_HMAC;
    }

    if(status == ret::A_OK)
    {
        std::string hexencoded;
        // Encode to hex
        CryptoPP::StringSource( mac, 
                      true,
                      new CryptoPP::HexEncoder(
                         new CryptoPP::StringSink(hexencoded)
                         ) // HexEncoder
                     ); // StringSource

        if(!hexencoded.empty())
        {
            macOut = hexencoded;
        }
        else
        {
            status = ret::A_FAIL_HEX_ENCODE;
        }
    }

    return status;
}

int Crypto::VerifyHMACForString( const std::string& input,
                                 const Credentials& cred,
                                 const std::string& mac)

{
    int status = ret::A_OK;

    try
    {
        // Decode hmac
        std::string decoded;
        CryptoPP::StringSource ss( mac,
                                   true,
                                   new CryptoPP::HexDecoder(
                                        new CryptoPP::StringSink(decoded)
                            ) // HexDecoder
        ); // StringSource

        if(!decoded.empty())
        {

            CryptoPP::HMAC<CryptoPP::SHA256> hmac(cred.m_Key, cred.GetKeySize());

            const int flags = CryptoPP::HashVerificationFilter::THROW_EXCEPTION | CryptoPP::HashVerificationFilter::HASH_AT_END;
        
            CryptoPP::StringSource( input + decoded, 
                                    true, 
                                    new CryptoPP::HashVerificationFilter(hmac, NULL, flags)
                                   ); // StringSource

        }
        else
        {
            status = ret::A_FAIL_HEX_DECODE;
        }
    }
    catch(const CryptoPP::Exception& e)
    {
        std::cout << e.what() << std::endl;
        status = ret::A_FAIL_HMAC_VERIFY;
    }

    return status;
}


int Crypto::GenerateHMACForFile( const std::string& filepath,
                                 const Credentials& cred,
                                 std::string& macOut)
{
    int status = ret::A_OK;

    std::ifstream ifs;
    // open file
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(ifs.is_open())
    {
        unsigned int filesize = utils::CheckFilesize(filepath);
        char* pBuffer = new char[filesize];
    
        memset(pBuffer, 0, (sizeof(char)*filesize));
        // read into buffer
        ifs.read(pBuffer, filesize);
        unsigned int readCount = ifs.gcount();

        ifs.close();

        status = GenerateHMACForString( std::string(pBuffer), 
                                        cred,
                                        macOut);
    }
    else
    {
        status = ret::A_FAIL_OPEN_FILE;
    }


    return status;
}

int Crypto::VerifyHMACForFile( const std::string& filepath,
                               const Credentials& cred,
                               const std::string& mac)
{
    int status = ret::A_OK;

    std::ifstream ifs;
    // open file
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(ifs.is_open())
    {
        unsigned int filesize = utils::CheckFilesize(filepath);
        char* pBuffer = new char[filesize];
    
        memset(pBuffer, 0, (sizeof(char)*filesize));
        // read into buffer
        ifs.read(pBuffer, filesize);
        unsigned int readCount = ifs.gcount();

        ifs.close();

        status = VerifyHMACForString( std::string(pBuffer), 
                                      cred,
                                      mac);
    }
    else
    {
        status = ret::A_FAIL_OPEN_FILE;
    }

    return status;
}



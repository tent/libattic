
#include "crypto.h"

#include <iostream>
#include <fstream>

#include <hex.h>         
#include <filters.h>     
#include <gcm.h>         

#include <sha.h>
#include <base64.h>

#include "utils.h"

const int TAG_SIZE = 16;
const int SALT_SIZE = 16;

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
    memset(cred.m_Key, 0, cred.GetKeySize());
    memset(cred.m_Iv, 0, cred.GetIvSize());

    // Generate a random key
    CryptoPP::SecByteBlock key(CryptoPP::AES::MAX_KEYLENGTH);
    m_Rnd.GenerateBlock( cred.m_Key, cred.GetKeySize());

    // // Generate a random IV
    m_Rnd.GenerateBlock(cred.m_Iv, cred.GetIvSize()); 

    return cred;
}

void Crypto::GenerateIv(std::string& out)
{
    byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0, CryptoPP::AES::BLOCKSIZE);
    m_Rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE); 

    out.append(reinterpret_cast<char*>(iv), CryptoPP::AES::BLOCKSIZE);
}

void Crypto::GenerateCredentials(Credentials& cred)
{
    memset(cred.m_Key, 0, cred.GetKeySize());
    memset(cred.m_Iv, 0, cred.GetIvSize());

    // Generate a random key
    CryptoPP::SecByteBlock key(CryptoPP::AES::MAX_KEYLENGTH);
    m_Rnd.GenerateBlock( cred.m_Key, cred.GetKeySize());

    // // Generate a random IV
    m_Rnd.GenerateBlock(cred.m_Iv, cred.GetIvSize()); 
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

int Crypto::EncryptString( const std::string& data,
                           const Credentials& cred,
                           std::string& out)
{
    try
    {
        std::string cipher;

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
       out = cipher;
       /*
       std::cout<< "CIPHER SIZE : " << cipher.size() << std::endl;
       std::cout << " KEY : " << cred.m_Key << std::endl;
       std::cout << " IV : " << cred.m_Iv << std::endl;
       */

    }
    catch (CryptoPP::Exception &e)
    {
            std::cerr << e.what() << "\n";
            return ret::A_FAIL_ENCRYPT;
    }

    return ret::A_OK;
}

int Crypto::DecryptString( const std::string& cipher,
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
        std::cerr << e.what() << "\n";                           
        return ret::A_FAIL_DECRYPT;
    }                                                            


    return ret::A_OK;
}
                       
ret::eCode Crypto::EncryptFile( const std::string &szFilepath, 
                                const std::string &szOutputPath, 
                                const Credentials &cred)
{
    // create ifstream (read in)
    std::ifstream ifs;
    // open file
    ifs.open(szFilepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    // create ofstream (write out)
    std::ofstream ofs;
    ofs.open(szOutputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

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
            std::cerr<<"FAILED TO ENCRYPT DATA\n";
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
    std::cout<<"TOTAL READ : " << totalread << std::endl;

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
       std::cout<< "CIPHER SIZE : " << cipher.size() << std::endl;
       std::cout << " KEY : " << cred.m_Key << std::endl;
       std::cout << " IV : " << cred.m_Iv << std::endl;

       /*
       std::string holdkey;
       holdkey.append(cred.m_Key, cred.GetKeySize());
       std::cout<< "HOLD KEY : " << holdkey << std::endl;
       */


    }
    catch (CryptoPP::Exception &e)
    {
            std::cerr << e.what() << "\n";
            return false;
    }

    return true;
}

ret::eCode Crypto::DecryptFile( const std::string &szFilePath, 
                                const std::string &szOutputPath, 
                                const Credentials &cred)
{

    std::cout << " KEY : " << reinterpret_cast<const char*>(cred.m_Key) << std::endl;
    std::cout << " IV : " << reinterpret_cast<const char*>(cred.m_Iv) << std::endl;

    // szFilePath, is the path to the encrypted data.

    // create ifstream (read in)
    std::ifstream ifs;
    ifs.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    // create ofstream (write out)
    std::ofstream ofs;
    ofs.open(szOutputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

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
            std::cerr<<"FAILED TO DECRYPT DATA\n";
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
        std::cerr << e.what() << "\n";                           
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

        // Copy into credentials // Char to to byte (unsigned char) conversion
        // just allow it.
        memcpy(out.m_Key, outKey.c_str(), CryptoPP::AES::MAX_KEYLENGTH);
        //memcpy(out.iv, outIv.c_str(), CryptoPP::AES::BLOCKSIZE);

        std::cout << "Cred key : \n" << out.m_Key << std::endl;
        //std::cout << "Cred iv : \n" << out.iv << std::endl;
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
    
//    std::cout<< " INPUT : " << input << std::endl;
//    std::cout<< " SIZE : " << size << std::endl;

    char* pData = new char[size];
    memcpy(pData, input.c_str(), size);

 //   std::cout<< "Data Buffer : \n" << pData << std::endl;

    uint8_t* buf = reinterpret_cast<uint8_t*>(pData);

    std::cout << crypto_scrypt( (uint8_t*)input.c_str(),
                                input.size(),
                                (uint8_t*)salt.c_str(),
                                salt.size(),
                                N,
                                r,
                                p,
                                dk,
                                size) << std::endl;
    

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

    if(pData)
    {
        delete pData;
        pData = NULL;
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


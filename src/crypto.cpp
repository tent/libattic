
#include "crypto.h"

#include <iostream>
#include <fstream>

#include <hex.h>         
#include <filters.h>     
#include <aes.h>         
#include <gcm.h>         

const int TAG_SIZE = 16;

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
    memset(cred.key, 0, cred.GetKeySize());
    memset(cred.iv, 0, cred.GetIvSize());

    // Generate a random key
    CryptoPP::SecByteBlock key(CryptoPP::AES::MAX_KEYLENGTH);
    m_Rnd.GenerateBlock( cred.key, cred.GetKeySize());

    // // Generate a random IV
    m_Rnd.GenerateBlock(cred.iv, cred.GetIvSize()); 

    return cred;
}


bool Crypto::EncryptFile(std::string &szFilepath, std::string &szOutputPath, Credentials &cred)
{
   // open file
   // create ifstream (read in)
   // create ofstream (write out)
   // begin reading
   // call encrypt data to write out to file (pass output stream)
   // close file
   return true;
}

bool Crypto::EncryptData(const char* pData, unsigned int size, Credentials &cred, std::ofstream &ofs)
{
    // Take data,
    // Encrypt
    // write out to buffer
    if(!pData)
        return false;

    try
    {
        std::string cipher;

        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.key, cred.GetKeySize(), cred.iv, cred.GetIvSize());

        CryptoPP::StringSource( pData,
                                true,
                                new CryptoPP::AuthenticatedEncryptionFilter( e,
                                new CryptoPP::StringSink(cipher),
                                false,
                                TAG_SIZE)
                                                                                                                          );

       std::cout<<" SOME DATA : " << pData << "\n";
       std::cout<<" cipher : " << cipher << "\n";

       // Write out cipher to ofstream
    }
    catch (CryptoPP::Exception &e)
    {
            std::cerr << e.what() << "\n";
            return false;
    }

    return true;

    
    return true;
}

bool Crypto::DecryptFile(std::string &szFilePath, std::string &szOutputPath, Credentials &cred)
{
    // open file
    // create ifstream (read in)
    // create ofstream (write out)
    // begin reading
    // call decrypt data to write out to file (pass output stream)
    // close file
}

bool Crypto::DecryptData(const char* pData, std::string &cipher, Credentials &cred, std::ofstream &ofs)
{
    // Read in cypher
    // Decrypt
    // write out to ostream

    try                                                                                                           
    {                                                                                                             
        CryptoPP::GCM<CryptoPP::AES>::Decryption d;                                                               
        d.SetKeyWithIV(cred.key, cred.GetKeySize(), cred.iv, cred.GetIvSize());

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
            std::cout<< "recovered text : " << rpdata << "\n";           
            // Write out data to ofstream
        }                                                                

    }
    catch (CryptoPP::Exception &e)                               
    {                                                            
        std::cerr << e.what() << "\n";                           
        return false;
    }                                                            

    return true;
}

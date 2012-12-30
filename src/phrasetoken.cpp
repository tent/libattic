#include "phrasetoken.h"

#include <fstream>

#include "utils.h"
#include "errorcodes.h"

PhraseToken::PhraseToken()
{

}

PhraseToken::~PhraseToken()
{

}

void PhraseToken::Serialize(Json::Value& root)
{
    root["key"] = m_Key;
}

void PhraseToken::Deserialize(Json::Value& root)
{
    m_Key = root.get("key", "").asString();
}

int PhraseToken::SaveToFile(const std::string& filepath)
{
    std::ofstream ofs;


    ofs.open(filepath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

    std::string serialized;
    JsonSerializer::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();

    return ret::A_OK;
}

int PhraseToken::LoadFromFile(const std::string& filepath)
{
    std::ifstream ifs;                                                                            
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);                        
                                                                                                  
    if(!ifs.is_open())                                                                            
        return ret::A_FAIL_OPEN;                                                                  
                                                                                                  
    unsigned int size = utils::CheckIStreamSize(ifs);                                             
    char* pBuf = new char[size+1];                                                                
    pBuf[size] = '\0';                                                                            
                                                                                                  
    ifs.read(pBuf, size);                                                                         
                                                                                                  
    // sanity check size and readcount should be the same                                         
    int readcount = ifs.gcount();                                                                 
    if(readcount != size)                                                                         
    std::cout<<"READCOUNT NOT EQUAL TO SIZE\n";                                               
    
    std::string loaded(pBuf);                                                                     
    
    if(pBuf)                                                                                      
    {                                                                                             
        delete[] pBuf;                                                                            
        pBuf = 0;                                                                                 
    }                                                                                             
    // Deserialize into self.                                                                     
    JsonSerializer::DeserializeObject(this, loaded);                                              
    
    return ret::A_OK;                                                                             
}




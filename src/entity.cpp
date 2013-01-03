#include "entity.h"


Entity::Entity()
{

}

Entity::~Entity()
{
    ProfileList::iterator itr = m_Profiles.begin();

    while(itr != m_Profiles.end())
    {
        if(*itr)
        {
            // TODO :: step through this and make sure
            //         it does what you think it does
            delete (*itr);
            (*itr) = NULL;
        }
        itr++;
    }

    m_Profiles.clear();
}
 


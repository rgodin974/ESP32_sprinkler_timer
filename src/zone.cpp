
#include "zone.h"

int Zone::m_searchIndex;
Zone Zone::m_zone[MAX_ZONE];

/********** Constructor **********/
Zone::Zone() : m_name("")
{
}


/********** Create a zone **********/
bool Zone::create(const char *def)
{
    Zone *z;

    // DEBUG Serial.printf("zone::create %s\n", def);
    z = &m_zone[m_searchIndex];
    z->m_name = def;
    m_searchIndex++;
    return true;
}


/********** Return number of zones **********/
int Zone::getCount()
{
    int n;

    for (n = 0 ; n < MAX_ZONE && m_zone[n].m_name != 0 ; n++);
    return n;
}


/********** Return zone name **********/
const char *Zone::getName(void)
{
    return m_name.c_str();
}


/********** Return first zone **********/
Zone *Zone::getFirst(void)
{
  m_searchIndex = 0;
  return &m_zone[m_searchIndex];
}


/********** Return next zone **********/
Zone *Zone::getNext(void)
{
  m_searchIndex++;
  if (m_searchIndex < MAX_ZONE && m_zone[m_searchIndex].m_name != "") {
    return &m_zone[m_searchIndex];
  }
  return 0;
}


/********** Print zone **********/
void Zone::print(void)
{
  Serial.printf("%d: %s\n", m_searchIndex, getName());
}
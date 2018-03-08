/*
 * NodeType.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */

#include <cassert>

#include "Metadata.h"



bool Metadata::addData(const std::string &field, const std::string &val)
{
	string_map[field] = val;

	if(std::find(key_order.begin(), key_order.end(), field) == key_order.end())
		key_order.push_back(field);

	return true;
}

const std::string& Metadata::getVal(const std::string &field) const
{

  static std::string foo = "";
  std::map<std::string,std::string>::const_iterator itr = string_map.find(field);
  if (itr != string_map.end())
    return itr->second;
  else 
    return foo;
}

const std::string& Metadata::getVal(unsigned int index) const
{
  assert (index < key_order.size());
  return getVal(key_order[index]);
}

void Metadata::setVal(unsigned int index,const std::string &val)
{
  assert (index < key_order.size());
  string_map[key_order[index]] = val;
}

const std::string& Metadata::getField(unsigned int index) const
{
  assert (index < key_order.size());
  return key_order[index];
}


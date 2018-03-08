/*
 * NodeType.h
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#ifndef NODETYPE_H_
#define NODETYPE_H_

class Metadata{

public:
	
  // ACCESSORS
  const std::string& getVal(const std::string &field) const;
  const std::string& getVal(unsigned int index) const;
  const std::string& getField(unsigned int index) const;
  unsigned int numFields() const {return key_order.size();}
  bool operator==(const Metadata& other) const;
  
  // MODIFIERS
  bool addData(const std::string &field, const std::string &val);
  void setVal(unsigned int index, const std::string&);

private:
  //Let's be clear here, this shit right here is for
  //edittable data in the synenv app, so let's not
  //mess with that
  std::map<std::string, std::string> string_map;
  std::vector<std::string> key_order;
};

#endif /* NODETYPE_H_ */

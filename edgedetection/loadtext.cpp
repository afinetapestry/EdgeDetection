//
//  LoadText.cpp
//  clenc
//
//  Created by Jonathan Hatchett on 12/02/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "LoadText.h"

#include <fstream>
#include <streambuf>

void LoadText(std::string filename, std::string & str) {
	std::ifstream t(filename.c_str());
	
	t.seekg(0, std::ios::end);   
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);
	
	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

#include "DataTypes.h"

JLDDoc::JLDDoc() : JLDData(),
data(std::list<std::pair<std::string, JLDData*>>())
{
}

JLDDoc::JLDDoc(std::list<std::pair<std::string, JLDData*>> d) : JLDData(),
data(std::list<std::pair<std::string, JLDData*>>(d))
{
}

JLDData* JLDDoc::getValue(std::string key)
{
	for (auto kvp : data)
	{
		auto kvpKey = kvp.first;
		if(key == kvpKey)
			return kvp.second;
	}
	return NULL;
}

bool JLDDoc::push_back(std::string key, JLDData* value)
{
	if(hasKey(key))
		return false;
	data.push_back(std::pair<std::string, JLDData*>(key, value));
	return true;
}

bool JLDDoc::push_front(std::string key, JLDData* value)
{
	if(hasKey(key))
		return false;
	data.push_front(std::pair<std::string, JLDData*>(key, value));
	return true;
}

bool JLDDoc::insert(std::string key, JLDData* value, int index)
{
	if(data.size() < index || hasKey(key))
		return false;
	data.insert(std::next(data.begin(), index), std::pair<std::string, JLDData*>(key, value));
	return true;
}

bool JLDDoc::replace(std::string key, JLDData* value)
{
	if(hasKey(key))
		return false;
	// Record the position to insert the replacement at
	int i = 0;
	for (auto kvp : data)
	{
		if(kvp.first == key)
			break;
		i++;
	}
	remove(key);
	insert(key, value, i);
	return true;
}

bool JLDDoc::remove(std::string key)
{
	if(hasKey(key))
		return false;
	delete getValue(key);
	int i = 0;
	for (auto kvp : data)
	{
		if(kvp.first == key)
			break;
		i++;
	}
	data.erase(std::next(data.begin(), i));
	return true;
}

bool JLDDoc::hasKey(std::string key)
{
	for (auto kvp : data)
	{
		if(key == kvp.first)
			return true;
	}
	return false;
}

///gets the next keyword in a period separated command string.
///strips the input to after the keyword
///e.g. "key1.key2.key3" -> key1 returned, input changed to key2.key3
///Q: What if the keyword needs to contain the period?
///A: Why would you do that
std::string JLDDoc::getNext(std::string* command)
{
	assert((*command)[0] != '.' && "Extra period");
	std::string out = "";
	int i = 0;
	while (i < command->size() && (*command)[i] != '.')
	{
		out += (*command)[i];
		i++;
	}
	*command = command->substr(i+1);
	assert((*command)[0] != '.' && "Extra period");
	return out;
}

std::string JLDDoc::toString(int tabIndex, bool compact)
{
	std::string out;
	if(compact)
		out = "{";
	else
	{
		out = "\n";
		for(int i=0; i<tabIndex; i++)
			out += tabString;
		out += "{\n";
	}
	tabIndex++;
	for (auto kvp : data)
	{
		std::string key = kvp.first;
		char delim = '\"';
		for(char c : key)
		{
			if(c == '\"')
			{
				delim = '\'';
				break;
			}
		}
		if(!compact)
		{
			for(int i=0; i<tabIndex; i++)
				out += tabString;
		}
		out += delim + key + delim;
		if(compact)
			out += ":";
		else
			out += " : ";
		out += kvp.second->toString(tabIndex + 1, compact) + ",";
		if(!compact)
			out += "\n";
	}
	if(data.size())
	{
		int shortenBy = 1;
		if(!compact)
			shortenBy++;
		out = out.substr(0, out.size()-shortenBy);
		if(!compact)
		{
			out += "\n";
			for(int i=0; i<tabIndex-1; i++)
				out += tabString;
		}
		out += "}";
	}
	else
		out = "{}";
	return out;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


JLDList::JLDList() : JLDData(),
data(std::list<JLDData*>())
{
}

JLDList::JLDList(std::list<JLDData*> d) : JLDData(),
data(std::list<JLDData*>(d))
{
}

JLDData* JLDList::getValue(int index)
{
	if(data.size() < index)
		return NULL;
	return *std::next(data.begin(), index);
}

void JLDList::push_back(JLDData* value)
{
	data.push_back(value);
}

void JLDList::push_front(JLDData* value)
{
	data.push_front(value);
}

bool JLDList::insert(JLDData* value, int index)
{
	if(data.size() < index)
		return false;
	data.insert(std::next(data.begin(), index), value);
	return true;
}

bool JLDList::replace(int index, JLDData* value)
{
	if(data.size() <= index)
		return false;
	remove(index);
	insert(value, index);
	return true;
}

bool JLDList::remove(int index)
{
	if(data.size() <= index)
		return false;
	delete *std::next(data.begin(), index);
	data.erase(std::next(data.begin(), index));
	return true;
}

std::string JLDList::toString(int tabIndex, bool compact)
{
	std::string out = "[";
	bool firstItem = true;
	for(auto iter : data)
	{
		if(!firstItem)
		{
			out += ",";
			if(!compact)
				out += " ";
		}
		firstItem = false;
		out += iter->toString(tabIndex + 1, compact);
	}
	return out + "]";
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


JLDString::JLDString() : JLDData(),
data("")
{
}

JLDString::JLDString(std::string d) : JLDData(),
data(d)
{
}

std::string JLDString::getValue()
{
	return data;
}

/// params not used
std::string JLDString::toString(int tabIndex, bool compact)
{
	char delim = '\"';
	for(char c : data)
	{
		if(c == '\"')
		{
			delim = '\'';
			break;
		}
	}
	return std::string(delim + data + delim);
}
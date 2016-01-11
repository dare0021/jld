#ifndef __JLD_DATATYPES_H__
#define __JLD_DATATYPES_H__

#include <list>
#include <string>
#include <assert.h>

enum class JLDDataType
{
    Doc,
    List,
    String
};

class JLDData
{
public:
	JLDData(){}
	virtual JLDDataType getType() = 0;
    virtual std::string toString(int tabIndex = 0, bool compact = false) = 0;

protected:
    std::string tabString = "    ";
};

class JLDDoc : public JLDData
{
public:
    JLDDoc();
    JLDDoc(std::list<std::pair<std::string, JLDData*>>);
    JLDDataType getType(){return JLDDataType::Doc;}
    JLDData* getValue(std::string key);
    /// Fails on finding an entry with the same key
    bool push_back(std::string key, JLDData* value);
    /// Fails on finding an entry with the same key
    bool push_front(std::string key, JLDData* value);
    /// Inserts the new item as the pos'th position and pushes the rest back
    /// e.g. insert(, , 0) is the same as push_front()
    /// Fails on finding an entry with the same key
    bool insert(std::string key, JLDData* value, int index);
    /// Removes the entry of the given key and inserts a new entry
    /// with the same key using the new value.
    /// The old value is deleted.
    /// Fails on finding no entry with the same key
    bool replace(std::string key, JLDData* value);
    /// Fails on finding no entry with the same key
    bool remove(std::string key);
    bool hasKey(std::string key);
    std::string toString(int tabIndex = 0, bool compact = false);

private:
    std::string getNext(std::string* command);
    std::list<std::pair<std::string, JLDData*>> data;
};

class JLDList : public JLDData
{
public:
    JLDList();
    JLDList(std::list<JLDData*>);
    JLDDataType getType(){return JLDDataType::List;}
    JLDData* getValue(int index);
    void push_back(JLDData* value);
    void push_front(JLDData* value);
    bool insert(JLDData* value, int index);
    bool replace(int index, JLDData* value);
    bool remove(int index);
    std::string toString(int tabIndex = 0, bool compact = false);

private:
    std::list<JLDData*> data;
};

class JLDString : public JLDData
{
public:
    JLDString();
    JLDString(std::string);
    JLDDataType getType(){return JLDDataType::String;}
    std::string getValue();
    std::string toString(int tabIndex = 0, bool compact = false);

private:
    std::string data;
};

#endif // __JLD_DATATYPES_H__
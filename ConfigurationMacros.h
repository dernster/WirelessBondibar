#include "Arduino.h"
#include "Dictionary.h"
#include <typeinfo>

#include <stdarg.h>
#include <vector>
using namespace std;

class Config;
typedef void (*setterPtr)(void* intance,const String& value);
typedef String (*getterPtr)(void* intance);

class StringConvertibleVariable{
public:
  StringConvertibleVariable(String name, setterPtr setter, getterPtr getter){
    this->name = name;
    this->setter = setter;
    this->getter = getter;
  }
  String name;
  setterPtr setter;
  getterPtr getter;
  
  virtual void persist(){
    
  }
  virtual bool isPersistent(){
    return false;
  }
};

template<class T>
class Variable: public StringConvertibleVariable{
public:
  Variable(T* variable, String name, setterPtr setter, getterPtr getter): StringConvertibleVariable(name,setter,getter){
    this->variable = variable;
  }
  T* variable;

  virtual void persist(){};
};

template<class T>
class PersistentVariable : public Variable<T>{
public:
  PersistentVariable(T* variable, String name, setterPtr setter, getterPtr getter, int size): Variable<T>(variable,name,setter,getter){
  }
  virtual void persist(){
    Serial.println("Persisting variable = " + String(*Variable<T>::variable));
  }
  int address;
};

class Config{
public:
  void setValue(const String& tag, const String& value){
    setterPtr f = getSetterFunction(tag);
    if (f){
      f(this,value);
    }
  };
    
  setterPtr getSetterFunction(const String& tag){
    int size = getMapperLength();
    StringConvertibleVariable* array = getMapper();
    for (int i = 0; i < size; i++){
      if (tag == getTag(array[i].name)){
        return array[i].setter;
      }
    }
    return NULL; 
  }

  String getTag(const String& varName){
    return getPrefix() + "." + varName;
  }

  Dictionary toDictionary(){
    Dictionary result;
    int size = getMapperLength();
    StringConvertibleVariable* array = getMapper();
    for (int i = 0; i < size; i++){
      result[getTag(array[i].name)] = array[i].getter(this);
    }
    return result;
  }
  
  String toString(){
    String result = "";
    int size = getMapperLength();
    StringConvertibleVariable* array = getMapper();
    for (int i = 0; i < size; i++){
      result += getPrefix() + "." + array[i].name + ":" + array[i].getter(this) +  (i == size-1) ? " " : "";
    }
    return result;
  }
  
  virtual String getPrefix() = 0;
  virtual StringConvertibleVariable* getMapper() = 0;
  virtual int getMapperLength() = 0;
};

//-------------------------
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N

#define var(type,name,setBody,getBody)\
private:\
  typedef type name ## __TYPE;\
public:\
type name; setterHeader(name,setBody); getterHeader(name,getBody);

#define persistentVar(type,name,setBody,getBody)\
private:\
  typedef type name ## __TYPE;\
public:\
type name; setterHeader(name,setBody); getterHeader(name,getBody);\
void persist_ ## name(){\
  StringConvertibleVariable* var = map(#name);\
  if (var){\
    var->persist();\
  }\
}

#define readOnly(type,name,getBody)\
private:\
  typedef type name ## __TYPE;\
public:\
type name; setterHeader(name,{}); getterHeader(name,getBody);

//---------------------------

#define FE_1(WHAT, X)       WHAT(X) 
#define FE_2(WHAT, X, ...)  WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...)  WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...)  WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...)  WHAT(X)FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...)  WHAT(X)FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...)  WHAT(X)FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...)  WHAT(X)FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...)  WHAT(X)FE_8(WHAT, __VA_ARGS__)
#define FE_10(WHAT, X, ...) WHAT(X)FE_9(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME 
#define FOR_EACH(action,...) \
  GET_MACRO(__VA_ARGS__,FE_10,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)
  
//--------------------------


#define expose(...)\
StringConvertibleVariable array[VA_NUM_ARGS(__VA_ARGS__)] = {FOR_EACH(bind,__VA_ARGS__)};\
\
StringConvertibleVariable* getMapper(){\
  return array;\
}\
\
int getMapperLength(){\
  return VA_NUM_ARGS(__VA_ARGS__);\
}

//--------------------------


#define bind(var)\
Variable<var ## __TYPE>(&var,#var,&setterFuncName(var),&getterFuncName(var)),


#define setterFuncName(name) set_ ## name
#define getterFuncName(name) get_ ## name

#define setterHeader(name,body)\
static void setterFuncName(name)(void* instance, const String& value){\
  ThisClass& configs = *cast(instance);\
  name ## __TYPE& config = configs.name;\
  body\
  StringConvertibleVariable* var = configs.map(#name);\
  if (var){\
    var->persist();\
  }\
}\

#define getterHeader(name,body)\
static String getterFuncName(name)(void* instance){\
  ThisClass& configs = *cast(instance);\
  name ## __TYPE& config = configs.name;\
  body\
}\





//--------------------------

// class macro
#define DefineConfig(Name,...)\
class _ ## Name : public Config{\
private:\
  typedef _ ## Name ThisClass;\
  StringConvertibleVariable* map(String key){\
    for(int i = 0; i < getMapperLength(); i++){\
      if (array[i].name == key){\
        return &array[i];\
      }\
      return NULL;\
    }\
  }\
public:\
static _ ## Name* cast(void* instance){\
    return (_ ## Name*) instance;\
}\
\
String prefix = String(#Name);\
String getPrefix(){\
  return prefix;\
}\
\
__VA_ARGS__\
\
};




  



#include "Arduino.h"

#include <stdarg.h>
#include <vector>
using namespace std;


class Config;
typedef void (*setterPtr)(Config* intance,const String& value);
typedef String (*getterPtr)(Config* intance);

class Variable{
public:
  Variable(String name, setterPtr setter, getterPtr getter){
    this->name = name;
    this->setter = setter;
    this->getter = getter;
  }
  String name;
  setterPtr setter;
  getterPtr getter;  
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
    Variable* array = getMapper();
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
    Variable* array = getMapper();
    for (int i = 0; i < size; i++){
      result[getTag(array[i].name)] = array[i].getter(this);
    }
    return result;
  }
  
  String toString(){
    String result = getPrefix() + ":\n";
    int size = getMapperLength();
    Variable* array = getMapper();
    for (int i = 0; i < size; i++){
      result += "\t" + array[i].name + ": " + array[i].getter(this) + "\n";
    }
    return result;
  }
  
  virtual String getPrefix() = 0;
  virtual Variable* getMapper() = 0;
  virtual int getMapperLength() = 0;
};

//-------------------------
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N

#define setter
#define getter

#define var(name,sett,gett)\
name; setterHeader(name)sett; getterHeader(name)gett;

//--------------------------

#define functions(...)\
Variable array[VA_NUM_ARGS(__VA_ARGS__)] = __VA_ARGS__;\
\
Variable* getMapper(){\
  return array;\
}\
\
int getMapperLength(){\
  return VA_NUM_ARGS(__VA_ARGS__);\
}

//--------------------------


#define bind(var)\
Variable(#var,&setterFuncName(var),&getterFuncName(var))


#define setterFuncName(name) set_ ## name
#define getterFuncName(name) get_ ## name

#define setterHeader(name)\
static void setterFuncName(name)(Config* instance, const String& value)

#define getterHeader(name)\
static String getterFuncName(name)(Config* instance)

#define __config (*cast(instance))

//--------------------------

// class macro
#define DefineConfig(Name,...)\
class _ ## Name : public Config {\
public:\
static _ ## Name* cast(Config* instance){\
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




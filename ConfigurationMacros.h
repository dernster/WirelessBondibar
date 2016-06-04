#include "Arduino.h"
#include "Dictionary.h"
#include <EEPROM.h>
#include <typeinfo>

#include <stdarg.h>
#include <vector>
using namespace std;
typedef void (*setterPtr)(void* intance,const String& value);
typedef String (*getterPtr)(void* intance);

class IStringConvertibleVariable{
public:
  virtual String getString() = 0;
  virtual String getTag() = 0;
  virtual String getName() = 0;
  virtual bool isPersistentVariable() = 0;
  virtual void setFromString(const String &value) = 0;
};

class Config{
public:
  void setValue(const String& tag, const String& value){
    int size = getMapperLength();
    IStringConvertibleVariable** array = getMapper();
    for (int i = 0; i < size; i++){
      if (tag == array[i]->getTag()){
        array[i]->setFromString(value);
      }
    }
  };

  String getTag(const String& varName){
    return getPrefix() + "." + varName;
  }

  Dictionary toDictionary(){
    Dictionary result;
    int size = getMapperLength();
    IStringConvertibleVariable** array = getMapper();
    for (int i = 0; i < size; i++){
      result[array[i]->getTag()] = array[i]->getString();
    }
    return result;
  }

  vector<IStringConvertibleVariable*> toVars(){
    vector<IStringConvertibleVariable*> result;
    int size = getMapperLength();
    IStringConvertibleVariable** array = getMapper();
    for (int i = 0; i < size; i++){
      result.push_back(array[i]);
    }
    return result;
  }

  String toString(){
    String result = "";
    int size = getMapperLength();
    IStringConvertibleVariable** array = getMapper();
    for (int i = 0; i < size; i++){
      String local = array[i]->getTag() + ":" + array[i]->getString() +  ((i == size-1) ? "" : " ");
      result = result + local;
    }
    return result;
  }

  virtual String getPrefix() = 0;
  virtual IStringConvertibleVariable** getMapper() = 0;
  virtual int getMapperLength() = 0;
};

class Type{
public:
  enum Types{PERSISTENT_VAR,VAR};
  virtual Types type() = 0;
};

class StringConvertibleVariable: public IStringConvertibleVariable, public Type{
public:
  StringConvertibleVariable(String name, setterPtr setter, getterPtr getter, Config* config){
    this->name = name;
    this->setter = setter;
    this->getter = getter;
    this->config = config;
  }

  bool isPersistentVariable(){
    return (type() == Types::PERSISTENT_VAR);
  }

  virtual Types type(){
    return Types::VAR;
  }

  String getString(){
    return getter(config);
  }

  void setFromString(const String &value){
    setter(config,value);
  }

  String getTag(){
    return config->getTag(name);
  }

  String getName(){
    return name;
  }

  String name;
  setterPtr setter;
  getterPtr getter;
  static int persistentVariablesSize;
  static bool eepromWasInitialized;
  Config* config;
};

template<class T>
class Variable: public StringConvertibleVariable{
public:
  Variable(T* variable, String name, setterPtr setter, getterPtr getter, Config* config): StringConvertibleVariable(name,setter,getter,config){
    this->variable = variable;
  }
  virtual Types type(){
    return Types::VAR;
  }
protected:
  T* variable;
};

#define STR_SIZE 20
template<class T>
class PersistentVariable : public Variable<T>{
public:
  bool isString;
  PersistentVariable(T* variable, String name, setterPtr setter, getterPtr getter, Config* config): Variable<T>(variable,name,setter,getter,config){

    if (!StringConvertibleVariable::eepromWasInitialized){
      initEEPROM();
      StringConvertibleVariable::eepromWasInitialized = true;
    }

    isString = std::is_same<T, String>::value;
    address = StringConvertibleVariable::persistentVariablesSize;
    StringConvertibleVariable::persistentVariablesSize += isString? STR_SIZE : sizeof(*variable);
    Serial.println(String("Creating persistent variable ") + Variable<T>::name);
    read();

    if (StringConvertibleVariable::persistentVariablesSize > 512){
      Serial.println("WARNING! persistent vars exceed EEPROM size");
    }
  }

  void initEEPROM(){
    EEPROM.begin(512);
    byte v;
    EEPROM.get(0,v);
    if (v != 0xAA){
      Serial.println("Initializing EEPROM...");
      EEPROM.write(0,0xAA);
      for(int i = 1; i < 512; i++)
        EEPROM.write(i,0);
      Serial.println("Initializing EEPROM...Done!");
    }
    EEPROM.end();
    delay(500);
  };

  virtual void read(){
    // read variable
    EEPROM.begin(512);
    if (isString){
      String res = "";
//      Serial.println(String("starting address = ") + String(address));
      for(int addr = address, i = 0; i < STR_SIZE-1; i++, addr++){
        char r = char(EEPROM.read(addr));
//        Serial.println(String("reading ") + String(i));
//        Serial.println(r);
        if (r == '\0')
          break;

        res += String(r);
      }
      *((String*)Variable<T>::variable) = res;
    }else{
      EEPROM.get<T>(address,*Variable<T>::variable);
    }
    EEPROM.end();
    Serial.println(String("Variable ") + Variable<T>::name + " read. -> " + String(*Variable<T>::variable));
  }

  virtual void persist(){
//    Serial.println(String("starting address = ") + String(address));
    EEPROM.begin(512);
    if (!isString){
      EEPROM.put<T>(address,*Variable<T>::variable);
    }else{
      String* var = (String*)Variable<T>::variable;
      int addr = address;
      for(int i = 0; i < STR_SIZE-1 && i < var->length(); i++, addr++){
        EEPROM.write(addr,var->charAt(i));
//        Serial.println(var->charAt(i));
      }
      EEPROM.write(addr,'\0');
    }
    EEPROM.end();
    delay(500);

    Serial.println(String("Variable ") + Variable<T>::name + " write. -> " + String(*Variable<T>::variable));
  }

  virtual Type::Types type(){
    return Type::Types::PERSISTENT_VAR;
  }

  int address;
};



//-------------------------
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,N,...) N

#define var(type,name,setBody,getBody)\
private:\
  typedef type name ## __TYPE;\
  typedef Variable<type> name ## __META_TYPE;\
public:\
type name; setterHeader(name,setBody); getterHeader(name,getBody);

#define persistentVar(type,name,setBody,getBody)\
private:\
  typedef type name ## __TYPE;\
  typedef PersistentVariable<type> name ## __META_TYPE;\
public:\
type name; setterHeader(name,setBody); getterHeader(name,getBody);\
void persist_ ## name(){\
  IStringConvertibleVariable* var = map(#name);\
  ((name ## __META_TYPE*) var)->persist();\
};\
void read_ ## name(){\
  IStringConvertibleVariable* var = map(#name);\
  ((name ## __META_TYPE*) var)->read();\
};

#define readOnly(type,name,getBody)\
private:\
  typedef type name ## __TYPE;\
  typedef Variable<type> name ## __META_TYPE;\
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
#define FE_11(WHAT, X, ...) WHAT(X)FE_10(WHAT, __VA_ARGS__)
#define FE_12(WHAT, X, ...) WHAT(X)FE_11(WHAT, __VA_ARGS__)
#define FE_13(WHAT, X, ...) WHAT(X)FE_12(WHAT, __VA_ARGS__)
#define FE_14(WHAT, X, ...) WHAT(X)FE_13(WHAT, __VA_ARGS__)
#define FE_15(WHAT, X, ...) WHAT(X)FE_14(WHAT, __VA_ARGS__)
#define FE_16(WHAT, X, ...) WHAT(X)FE_15(WHAT, __VA_ARGS__)
#define FE_17(WHAT, X, ...) WHAT(X)FE_16(WHAT, __VA_ARGS__)
#define FE_18(WHAT, X, ...) WHAT(X)FE_17(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,NAME,...) NAME
#define FOR_EACH(action,...) \
  GET_MACRO(__VA_ARGS__,FE_18,FE_17,FE_16,FE_15,FE_14,FE_13,FE_12,FE_11,FE_10,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)

//--------------------------


#define expose(...)\
IStringConvertibleVariable* array[VA_NUM_ARGS(__VA_ARGS__)] = {FOR_EACH(bind,__VA_ARGS__)};\
\
IStringConvertibleVariable** getMapper(){\
  return array;\
}\
\
int getMapperLength(){\
  return VA_NUM_ARGS(__VA_ARGS__);\
}

//--------------------------


#define bind(var)\
new var ## __META_TYPE(&var,#var,&setterFuncName(var),&getterFuncName(var),this),


#define setterFuncName(name) set_ ## name
#define getterFuncName(name) get_ ## name

#define setterHeader(name,body)\
static void setterFuncName(name)(void* instance, const String& value){\
  ThisClass& configs = *cast(instance);\
  name ## __TYPE& config = configs.name;\
  body\
  IStringConvertibleVariable* var = configs.map(#name);\
  if (var->isPersistentVariable()){\
    ((PersistentVariable<name ## __TYPE>*) var)->persist();\
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
public:\
  typedef _ ## Name ThisClass;\
  IStringConvertibleVariable* map(String key){\
    for(int i = 0; i < getMapperLength(); i++){\
      if (array[i]->getName() == key){\
        return array[i];\
      }\
    }\
    Serial.println(String("mapping error with " + key));\
    return NULL;\
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

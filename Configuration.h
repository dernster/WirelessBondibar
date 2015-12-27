#pragma once
#include "utils.h"
#include "ConfigurationMacros.h"

//-------------Wifi---------------

DefineConfig( Wifi,

  String var(ssid,
  set{
    config.ssid = value;
  },
  get{
    return config.ssid;
  })

  String var(password,
  set{
    config.password = value;
  },
  get{
    return config.password;
  })

  String var(ip,
  set{
    config.ip = value;
  },
  get{
    return config.ip;
  })

  functions({
      bind(ssid),
      bind(password),
      bind(ip)
  });
)

//-------------ConfigurationServer---------------

DefineConfig( ConfigurationServer,
  
  String var(ip,
  set{
    config.ip = value;
  },
  get{
    return config.ip;
  })
  
  int var(port,
  set{
    config.port = value.toInt();
  },
  get{
    return String(config.port);
  })
  
  int var(discoveryPort,
  set{
    config.discoveryPort = value.toInt();
  },
  get{
    return String(config.discoveryPort);
  })
  
  int var(packetLength,
  set{
    config.packetLength = value.toInt();
  },
  get{
    return String(config.packetLength);
  })

  functions({
      bind(ip),
      bind(port),
      bind(discoveryPort),
      bind(packetLength)
  });
);

//-------------Device---------------

DefineConfig( Device,
  
  int var(number,
  set{
    config.number = value.toInt();
  },
  get{
    return String(config.number);
  })

  int var(managedPixelsQty,
  set{
    config.managedPixelsQty = value.toInt();
  },
  get{
    return String(config.managedPixelsQty);
  })

  int var(firstPixel,
  set{
    config.firstPixel = value.toInt();
  },
  get{
    return String(config.firstPixel);
  })
 
  functions({
      bind(number),
      bind(managedPixelsQty),
      bind(firstPixel)
  });
);


//-------------Streaming---------------

DefineConfig( Streaming,
  
  int var(port,
  set{
    config.port = value.toInt();
  },
  get{
    return String(config.port);
  })
 
  functions({
      bind(port)
  });
);

//-------------Global---------------

DefineConfig( Global,
  
  int var(pixelsQty,
  set{
    config.pixelsQty = value.toInt();
  },
  get{
    return String(config.pixelsQty);
  })
 
  functions({
      bind(pixelsQty)
  });
);

class ConfigurationObserver{
public:
  virtual void configurationChanged() = 0;
};

class Configuration{
SINGLETON_H(Configuration)
public:
  Configuration(){
    Serial.begin(9600);
    Serial.println("Configuration()");
    configs.push_back(Wifi = new _Wifi);
    configs.push_back(ConfigurationServer = new _ConfigurationServer);
    configs.push_back(Device = new _Device);
    configs.push_back(Streaming = new _Streaming);
    configs.push_back(Global = new _Global);

    this->Global->pixelsQty = 200; /* not set yet */
    this->ConfigurationServer->packetLength = 200;
  }

  vector<Config*> configs;
  
  _Wifi* Wifi;
  _ConfigurationServer* ConfigurationServer;
  _Device* Device;
  _Streaming* Streaming;
  _Global* Global;

  vector<ConfigurationObserver*> observers;

  void addObserver(ConfigurationObserver* obs){
    observers.push_back(obs);
  }

  void removeObserver(ConfigurationObserver* obs){
    for(int i = 0; i < observers.size(); i++){
      if (observers[i] == obs){
        observers.erase(observers.begin()+i);
      }
    }
  }

  void notifyObservers(){
    // because observers may remove themselves, copy list first
    vector<ConfigurationObserver*> current(observers);
    for(int i = 0; i < current.size(); i++){
      current[i]->configurationChanged();
    }
  }

  String toString(){
    String res = "========== Settings ==========\n";
    for(int i = 0; i < configs.size(); i++){
      res += configs[i]->toString();
    }
    res += "==============================\n";
    return res;
  }

  Dictionary toDictionary(){
    Dictionary res;
    for(int i = 0; i < configs.size(); i++){
      res.append(configs[i]->toDictionary());
    }
    return res;
  }

  void setValue(const String& tag, const String& value){
    for(int i = 0; i < configs.size(); i++){
      // tags are unique, so this will set de value only in one config
      configs[i]->setValue(tag,value);
    }
  }

  void setValues(Dictionary& dict, bool notify = true){
    for(int i = 0; i < dict.size(); i++){
      StringPair& pair = dict.pairAt(i);
      setValue(pair.first,pair.second);
    }
    if (notify)
      notifyObservers();
  }
};






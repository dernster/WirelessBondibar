#pragma once
#include "utils.h"
#include "Storage.h"
#include "ConfigurationMacros.h"


//-------------Wifi---------------

DefineConfig( Wifi,

  String var(ssid,
  setter{
    __config.ssid = value;
  },
  getter{
    return __config.ssid;
  })

  String var(password,
  setter{
    __config.password = value;
  },
  getter{
    return __config.password;
  })

  String var(ip,
  setter{
    /* read only */
  },
  getter{
    return __config.ip;
  })

  functions({
      bind(ssid),
      bind(password),
      bind(ip)
  });
)

//-------------ConfigurationServer---------------

DefineConfig( ControlServer,
  
  String var(ip,
  setter{
    __config.ip = value;
  },
  getter{
    return __config.ip;
  })
  
  int var(port,
  setter{
    __config.port = value.toInt();
  },
  getter{
    return String(__config.port);
  })
  
  int var(discoveryPort,
  setter{
    __config.discoveryPort = value.toInt();
  },
  getter{
    return String(__config.discoveryPort);
  })
  
  int var(packetLength,
  setter{
    __config.packetLength = value.toInt();
  },
  getter{
    return String(__config.packetLength);
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
  setter{
    /* read only */
  },
  getter{
    return String(__config.number);
  })

  int var(managedPixelsQty,
  setter{
    __config.managedPixelsQty = value.toInt();
  },
  getter{
    return String(__config.managedPixelsQty);
  })

  int var(firstPixel,
  setter{
    __config.firstPixel = value.toInt();
  },
  getter{
    return String(__config.firstPixel);
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
  setter{
    __config.port = value.toInt();
  },
  getter{
    return String(__config.port);
  })
 
  functions({
      bind(port)
  });
);


//-------------Stats---------------

DefineConfig( Stats,
  
  float var(bitRate,
  setter{
    /* read only */
  },
  getter{
    return String(__config.bitRate) + " kbps";
  })

  float var(streamingQueueMeanSize,
  setter{
    /* read only */
  },
  getter{
    return String(__config.streamingQueueMeanSize) + " frames";
  })
 
  float var(playbackMeanDelay,
  setter{
    /* read only */
  },
  getter{
    return String(__config.playbackMeanDelay) + " milliseconds";
  })

  float var(playbackMaxDelay,
  setter{
    /* read only */
  },
  getter{
    return String(__config.playbackMaxDelay) + " milliseconds";
  })

  functions({
      bind(bitRate),
      bind(streamingQueueMeanSize),
      bind(playbackMeanDelay),
      bind(playbackMaxDelay)
  });
);

//-------------Global---------------

DefineConfig( Global,
  
  int var(pixelsQty,
  setter{
    __config.pixelsQty = value.toInt();
  },
  getter{
    return String(__config.pixelsQty);
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
    
    configs.push_back(Wifi = new _Wifi);
    configs.push_back(ControlServer = new _ControlServer);
    configs.push_back(Device = new _Device);
    configs.push_back(Streaming = new _Streaming);
    configs.push_back(Global = new _Global);
    configs.push_back(Stats = new _Stats);

    this->Global->pixelsQty = 200; /* not set yet */
    this->ControlServer->packetLength = 200;
  }

  vector<Config*> configs;
  
  _Wifi* Wifi;
  _ControlServer* ControlServer;
  _Device* Device;
  _Streaming* Streaming;
  _Global* Global;
  _Stats* Stats;

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






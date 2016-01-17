#pragma once
#include "utils.h"
#include "Storage.h"
#include "ConfigurationMacros.h"


//-------------Wifi---------------


DefineConfig( Wifi,

  expose(
      ssid,
      password,
      ip
  );

  var( String, ssid,
  {
    config = value;
  },
  {
    return config;
  })

  var( String, password,
  {
    config = value;
  },
  {
    return config;
  })

  readOnly( String, ip,
  {
    return config;
  })
)

//-------------ConfigurationServer---------------

DefineConfig( ControlServer,

  
  expose(
      ip,
      port,
      discoveryPort,
      packetLength
  );
  
  var( String, ip,
  {
    config = value;
  },
  {
    return config;
  })
  
  var( int, port,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
  
  var( int, discoveryPort,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
  
  var( int, packetLength,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
);

//-------------Device---------------

DefineConfig( Device,

  expose(
      number,
      managedPixelsQty,
      firstPixel
  );
  
  readOnly( int, number,
  {
    return String(config);
  })

  var( int, managedPixelsQty,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( int, firstPixel,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
);


//-------------Streaming---------------

DefineConfig( Streaming,

  expose(
      port
  );
  
  var( int, port,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
);


//-------------Stats---------------

DefineConfig( Stats,

  expose(
      bitRate,
      streamingQueueMeanSize,
      playbackMeanDelay,
      playbackMaxDelay
  );
  
  readOnly( float, bitRate,
  {
    return String(config) + " kbps";
  })

  readOnly( float, streamingQueueMeanSize,
  {
    return String(config) + " frames";
  })
 
  readOnly( float, playbackMeanDelay,
  {
    return String(config) + " milliseconds";
  })

  readOnly( float, playbackMaxDelay,
  {
    return String(config) + " milliseconds";
  })
);

//-------------Global---------------

DefineConfig( Global,
  
  var( int, pixelsQty,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })
 
  expose(
      pixelsQty
  );
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






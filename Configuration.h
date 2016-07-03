#pragma once
#include "utils.h"
#include "ConfigurationMacros.h"


//-------------Wifi---------------


DefineConfig( Wifi,

  persistentVar( String, ssid,
  {
    config = value;
  },
  {
    return config;
  })

  persistentVar( String, password,
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

  expose(
      ssid,
      password,
      ip
  );
)

//-------------ConfigurationServer---------------

DefineConfig( ControlServer,

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

  var( int, keepAliveSeconds,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  expose(
      port,
      discoveryPort,
      packetLength,
      keepAliveSeconds
  );
);

//-------------Device---------------

DefineConfig( Device,

  persistentVar( int, number,
  {
    config = value.toInt();
  },
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

  var( String, colorOrder,
  {
    config = value;
  },
  {
    return config;
  })

  expose(
      number,
      managedPixelsQty,
      firstPixel,
      colorOrder
  );
);


//-------------Streaming---------------

DefineConfig( Streaming,

  var( String, serverIP,
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

  var( unsigned long, playbackTimeDelay,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( String, multicastGroupIp,
  {
    config = value;
  },
  {
    return config;
  })

  var( int, multicastGroupFirstPixel,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( int, pixelsQty,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  expose(
      port,
      serverIP,
      playbackTimeDelay,
      multicastGroupIp,
      multicastGroupFirstPixel,
      pixelsQty
  );
);


//-------------ClockSync---------------

DefineConfig( ClockSync,

  var( long, offsetSigma,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( unsigned long, expirationPeriod,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( unsigned long, offsetMeanCalibrationConsecutivePackets,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  var( float, offsetMeanCalibrationDerivativeThreshold,
  {
    config = value.toFloat();
  },
  {
    return String(config);
  })

  var( unsigned long, firstPacketsIgnoreQty,
  {
    config = value.toInt();
  },
  {
    return String(config);
  })

  expose(
      offsetSigma,
      expirationPeriod,
      offsetMeanCalibrationConsecutivePackets,
      offsetMeanCalibrationDerivativeThreshold,
      firstPacketsIgnoreQty
  );
);

//-------------Stats---------------

DefineConfig( Stats,

  _Stats(){
    bitRate = 0;
    streamingQueueMeanWaitingTime = 0;
    streamingQueueWaitingTimeStdev = 0;
    playbackMeanDelay = 0;
    playbackMaxDelay = 0;
    packetLossRate = 0;
    delayedFramesRate = 0;
    ptFrameRateMean = 0;
    ptFrameRateStdev = 0;
    ptFrameRateMax = 0;
    ptFrameRateMin = 999999999;
    latePacketsRate = 0;
    dirty = false;
  }

  readOnly( float, bitRate,
  {
    return String(config,4); // kbps
  })

  readOnly( double, streamingQueueWaitingTimeStdev,
  {
    return String(config,5);
  })

  readOnly( double, streamingQueueMeanWaitingTime,
  {
    return String(config, 5);
  })

  readOnly( float, playbackMeanDelay,
  {
    return String(config,4); // milliseconds
  })

  readOnly( unsigned long, playbackMaxDelay,
  {
    return String(config); // milliseconds
  })

  readOnly( float, packetLossRate,
  {
    return String(config,4);
  })

  readOnly( float, delayedFramesRate,
  {
    return String(config,4);
  })

  readOnly( double, ptFrameRateMean,
  {
    return String(config, 5);
  })

  readOnly( double, ptFrameRateStdev,
  {
    return String(config, 5);
  })

  readOnly( double, ptFrameRateMax,
  {
    return String(config, 5);
  })

  readOnly( double, ptFrameRateMin,
  {
    return String(config, 5);
  })

  readOnly( bool, dirty,
  {
    return config ? "True" : "False";
  })

  readOnly( double, latePacketsRate,
  {
    return String(config, 5);
  })

  expose(
      bitRate,
      streamingQueueMeanWaitingTime,
      streamingQueueWaitingTimeStdev,
      playbackMeanDelay,
      playbackMaxDelay,
      packetLossRate,
      delayedFramesRate,
      ptFrameRateMean,
      ptFrameRateStdev,
      ptFrameRateMax,
      ptFrameRateMin,
      dirty,
      latePacketsRate
    );
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
    configs.push_back(ClockSync = new _ClockSync);

    this->Global->pixelsQty = 200; /* not set yet */
    this->ControlServer->packetLength = 600;
    this->ControlServer->keepAliveSeconds = 20;
    this->Stats->packetLossRate = 0;
    this->Device->managedPixelsQty = 8;

    this->Device->firstPixel = this->Device->number*this->Device->managedPixelsQty;
    this->Streaming->port = 7788;
    this->ControlServer->discoveryPort = 8888;
    this->ControlServer->port = 8889;
  }

  vector<Config*> configs;

  _Wifi* Wifi;
  _ControlServer* ControlServer;
  _Device* Device;
  _Streaming* Streaming;
  _Global* Global;
  _Stats* Stats;
  _ClockSync* ClockSync;

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

  vector<IStringConvertibleVariable*> toVars(){
    vector<IStringConvertibleVariable*> result;
    for(int i = 0; i < configs.size(); i++){
      vector<IStringConvertibleVariable*> vars = configs[i]->toVars();
      result.insert(result.end(),vars.begin(),vars.end());
    }
    return result;
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
      Serial.println(String("setting ") + pair.first + "=" + pair.second);
      setValue(pair.first,pair.second);
    }
    if (notify)
      notifyObservers();
  }
};

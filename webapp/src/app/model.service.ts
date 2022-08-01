import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, ReplaySubject, Subject } from 'rxjs';
import { SockIOClient } from './sockio';

class Settings {
  mode: string = 'Tractor';  // Tractor or Implement
  wifiMode: string = 'TractorWifi';  // HouseWifi or TractorWifi
  name: string = 'Tractor';
  houseSSID: string = '';
  housePassword: string = '';
  tractorSSID: string = 'Tractor';
  tractorPassword: string = '12345678';
  tractorAddress: string = '8.8.8.8';
  enableDisplay: boolean = true;
}

@Injectable({
  providedIn: 'root'
})
export class ModelService {

  // TODO: set this for production
  io = new SockIOClient('ws://' + window.location.hostname + ':81/')
  //io = new SockIOClient('ws://localhost:8080/');
  //io = new SockIOClient('ws://10.10.10.122:81/');

  connected: boolean = false;

  running: Settings = new Settings();
  save: Settings = new Settings();
  wifiRSSI: number = 0;
  calibrated: boolean = false;
  roll: number = 0;
  pitch: number = 0;
  remoteConnected: boolean = false;
  remoteName: String = '';
  remoteAddress: String = '';
  remoteRoll: number = 0;
  remotePitch: number = 0;
  configDirty: boolean  = false;
  
  configUpdatedSubject = new ReplaySubject<void>();
  connectedSubject = new BehaviorSubject<boolean>(false);

  constructor() {

    this.io.on('connected', () => {
      this.connected = true;
      this.connectedSubject.next(true);
    });
    
    this.io.on('disconnected', () => {
      this.connected = false;
      this.connectedSubject.next(false);
    });

    this.io.on('runningSettings', o => {
      this.running.mode = o.mode;
      this.running.wifiMode = o.wifiMode;
      this.running.name = o.name;
      this.running.houseSSID = o.houseSSID;
      this.running.housePassword = o.housePassword;
      this.running.tractorSSID = o.tractorSSID;
      this.running.tractorPassword = o.tractorPassword;
      this.running.tractorAddress = o.tractorAddress;
      this.running.enableDisplay = o.enableDisplay;
      this.configUpdatedSubject.next();
    });

    this.io.on('saveSettings', o => {
      this.save.mode = o.mode;
      this.save.wifiMode = o.wifiMode;
      this.save.name = o.name;
      this.save.houseSSID = o.houseSSID;
      this.save.housePassword = o.housePassword;
      this.save.tractorSSID = o.tractorSSID;
      this.save.tractorPassword = o.tractorPassword;
      this.save.tractorAddress = o.tractorAddress;
      this.save.enableDisplay = o.enableDisplay;
      this.configUpdatedSubject.next();
    });

    this.io.on('wifiRSSI', n => {
      this.wifiRSSI = n;
    });

    this.io.on('calibrated', b => {
      this.calibrated = b;
    });
    
     this.io.on('angles', (roll, pitch) => {
      this.roll = roll;
      this.pitch = pitch;
    });

    this.io.on('remoteConnected', b => {
      this.remoteConnected = b;
    });

    this.io.on('remoteInfo', (name, address) => {
      this.remoteName = name;
      this.remoteAddress = address;
    });

    this.io.on('remoteAngles', (roll, pitch) => {
      this.remoteRoll = roll;
      this.remotePitch = pitch;
    });

    this.io.on('configDirty', b => {
      this.configDirty = b;
    });

  }
  
}

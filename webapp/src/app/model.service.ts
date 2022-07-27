import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, ReplaySubject, Subject } from 'rxjs';
import { SockIOClient } from './sockio';

@Injectable({
  providedIn: 'root'
})
export class ModelService {

  // TODO: set this for production
  //private apiUrl = '/api/';
  //private apiUrl = 'http://10.10.10.122/api/';
  //private wsUrl = '/ws';
  //private wsUrl = 'ws://10.10.10.122/ws';
  //private url = 'ws://localhost:8080/';
  
  //io = new SockIOClient('ws://localhost:8080/');
  io = new SockIOClient('ws://10.10.10.122:81/');

  connected: boolean = false;
  
  mode: string = 'Tractor';  // Tractor or Implement
  wifiMode: string = 'TractorWifi';  // HouseWifi or TractorWifi
  name: string = 'Missy';
  houseSSID: string = '';
  housePassword: string = '';
  tractorSSID: string = 'Tractor';
  tractorPassword: string = '12345678';
  tractorAddress: string = '8.8.8.8';
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

    this.io.on('settings', o => {
      this.mode = o.mode;
      this.wifiMode = o.wifiMode;
      this.name = o.name;
      this.houseSSID = o.houseSSID;
      this.housePassword = o.housePassword;
      this.tractorSSID = o.tractorSSID;
      this.tractorPassword = o.tractorPassword;
      this.tractorAddress = o.tractorAddress;
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
      this.remoteRoll = pitch;
    });

    this.io.on('configDirty', b => {
      this.configDirty = b;
    });

  }
  
}

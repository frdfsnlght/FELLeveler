import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, ReplaySubject, Subject } from 'rxjs';
//import { webSocket, WebSocketSubject, WebSocketSubjectConfig } from 'rxjs/webSocket';
import { SockIOClient } from './sockio';

export class Device {
  name: string;
  address: string;
  constructor(name: string, address: string) {
    this.name = name;
    this.address = address;
  }
}

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
  
  io = new SockIOClient('ws://localhost:8080/');

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
  implementConnected: boolean = false;
  implementInfo: Device = new Device('', '');
  implementRoll: number = 0;
  implementPitch: number = 0;
  configDirty: boolean  = false;
  
  configUpdatedSubject = new ReplaySubject<void>();
  connectedSubject = new BehaviorSubject<boolean>(false);

  /*
  connected = new BehaviorSubject<boolean>(true);
  mode = new BehaviorSubject<string>('Tractor');  // Tractor or Implement
  wifiMode = new BehaviorSubject<string>('TractorWifi');  // HouseWifi or TractorWifi
  name = new BehaviorSubject<string>('Missy');
  houseSSID = new BehaviorSubject<string>('');
  housePassword = new BehaviorSubject<string>('');
  tractorSSID = new BehaviorSubject<string>('Tractor');
  tractorPassword = new BehaviorSubject<string>('12345678');
  tractorAddress = new BehaviorSubject<string>('8.8.8.8');
  wifiRSSI = new BehaviorSubject<number>(0);
  netsockConnected = new BehaviorSubject<boolean>(false);
  netsockRemoteDevice = new BehaviorSubject<Device>(new Device('', ''));

  calibrated = new BehaviorSubject<boolean>(false);
  roll = new BehaviorSubject<number>(0);
  pitch = new BehaviorSubject<number>(0);
  implementRoll = new BehaviorSubject<number>(0);  // tractor only
  implementPitch = new BehaviorSubject<number>(0);  // tractor only

  configDirty = new BehaviorSubject<boolean>(false);
  */

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
    
     this.io.on('pitch', n => {
      this.pitch = n;
    });

    this.io.on('roll', n => {
      this.roll = n;
    });

    this.io.on('implementConnected', b => {
      this.implementConnected = b;
    });

    this.io.on('implementInfo', o => {
      this.implementInfo = new Device(o.name, o.address);
    });

    this.io.on('implementRoll', n => {
      this.implementRoll = n;
    });

    this.io.on('implementPitch', n => {
      this.implementPitch = n;
    });

    this.io.on('configDirty', b => {
      this.configDirty = b;
    });

  }
  
}

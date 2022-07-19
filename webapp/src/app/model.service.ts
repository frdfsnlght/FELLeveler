import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable, ReplaySubject, Subject } from 'rxjs';
import { webSocket, WebSocketSubject, WebSocketSubjectConfig } from 'rxjs/webSocket';

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
  private wsUrl = 'ws://10.10.10.122/ws';

  private ws: WebSocketSubject<any>;

  private watchdogTimer: any = false;
  private watchdogTimeout = 3000

  connected = new BehaviorSubject<boolean>(false);

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

  private nextRequestId: number = 1;
  private requests = new Map<number, ReplaySubject<any>>();
  private eventHandlers = new Map<string, (data: any)=>void>();
    
  constructor() {

    this.eventHandlers.set('keepAlive', b => {
      this.connected.next(true);
      //this.resetWatchdog();
    });

    this.eventHandlers.set('settings', o => {
      this.mode.next(o.mode);
      this.wifiMode.next(o.wifiMode);
      this.name.next(o.name);
      this.houseSSID.next(o.houseSSID);
      this.housePassword.next(o.housePassword);
      this.tractorSSID.next(o.tractorSSID);
      this.tractorPassword.next(o.tractorPassword);
      this.tractorAddress.next(o.tractorAddress);
    });

    this.eventHandlers.set('wifiRSSI', n => {
      this.wifiRSSI.next(n);
    });

    this.eventHandlers.set('netsockConnected', b => {
      this.netsockConnected.next(b);
    });

    this.eventHandlers.set('netsockRemoteDevice', o => {
      this.netsockRemoteDevice.next(new Device(o.name, o.address));
    });

    this.eventHandlers.set('calibrated', b => {
      this.calibrated.next(b);
    });

    this.eventHandlers.set('roll', n => {
      this.roll.next(n);
    });

    this.eventHandlers.set('pitch', n => {
      this.pitch.next(n);
    });

    this.eventHandlers.set('implementRoll', n => {
      this.implementRoll.next(n);
    });

    this.eventHandlers.set('implementPitch', n => {
      this.implementPitch.next(n);
    });

    this.eventHandlers.set('configDirty', b => {
      this.configDirty.next(b);
    });

    this.ws = webSocket({
      url: this.wsUrl
    });

    this.ws.subscribe({
      next: (obj: any) => {
        this.connected.next(true);
        //console.log(obj);
        if ('id' in obj && 'data' in obj) {
          console.log(obj);
          var id = obj.id;
          var data = obj.data;
          var sub = this.requests.get(id);
          if (sub) {
            console.debug('found matching request for ' + id);
            this.requests.delete(id);
            sub.next(data);
            return;
          }
          console.warn('no request found for ' + id);
          return;
        }
        if ('event' in obj && 'data' in obj) {
          var event = obj.event;
          var data = obj.data;
          var handler = this.eventHandlers.get(event);
          if (handler) {
            //console.debug('found matching event handler for "' + event + '"');
            handler(data);
            return;
          }
          console.warn('no event handler found for "' + event + '"');
          return;
        }
        // TODO: add RPC call handling if needed
      },
      error: (err) => {
        console.log('Websocket error: ' + err);
      },
      complete: () => {
        console.log('Websocket closed');
        this.connected.next(false);
        // TODO: set timer to reconnect?
      }
    });

    //this.resetWatchdog();
  }
  
/*
  resetWatchdog(): void {
    if (this.watchdogTimer)
      clearTimeout(this.watchdogTimer);
    this.watchdogTimer = setTimeout(() => {
      console.log("Watchdog expired, reopening SSE connection");
      this.connected.next(false);
      if (this.eventSource)
        this.eventSource.close();
      this.eventSource = this.createEventSource();
    }, this.watchdogTimeout);
  }
*/

  configure(data: object): Observable<any> {
    return this.makeRequest('configure', data);
  }

  calibrateLevel(): Observable<any> {
    return this.makeRequest('calibrateLevel');
  }

  calibrateTipped(): Observable<any> {
    return this.makeRequest('calibrateTipped');
  }
  
  saveConfig(): Observable<any> {
    return this.makeRequest('saveConfig');
  }

  reboot(): Observable<any> {
    return this.makeRequest('reboot');
  }

  test(): Observable<any> {
    return this.makeRequest('test');
  }

  makeRequest(method: string, data: any = null): Observable<any> {
    if (! this.connected)
      return new BehaviorSubject<string>("Not connected").asObservable();
    var req = {id: this.nextRequestId++, method: method, data: data};
    var sub = new ReplaySubject<any>(1);
    this.requests.set(req.id, sub);
    console.log('Request: ' + JSON.stringify(req));
    this.ws.next(req);
    return sub.asObservable();
  }

}

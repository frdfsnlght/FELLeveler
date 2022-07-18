import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';
import { HttpClient, HttpParams } from '@angular/common/http';

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

  //private apiUrl = '/api/';
  private apiUrl = 'http://10.10.10.122/api/';
  
  private eventSource: EventSource;
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

  constructor(
    private http: HttpClient
  ) {
    this.eventSource = this.createEventSource();
    this.resetWatchdog();
  }

  createEventSource(): EventSource {
    //var es:EventSource = new EventSource('/events');
    var es:EventSource = new EventSource('http://10.10.10.122/events');

    es.addEventListener('keepAlive', e => {
      this.connected.next(true);
      this.resetWatchdog();
    });

    es.addEventListener('settings', e => {
      var o = JSON.parse(e.data);
      this.mode.next(o.mode);
      this.wifiMode.next(o.wifiMode);
      this.name.next(o.name);
      this.houseSSID.next(o.houseSSID);
      this.housePassword.next(o.housePassword);
      this.tractorSSID.next(o.tractorSSID);
      this.tractorPassword.next(o.tractorPassword);
      this.tractorAddress.next(o.tractorAddress);
    });

    es.addEventListener('wifiRSSI', e => {
      this.wifiRSSI.next(parseInt(e.data.toString()));
    });

    es.addEventListener('netsockConnected', e => {
      this.netsockConnected.next(e.data.toString() == 'true');
    });

    es.addEventListener('netsockRemoteDevice', e => {
      var o = JSON.parse(e.data.toString());
      this.netsockRemoteDevice.next(new Device(o.name, o.address));
    });

    es.addEventListener('calibrated', e => {
      this.calibrated.next(e.data.toString() == 'true');
    });

    es.addEventListener('roll', e => {
      this.roll.next(parseInt(e.data.toString()));
    });

    es.addEventListener('pitch', e => {
      this.pitch.next(parseInt(e.data.toString()));
    });

    es.addEventListener('implementRoll', e => {
      this.implementRoll.next(parseInt(e.data.toString()));
    });

    es.addEventListener('implementPitch', e => {
      this.implementPitch.next(parseInt(e.data.toString()));
    });

    es.addEventListener('configDirty', e => {
      this.configDirty.next(e.data.toString() == 'true');
    });

    return es;
  }

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

  configure(data: object): Observable<string> {
    return this.http.post(this.apiUrl + 'configure', data, {responseType: 'text'});
  }

  calibrateLevel(): Observable<string> {
    return this.http.get(this.apiUrl + 'calibrateLevel', {responseType: 'text'});
  }

  calibrateTipped(): Observable<string> {
    return this.http.get(this.apiUrl + 'calibrateTipped', {responseType: 'text'});
  }
  
  saveConfig(): Observable<string> {
    return this.http.get(this.apiUrl + 'saveConfig', {responseType: 'text'});
  }

  reboot(): Observable<string> {
    return this.http.get(this.apiUrl + 'reboot', {responseType: 'text'});
  }

}

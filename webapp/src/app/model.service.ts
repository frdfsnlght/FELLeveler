import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';
import { HttpClient, HttpParams } from '@angular/common/http';
import { BtDevice } from './bt-device';

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
  name = new BehaviorSubject<string>('Missy');
  wifiMode = new BehaviorSubject<string>('Station'); // Station or AP
  wifiSSID = new BehaviorSubject<string>('');
  wifiPassword = new BehaviorSubject<string>('');
  wifiRSSI = new BehaviorSubject<number>(0);
  btScannedDevices = new BehaviorSubject<Array<BtDevice>>([]);  // tractor only
  btScanComplete = new BehaviorSubject<boolean>(true);  // tractor only, local
  btPairedDevices = new BehaviorSubject<Array<BtDevice>>([]); // tractor only
  btConnected = new BehaviorSubject<boolean>(false);
  btConnectedDevice = new BehaviorSubject<BtDevice | null>(null);

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
      this.name.next(o.name);
      this.wifiSSID.next(o.wifiSSID);
      this.wifiPassword.next(o.wifiPassword);
    });

    es.addEventListener('wifiMode', e => {
      this.wifiMode.next(e.data.toString());
    });

    es.addEventListener('wifiRSSI', e => {
      this.wifiRSSI.next(parseInt(e.data.toString()));
    });

    es.addEventListener('btScannedDevices', e => {
      var a = JSON.parse(e.data.toString());
      var devs:Array<BtDevice> = [];
      a.forEach((e: { name: string; address: string; }) => {
        devs.push(new BtDevice(e.name, e.address));
      });
      this.btScannedDevices.next(devs);
      this.btScanComplete.next(true);
    });

    es.addEventListener('btPairedDevices', e => {
      var a = JSON.parse(e.data.toString());
      var devs:Array<BtDevice> = [];
      a.forEach((e: { name: string; address: string; }) => {
        devs.push(new BtDevice(e.name, e.address));
      });
      this.btPairedDevices.next(devs);
    });

    es.addEventListener('btConnected', e => {
      this.btConnected.next(e.data.toString() == 'true');
      if (! this.btConnected.value)
        this.btConnectedDevice.next(null);
    });

    es.addEventListener('btConnectedDevice', e => {
      var o = JSON.parse(e.data.toString());
      this.btConnectedDevice.next(new BtDevice(o.name, o.address));
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
  
  scanBTDevices(): Observable<string> {
    this.btScannedDevices.next([]);
    this.btScanComplete.next(false);
    return this.http.get(this.apiUrl + 'scanBTDevices', {responseType: 'text'});
  }

  pairBTDevice(address: string): Observable<string> {
    return this.http.post(this.apiUrl + 'pairBTDevice', {address: address}, {responseType: 'text'});
  }

  unpairBTDevice(address: string): Observable<string> {
    return this.http.post(this.apiUrl + 'unpairBTDevice', {address: address}, {responseType: 'text'});
  }

  saveConfig(): Observable<string> {
    return this.http.get(this.apiUrl + 'saveConfig', {responseType: 'text'});
  }

  reboot(): Observable<string> {
    return this.http.get(this.apiUrl + 'reboot', {responseType: 'text'});
  }

}

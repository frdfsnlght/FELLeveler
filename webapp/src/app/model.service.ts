import { Injectable } from '@angular/core';
import { BehaviorSubject, Observable } from 'rxjs';
import { HttpClient, HttpParams } from '@angular/common/http';
import { BtDevice } from './bt-device';

@Injectable({
  providedIn: 'root'
})
export class ModelService {

  private apiUrl = '/api/';

  private eventSource: EventSource;

  mode = new BehaviorSubject<string>('Tractor');  // Tractor or Implement
  name = new BehaviorSubject<string>('Missy');
  wifiMode = new BehaviorSubject<string>('Station'); // Station or AP
  wifiSSID = new BehaviorSubject<string>('');
  wifiPassword = new BehaviorSubject<string>('');
  wifiConnected = new BehaviorSubject<boolean>(true);  // station mode only
  wifiRSSI = new BehaviorSubject<number>(0);
  calibrated = new BehaviorSubject<boolean>(true);
  roll = new BehaviorSubject<number>(0);  // tractor only
  pitch = new BehaviorSubject<number>(0);
  implementAngle = new BehaviorSubject<number>(0);  // tractor only
  btScannedDevices = new BehaviorSubject<Array<BtDevice>>([]);  // tractor only
  btPairedDevices = new BehaviorSubject<Array<BtDevice>>([]); // tractor only
  btPaired = new BehaviorSubject<boolean>(false); // implement only
  btConnected = new BehaviorSubject<boolean>(false);
  btConnectedDevice = new BehaviorSubject<BtDevice | null>(null); // tractor only

  configDirty = new BehaviorSubject<boolean>(false);

  constructor(
    private http: HttpClient
  ) {
    this.eventSource = new EventSource('/events');

    this.eventSource.addEventListener('open', e => {
      console.log("Events Connected");
    });

    this.eventSource.addEventListener('error', e => {
      if (this.eventSource.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      } 
    });

    this.eventSource.addEventListener('settings', e => {
      var o = JSON.parse(e.data);
      this.mode.next(o.mode);
      this.name.next(o.name);
      this.wifiSSID.next(o.wifiSSID);
      this.wifiPassword.next(o.wifiPassword);
    });

    this.eventSource.addEventListener('wifiMode', e => {
      this.wifiMode.next(e.data.toString());
    });

    this.eventSource.addEventListener('wifiConnected', e => {
      this.wifiConnected.next(e.data.toString() == 'true');
    });

    this.eventSource.addEventListener('wifiRSSI', e => {
      this.wifiRSSI.next(parseInt(e.data.toString()));
    });

    this.eventSource.addEventListener('calibrated', e => {
      this.calibrated.next(e.data.toString() == 'true');
    });

    this.eventSource.addEventListener('roll', e => {
      this.roll.next(parseInt(e.data.toString()));
    });

    this.eventSource.addEventListener('pitch', e => {
      this.pitch.next(parseInt(e.data.toString()));
    });

    this.eventSource.addEventListener('implementAngle', e => {
      this.implementAngle.next(parseInt(e.data.toString()));
    });

    this.eventSource.addEventListener('btScannedDevices', e => {
      var a = JSON.parse(e.data.toString());
      var devs:Array<BtDevice> = [];
      a.forEach((e: { name: string; address: string; }) => {
        devs.push(new BtDevice(e.name, e.address));
      });
      this.btScannedDevices.next(devs);
    });

    this.eventSource.addEventListener('btPairedDevices', e => {
      var a = JSON.parse(e.data.toString());
      var devs:Array<BtDevice> = [];
      a.forEach((e: { name: string; address: string; }) => {
        devs.push(new BtDevice(e.name, e.address));
      });
      this.btPairedDevices.next(devs);
    });

    this.eventSource.addEventListener('btPaired', e => {
      this.btPaired.next(e.data.toString() == 'true');
    });

    this.eventSource.addEventListener('btConnected', e => {
      this.btConnected.next(e.data.toString() == 'true');
      if (! this.btConnected.value)
        this.btConnectedDevice.next(null);
    });

    this.eventSource.addEventListener('btConnectedDevice', e => {
      var o = JSON.parse(e.data.toString());
      this.btConnectedDevice.next(new BtDevice(o.name, o.address));
    });

    this.eventSource.addEventListener('configDirty', e => {
      this.configDirty.next(e.data.toString() == 'true');
    });

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
    return this.http.get(this.apiUrl + 'scanBTDevices', {responseType: 'text'});
  }

  pairBTDevice(address: string): Observable<string> {
    return this.http.post(this.apiUrl + 'pairBTDevice', {address: address}, {responseType: 'text'});
  }

  unpairBTDevice(address: string): Observable<string> {
    return this.http.post(this.apiUrl + 'unpairBTDevice', {address: address}, {responseType: 'text'});
  }

  unpairBT(): Observable<string> {
    return this.http.get(this.apiUrl + 'unpairBT', {responseType: 'text'});
  }

  startPairing(): Observable<string> {
    this.btPaired.next(false);
    return this.http.get(this.apiUrl + 'startPairing', {responseType: 'text'});
  }

  stopPairing(): Observable<string> {
    return this.http.get(this.apiUrl + 'stopPairing', {responseType: 'text'});
  }

  saveConfig(): Observable<string> {
    return this.http.get(this.apiUrl + 'saveConfig', {responseType: 'text'});
  }

  reboot(): Observable<string> {
    return this.http.get(this.apiUrl + 'reboot', {responseType: 'text'});
  }

}

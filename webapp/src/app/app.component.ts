import { Component } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { MatDialog } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from './model.service';
import { ConnectingDialogComponent } from './connecting-dialog/connecting-dialog.component';
import { SettingsDialogComponent } from './settings-dialog/settings-dialog.component';
import { RebootDialogComponent } from './reboot-dialog/reboot-dialog.component';
import { CalibrateDialogComponent } from './calibrate-dialog/calibrate-dialog.component';
import { SockIOClient } from './sockio';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {

  title: string = '';

  private modeSubscription!: Subscription;
  private nameSubscription!: Subscription;
  private connectedSubscription!: Subscription;

  private io!: SockIOClient;

  constructor(
    private titleService: Title,
    public model: ModelService,
    private dialog: MatDialog
  ) {}

  ngOnInit(): void {
    this.modeSubscription = this.model.mode.asObservable().subscribe(s => {this.setTitle()});
    this.nameSubscription = this.model.name.asObservable().subscribe(s => {this.setTitle()});
    this.connectedSubscription = this.model.name.asObservable().subscribe(b => {this.showConnecting(!b)});

    this.io = new SockIOClient('ws://10.10.10.122:81/ws');
    //this.io = new SockIOClient('ws://localhost:8080');

  }

  ngOnDestroy() {
    this.modeSubscription.unsubscribe();
    this.nameSubscription.unsubscribe();
    this.connectedSubscription.unsubscribe();
  }

  setTitle(): void {
    this.title = this.model.mode.value;
    if (this.model.name.value) this.title += ': ' + this.model.name.value;
    this.titleService.setTitle(this.title);
  }

  showConnecting(show: boolean): void {
    this.dialog.open(ConnectingDialogComponent);
  }

  test(): void {
    /*
    this.model.test().subscribe({
      next: (res: any) => {
        console.info('test: ' + res);
      }
    });
    */
    this.io.emit('test', (res: any) => {
        console.log('got a response:', res);
    });



  }

  settings(): void {
    this.dialog.open(SettingsDialogComponent);
  }

  calibrate(): void {
    this.dialog.open(CalibrateDialogComponent);
  }
  
  saveConfig(): void {
    this.model.saveConfig().subscribe({
      next: (res: string) => console.info('saveConfig: ' + res)
    });
  }

  reboot(): void {
    this.dialog.open(RebootDialogComponent);
  }

}

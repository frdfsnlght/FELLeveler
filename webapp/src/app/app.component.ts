import { Component, ElementRef, ViewChild } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { MatDialog } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from './model.service';
import { ConnectingDialogComponent } from './connecting-dialog/connecting-dialog.component';
import { SettingsDialogComponent } from './settings-dialog/settings-dialog.component';
import { RebootDialogComponent } from './reboot-dialog/reboot-dialog.component';
import { CalibrateDialogComponent } from './calibrate-dialog/calibrate-dialog.component';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {

  title: string = '';
  @ViewChild('sidenav', { static: true }) sidenav!: ElementRef;

  private configUpdatedSubscription!: Subscription;
  private connectedSubscription!: Subscription;

  constructor(
    private titleService: Title,
    public model: ModelService,
    private dialog: MatDialog
  ) {}

  ngOnInit(): void {
    this.configUpdatedSubscription = this.model.configUpdatedSubject.asObservable().subscribe(() => {this.setTitle()});
    this.connectedSubscription = this.model.connectedSubject.asObservable().subscribe((b: boolean) => {this.showConnecting(!b)});
    this.setTitle();
  }

  ngOnDestroy() {
    this.configUpdatedSubscription.unsubscribe();
    this.connectedSubscription.unsubscribe();
  }

  setTitle(): void {
    this.title = this.model.mode;
    if (this.model.name) this.title += ': ' + this.model.name;
    this.titleService.setTitle(this.title);
  }

  showConnecting(show: boolean): void {
    this.dialog.open(ConnectingDialogComponent);
  }

  test(): void {
    this.model.io.emit('test', (res: any) => {
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
    this.model.io.emit('saveConfig', (res: any) => {
      console.log('saveConfig:', res);
    })
  }

  reboot(): void {
    this.dialog.open(RebootDialogComponent);
  }

}
